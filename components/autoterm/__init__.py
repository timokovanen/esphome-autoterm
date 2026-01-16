import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, text_sensor, switch, number
from esphome.const import CONF_ID
from esphome.core import CORE

AUTO_LOAD = ["switch", "number"]

autoterm_ns = cg.esphome_ns.namespace('autoterm')
switches_ns = autoterm_ns.namespace('switches')
numbers_ns = autoterm_ns.namespace('numbers')

AUTOTerm = autoterm_ns.class_('AUTOTerm', cg.Component)

VentilationSwitch = switches_ns.class_(
    'VentilationSwitch',
    switch.Switch,
    cg.Parented.template(AUTOTerm)
)

PowerLevelNumber = numbers_ns.class_(
    'PowerLevelNumber',
    number.Number,
    cg.Parented.template(AUTOTerm)
)

CONF_UART_PANEL = "uart_panel"
CONF_UART_HEATER = "uart_heater"
CONF_TIMEOUT_MS = "timeout_ms"
CONF_BUFFER_SIZE = "buffer_size"
CONF_PUBLISH_TOPIC = "publish_topic"
CONF_SUBSCRIBE_TOPIC = "subscribe_topic"
CONF_REFRESH_MS = "refresh_ms"
CONF_HEATER_TEMPERATURE = "heater_temperature"
CONF_PANEL_TEMPERATURE = "panel_temperature"
CONF_EXTERNAL_TEMPERATURE = "external_temperature"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_TEMPERATURE_SETPOINT = "temperature_setpoint"
CONF_POWER_LEVEL = "power_level"
CONF_OPERATING_STATE = "operating_state"
CONF_OPERATING_MODE = "operating_mode"
CONF_VENTILATION = "ventilation"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AUTOTerm),
    cv.Required(CONF_UART_PANEL): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_UART_HEATER): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_TIMEOUT_MS, default=20): cv.int_range(min=1, max=2000),
    cv.Optional(CONF_BUFFER_SIZE, default=256): cv.int_range(min=16, max=4096),
    cv.Optional(CONF_PUBLISH_TOPIC): cv.string,
    cv.Optional(CONF_SUBSCRIBE_TOPIC): cv.string,
    cv.Optional(CONF_REFRESH_MS, default=30000): cv.int_range(min=5000, max=60000),
    cv.Optional(CONF_HEATER_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement="°C",
        accuracy_decimals=0,
        device_class="temperature"
    ),
    cv.Optional(CONF_PANEL_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement="°C",
        accuracy_decimals=0,
        device_class="temperature"
    ),
    cv.Optional(CONF_EXTERNAL_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement="°C",
        accuracy_decimals=0,
        device_class="temperature"
    ),
    cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement="V",
        accuracy_decimals=1,
        device_class="voltage"
    ),
    cv.Optional(CONF_TEMPERATURE_SETPOINT): sensor.sensor_schema(
        unit_of_measurement="°C",
        accuracy_decimals=0,
        device_class="temperature"
    ),
    cv.Optional(CONF_POWER_LEVEL): number.number_schema(PowerLevelNumber),
    cv.Optional(CONF_OPERATING_STATE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_OPERATING_MODE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_VENTILATION): switch.switch_schema(VentilationSwitch),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    uart_panel = await cg.get_variable(config[CONF_UART_PANEL])
    uart_heater = await cg.get_variable(config[CONF_UART_HEATER])
    cg.add(var.set_uart_panel(uart_panel))
    cg.add(var.set_uart_heater(uart_heater))
    cg.add(var.set_timeout_ms(config[CONF_TIMEOUT_MS]))
    cg.add(var.set_buffer_size(config[CONF_BUFFER_SIZE]))
    publish_topic = config.get(CONF_PUBLISH_TOPIC, f"{CORE.name}/autoterm")
    cg.add(var.set_publish_topic(cg.std_string(publish_topic)))
    subscribe_topic = config.get(CONF_SUBSCRIBE_TOPIC, f"{CORE.name}/autoterm/cmd")
    cg.add(var.set_subscribe_topic(cg.std_string(subscribe_topic)))
    cg.add(var.set_refresh_ms(config[CONF_REFRESH_MS]))

    # Register sensors
    if CONF_HEATER_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_HEATER_TEMPERATURE])
        cg.add(var.set_heater_temperature_sensor(sens))
    if CONF_PANEL_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_PANEL_TEMPERATURE])
        cg.add(var.set_panel_temperature_sensor(sens))
    if CONF_EXTERNAL_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_EXTERNAL_TEMPERATURE])
        cg.add(var.set_external_temperature_sensor(sens))
    if CONF_BATTERY_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage_sensor(sens))
    if CONF_TEMPERATURE_SETPOINT in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_SETPOINT])
        cg.add(var.set_temperature_setpoint_sensor(sens))
    if CONF_OPERATING_STATE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_OPERATING_STATE])
        cg.add(var.set_operating_state_sensor(sens))
    if CONF_OPERATING_MODE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_OPERATING_MODE])
        cg.add(var.set_operating_mode_sensor(sens))
    if CONF_VENTILATION in config:
        sw = await switch.new_switch(config[CONF_VENTILATION])
        cg.add(sw.set_parent(var))
        cg.add(var.set_ventilation_switch(sw))
    if CONF_POWER_LEVEL in config:
        num = await number.new_number(
            config[CONF_POWER_LEVEL],
            min_value=0.0,
            max_value=9.0,
            step=1.0,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_power_level_number(num))
        # Lock traits: 0..9 step 1 (integers)
        # cg.add(num.set_min_value(0.0))
        # cg.add(num.set_max_value(9.0))
        # cg.add(num.set_step(1.0))
        # Optional: set mode if you prefer slider vs box (defaults are fine)
        # from esphome.components.number import NumberMode
        # cg.add(num.set_mode(number.NumberMode.BOX))
