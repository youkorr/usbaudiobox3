import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import text_sensor
from esphome import pins

DEPENDENCIES = ["esp32"]

CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"
CONF_TEXT_SENSOR = "text_sensor"
CONF_DMINUS_PIN = "dminus_pin"
CONF_DPLUS_PIN = "dplus_pin"

AUDIO_OUTPUT_MODES = {
    "internal_speakers": 0,
    "usb_headset": 1,
    "auto_select": 2,
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="auto_select"): cv.enum(AUDIO_OUTPUT_MODES),
    cv.Optional(CONF_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
    cv.Required(CONF_DMINUS_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_DPLUS_PIN): pins.gpio_input_pin_schema,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    if CONF_AUDIO_OUTPUT_MODE in config:
        cg.add(var.set_audio_output_mode(AUDIO_OUTPUT_MODES[config[CONF_AUDIO_OUTPUT_MODE]]))

    if CONF_TEXT_SENSOR in config:
        sens = await cg.get_variable(config[CONF_TEXT_SENSOR])
        cg.add(var.set_text_sensor(sens))

    dminus_pin = await cg.gpio_pin_expression(config[CONF_DMINUS_PIN])
    cg.add(var.set_dminus_pin(dminus_pin))

    dplus_pin = await cg.gpio_pin_expression(config[CONF_DPLUS_PIN])
    cg.add(var.set_dplus_pin(dplus_pin))



