#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
//#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
//#include "esphome/components/switch/switch.h"
//#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace litime_mppt_ble {

namespace espbt = esphome::esp32_ble_tracker;

class LiTimeMpptBle : public esphome::ble_client::BLEClientNode, public PollingComponent {
    public:
        void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                esp_ble_gattc_cb_param_t *param) override;
        void dump_config() override;
        void update() override;
        float get_setup_priority() const override { return setup_priority::DATA; }

        void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
            online_status_binary_sensor_ = online_status_binary_sensor;
        }

        void set_panel_voltage_sensor(sensor::Sensor *s) {
            this->panel_voltage_sensor_ = s;
        }
        void set_battery_voltage_sensor(sensor::Sensor *s) {
            this->battery_voltage_sensor_ = s;
        }
        void set_battery_current_sensor(sensor::Sensor *s) {
            this->battery_current_sensor_ = s;
        }
        void set_battery_power_sensor(sensor::Sensor *s) {
            this->battery_power_sensor_ = s;
        }
        void set_load_voltage_sensor(sensor::Sensor *s) {
            this->load_voltage_sensor_ = s;
        }
        void set_load_current_sensor(sensor::Sensor *s) {
            this->load_current_sensor_ = s;
        }
        void set_load_power_sensor(sensor::Sensor *s) {
            this->load_power_sensor_ = s;
        }
        void set_controller_temperature_sensor(sensor::Sensor *s) {
            this->controller_temperature_sensor_ = s;
        }
        void set_energy_production_today_sensor(sensor::Sensor *s) {
            this->energy_production_today_sensor_ = s;
        }
        void set_energy_production_lifetime_sensor(sensor::Sensor *s) {
            this->energy_production_lifetime_sensor_ = s;
        }
        void set_peak_power_today_sensor(sensor::Sensor *s) {
            this->peak_power_today_sensor_ = s;
        }
        void set_days_running_sensor(sensor::Sensor *s) {
            this->days_running_sensor_ = s;
        }

        void on_modbus_response(const std::vector<uint8_t> &data);
        void assemble(const uint8_t *data, uint16_t length);

    protected:
        binary_sensor::BinarySensor *online_status_binary_sensor_;

        sensor::Sensor *panel_voltage_sensor_;
        sensor::Sensor *battery_voltage_sensor_;
        sensor::Sensor *battery_current_sensor_;
        sensor::Sensor *battery_power_sensor_;
        sensor::Sensor *load_voltage_sensor_;
        sensor::Sensor *load_current_sensor_;
        sensor::Sensor *load_power_sensor_;
        sensor::Sensor *controller_temperature_sensor_;
        sensor::Sensor *energy_production_today_sensor_;
        sensor::Sensor *energy_production_lifetime_sensor_;
        sensor::Sensor *peak_power_today_sensor_;
        sensor::Sensor *days_running_sensor_;

        std::vector<uint8_t> frame_buffer_;
        uint16_t char_handle_;
        uint8_t no_response_count_{0};

        void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
        void publish_state_(sensor::Sensor *sensor, float value);
        void publish_device_unavailable_();
        void track_online_status_();
        void reset_online_status_tracker_();
        void begin_read_registers();
};

}  // namespace litime_mppt_ble
}  // namespace esphome

#endif
