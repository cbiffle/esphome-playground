#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "litime_mppt_ble.h"

namespace esphome {
namespace litime_mppt_ble {

static const char *const TAG = "litime_mppt_ble";

static const uint8_t MAX_NO_RESPONSE_COUNT = 10;

static const uint16_t MODBUS_SERVICE_UUID = 0xFFE0;
static const uint16_t MODBUS_CHARACTERISTIC_UUID = 0xFFE1;

static const uint16_t MAX_RESPONSE_SIZE = 43; // TODO

void LiTimeMpptBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                    esp_ble_gattc_cb_param_t *param) {
    switch (event) {
        case ESP_GATTC_OPEN_EVT: {
            break;
        }
        case ESP_GATTC_DISCONNECT_EVT: {
            this->node_state = espbt::ClientState::IDLE;

            if (this->char_handle_ != 0) {
                auto status = esp_ble_gattc_unregister_for_notify(this->parent()->get_gattc_if(),
                        this->parent()->get_remote_bda(), this->char_handle_);
                if (status) {
                    ESP_LOGW(TAG, "esp_ble_gattc_unregister_for_notify failed, status=%d", status);
                }
            }
            this->char_handle_ = 0;

            this->frame_buffer_.clear();

            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT: {
            auto *char_notify = this->parent_->get_characteristic(MODBUS_SERVICE_UUID, MODBUS_CHARACTERISTIC_UUID);
            if (char_notify == nullptr) {
                ESP_LOGE(TAG, "[%s] No notify service found at device, not a compatible MPPT..?",
                        this->parent_->address_str().c_str());
                break;
            } else {
                ESP_LOGV(TAG, "[%s] found modbus characteristic", this->parent_->address_str().c_str());
            }
            auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                    char_notify->handle);
            if (status) {
                ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
            } else {
                ESP_LOGV(TAG, "[%s] registered handle", this->parent_->address_str().c_str());
            }
            this->char_handle_ = char_notify->handle;

            break;
        }
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
            ESP_LOGV(TAG, "[%s] registered for notify", this->parent_->address_str().c_str());
            this->node_state = espbt::ClientState::ESTABLISHED;
            break;
        }
        case ESP_GATTC_NOTIFY_EVT: {
            ESP_LOGV(TAG, "Notification received (handle 0x%02X): %s", param->notify.handle,
                    format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

            if (param->notify.handle != this->char_handle_) {
                ESP_LOGV(TAG, "...not for us??");
                break;
            }

            this->assemble(param->notify.value, param->notify.value_len);

            break;
        }
    }
}

void LiTimeMpptBle::assemble(const uint8_t *data, uint16_t length) {
    if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
        ESP_LOGW(TAG, "Maximum response size exceeded");
        this->frame_buffer_.clear();
    }

    this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);
    ESP_LOGV(TAG, "Appending %u bytes", length);

    if (this->frame_buffer_.size() >= 5
            && this->frame_buffer_[0] == 0x01
            && this->frame_buffer_[1] == 0x03
            && this->frame_buffer_.size() == (3 + this->frame_buffer_[2] + 2)
    ) {
        // TODO compute CRC here

        std::vector<uint8_t> frame_data(this->frame_buffer_.begin() + 3, this->frame_buffer_.end() - 2);
        if (frame_data.size() == 0x13 * 2) {
            this->on_modbus_response(frame_data);
        } else {
            ESP_LOGW(TAG, "[%s] invalid response length: %u", this->parent_->address_str().c_str(), frame_data.size());
        }
    }
}

void LiTimeMpptBle::update() {
    this->track_online_status_();
    if (this->node_state != espbt::ClientState::ESTABLISHED) {
        ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
        return;
    }

    this->frame_buffer_.clear();
    this->begin_read_registers();
}

void LiTimeMpptBle::begin_read_registers() {
    uint8_t frame[8] = {
        0x01, 0x03,
        0x01,
        0x01,
        0x00,
        0x13,
        0x54,
        0x3B,
    };

    ESP_LOGV(TAG, "Send command (handle 0x%02X)", this->char_handle_);
    auto status = esp_ble_gattc_write_char(
            this->parent_->get_gattc_if(),
            this->parent_->get_conn_id(),
            this->char_handle_,
            sizeof(frame),
            frame,
            ESP_GATT_WRITE_TYPE_RSP,
            ESP_GATT_AUTH_REQ_NONE
    );
    if (status) {
        ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status = %d", this->parent_->address_str().c_str(), status);
    }
}

float bef16(std::vector<uint8_t> const &data, size_t index, float unit) {
    size_t offset = index * 2;
    return (((uint16_t)data[offset] << 8) | data[offset + 1]) * unit;
}

void LiTimeMpptBle::on_modbus_response(std::vector<uint8_t> const &data) {
    this->reset_online_status_tracker_();

    this->publish_state_(this->battery_voltage_sensor_, bef16(data, 1, 0.1f));
    this->publish_state_(this->battery_current_sensor_, bef16(data, 2, 0.01f));
    this->publish_state_(this->battery_power_sensor_, bef16(data, 3, 1));

    // Register 4 contains two packed 8-bit temperatures, we want the higher
    // (first) - the lower (second) appears to be external battery temperature
    this->publish_state_(this->controller_temperature_sensor_, data[2 * 4]);
    this->publish_state_(this->load_voltage_sensor_, bef16(data, 5, 0.1f));
    this->publish_state_(this->load_current_sensor_, bef16(data, 6, 0.01f));
    this->publish_state_(this->load_power_sensor_, bef16(data, 7, 1));
    this->publish_state_(this->panel_voltage_sensor_, bef16(data, 8, 0.1f));
    this->publish_state_(this->peak_power_today_sensor_, bef16(data, 9, 1));
    this->publish_state_(this->energy_production_today_sensor_, bef16(data, 10, 1));
    // 11: not used, controller mode related?
    // 12: mode in my Rust client
    // 13: not understood
    this->publish_state_(this->days_running_sensor_, bef16(data, 14, 1));
    // 15: not understood
    this->publish_state_(this->energy_production_lifetime_sensor_, bef16(data, 16, 1));
}

void LiTimeMpptBle::track_online_status_() {
    if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
        this->no_response_count_++;
    }
    if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
        // Advance the count exactly one more time
        this->no_response_count_++;
        ESP_LOGW(TAG, "[%s] device does not appear to be responding", this->parent_->address_str().c_str());
        this->publish_device_unavailable_();
    }
}

void LiTimeMpptBle::reset_online_status_tracker_() {
    this->no_response_count_ = 0;
    this->publish_state_(this->online_status_binary_sensor_, true);
}

void LiTimeMpptBle::publish_device_unavailable_() {
    this->publish_state_(this->online_status_binary_sensor_, false);

    // TODO: the original NaN'd all the sensors at this point, but I'm real
    // suspicious of that.
}

void LiTimeMpptBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
    // TODO
}

void LiTimeMpptBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
    if (binary_sensor) {
        binary_sensor->publish_state(state);
    }
}

void LiTimeMpptBle::publish_state_(sensor::Sensor *sensor, float value) {
    if (sensor) {
        sensor->publish_state(value);
    }
}

/*
void LiTimeMpptBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void LiTimeMpptBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}
*/

}  // namespace litime_mppt_ble
}  // namespace esphome
