import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, binary_sensor, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["sensor", "binary_sensor", "text_sensor"]

litime_mppt_ble_ns = cg.esphome_ns.namespace("litime_mppt_ble")
LiTimeMpptBle = litime_mppt_ble_ns.class_(
    "LiTimeMpptBle", ble_client.BLEClientNode, cg.PollingComponent
)

MULTI_CONF = True

CONF_LITIME_MPPT_BLE_ID = "litime_mppt_ble_id"

LITIME_MPPT_BLE_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LITIME_MPPT_BLE_ID): cv.use_id(LiTimeMpptBle),
    }
)

CONF_ONLINE_STATUS = "online_status"
BINARY_SENSOR_NAMES = [
    CONF_ONLINE_STATUS,
]

CONF_PANEL_VOLTAGE = "panel_voltage"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_BATTERY_CURRENT = "battery_current"
CONF_BATTERY_POWER = "battery_power"
CONF_LOAD_VOLTAGE = "load_voltage"
CONF_LOAD_CURRENT = "load_current"
CONF_LOAD_POWER = "load_power"
CONF_CONTROLLER_TEMP = "controller_temperature"
CONF_ENERGY_DAY = "energy_production_today"
CONF_ENERGY = "energy_production_lifetime"
CONF_PEAK_POWER_TODAY = "peak_power_today"
CONF_DAYS_RUNNING = "days_running"

SENSOR_NAMES = [
    CONF_PANEL_VOLTAGE,
    CONF_BATTERY_VOLTAGE,
    CONF_BATTERY_CURRENT,
    CONF_BATTERY_POWER,
    CONF_LOAD_VOLTAGE,
    CONF_LOAD_CURRENT,
    CONF_LOAD_POWER,
    CONF_CONTROLLER_TEMP,
    CONF_ENERGY_DAY,
    CONF_ENERGY,
    CONF_PEAK_POWER_TODAY,
    CONF_DAYS_RUNNING,
]

CONF_MODE = "mode"
TEXT_SENSOR_NAMES = [
    CONF_MODE,
]


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LiTimeMpptBle),

            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement = UNIT_VOLT,
                accuracy_decimals = 1,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_CURRENT): sensor.sensor_schema(
                unit_of_measurement = UNIT_AMPERE,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_CURRENT,
                state_class = STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_POWER): sensor.sensor_schema(
                unit_of_measurement = UNIT_WATT,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_POWER,
                state_class = STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_LOAD_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement = UNIT_VOLT,
                accuracy_decimals = 1,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_LOAD_CURRENT): sensor.sensor_schema(
                unit_of_measurement = UNIT_AMPERE,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_CURRENT,
                state_class = STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_LOAD_POWER): sensor.sensor_schema(
                unit_of_measurement = UNIT_WATT,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_POWER,
                state_class = STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_CONTROLLER_TEMP): sensor.sensor_schema(
                unit_of_measurement = UNIT_CELSIUS,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_TEMPERATURE,
                state_class = STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_PANEL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement = UNIT_VOLT,
                accuracy_decimals = 1,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_ENERGY_DAY): sensor.sensor_schema(
                unit_of_measurement = UNIT_WATT_HOURS,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_ENERGY,
                state_class = STATE_CLASS_TOTAL_INCREASING,
            ),

            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                unit_of_measurement = UNIT_WATT_HOURS,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_ENERGY,
                state_class = STATE_CLASS_TOTAL_INCREASING,
            ),

            cv.Optional(CONF_PEAK_POWER_TODAY): sensor.sensor_schema(
                unit_of_measurement = UNIT_WATT,
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_POWER,
                state_class = STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_DAYS_RUNNING): sensor.sensor_schema(
                unit_of_measurement = "d",
                accuracy_decimals = 0,
                device_class = DEVICE_CLASS_DURATION,
                state_class = STATE_CLASS_TOTAL_INCREASING,
            ),
            cv.Optional(CONF_ONLINE_STATUS): binary_sensor.binary_sensor_schema(
                device_class = DEVICE_CLASS_CONNECTIVITY,
                entity_category = ENTITY_CATEGORY_DIAGNOSTIC,
            ),

            cv.Optional(CONF_MODE): text_sensor.text_sensor_schema(),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    for name in BINARY_SENSOR_NAMES:
        if conf := config.get(name):
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(var, f"set_{name}_binary_sensor")(sens))

    for name in SENSOR_NAMES:
        if conf := config.get(name):
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(var, f"set_{name}_sensor")(sens))

    for name in TEXT_SENSOR_NAMES:
        if conf := config.get(name):
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(var, f"set_{name}_text_sensor")(sens))

    #if CONF_MODE in config:
    #    sens = await text_sensor.new_text_sensor(config[CONF_MODE])
    #    fcg.add(var.set_mode(sens))

