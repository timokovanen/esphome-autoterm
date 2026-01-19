import gc
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, text_sensor, switch, number, select
from esphome.const import CONF_ID, UNIT_CELSIUS, UNIT_VOLT, DEVICE_CLASS_TEMPERATURE, DEVICE_CLASS_VOLTAGE
from esphome.core import CORE

AUTO_LOAD = ["sensor", "text_sensor", "switch", "number", "select"]

autoterm_ns = cg.esphome_ns.namespace('autoterm')

AUTOTerm = autoterm_ns.class_('AUTOTerm', cg.Component)

VentilationSwitch = autoterm_ns.class_(
    'VentilationSwitch',
    switch.Switch,
    cg.Parented.template(AUTOTerm)
)

PowerSwitch = autoterm_ns.class_(
    'PowerSwitch',
    switch.Switch,
    cg.Parented.template(AUTOTerm)
)

PowerLevelNumber = autoterm_ns.class_(
    'PowerLevelNumber',
    number.Number,
    cg.Parented.template(AUTOTerm)
)
TemperatureSetpointNumber = autoterm_ns.class_(
    'TemperatureSetpointNumber',
    number.Number,
    cg.Parented.template(AUTOTerm)
)

ModeSelect = autoterm_ns.class_(
    "ModeSelect",
    select.Select,
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
CONF_POWER_SWITCH = "power_switch"

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
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_TEMPERATURE
    ),
    cv.Optional(CONF_PANEL_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_TEMPERATURE
    ),
    cv.Optional(CONF_EXTERNAL_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_TEMPERATURE
    ),
    cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_VOLTAGE
    ),
    cv.Optional(CONF_TEMPERATURE_SETPOINT): number.number_schema(TemperatureSetpointNumber),
    cv.Optional(CONF_POWER_LEVEL): number.number_schema(PowerLevelNumber),
    cv.Optional(CONF_OPERATING_STATE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_OPERATING_MODE): select.select_schema(ModeSelect),
    cv.Optional(CONF_VENTILATION): switch.switch_schema(VentilationSwitch),
    cv.Optional(CONF_POWER_SWITCH): switch.switch_schema(PowerSwitch),
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
    publish_topic = config.get(CONF_PUBLISH_TOPIC, f"{CORE.name}/autoterm/state")
    cg.add(var.set_publish_topic(cg.std_string(publish_topic)))
    subscribe_topic = config.get(CONF_SUBSCRIBE_TOPIC, f"{CORE.name}/autoterm/command")
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
    if CONF_OPERATING_STATE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_OPERATING_STATE])
        cg.add(var.set_operating_state_sensor(sens))
    if CONF_OPERATING_MODE in config:
        s = await select.new_select(
            config[CONF_OPERATING_MODE],
            options=["By Heater", "By Panel", "By External", "By Power"]
        )
        cg.add(s.set_parent(var))
        cg.add(var.set_operating_mode_select(s))
    if CONF_VENTILATION in config:
        sw = await switch.new_switch(config[CONF_VENTILATION])
        cg.add(sw.set_parent(var))
        cg.add(var.set_ventilation_switch(sw))
    if CONF_POWER_SWITCH in config:
        sw = await switch.new_switch(config[CONF_POWER_SWITCH])
        cg.add(sw.set_parent(var))
        cg.add(var.set_power_switch(sw))
    if CONF_POWER_LEVEL in config:
        num = await number.new_number(
            config[CONF_POWER_LEVEL],
            min_value=0.0,
            max_value=9.0,
            step=1.0,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_power_level_number(num))
    if CONF_TEMPERATURE_SETPOINT in config:
        num = await number.new_number(
            config[CONF_TEMPERATURE_SETPOINT],
            min_value=1.0,
            max_value=30.0,           
            step=1.0,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_temperature_setpoint_number(num))

    