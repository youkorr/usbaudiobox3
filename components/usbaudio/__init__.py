import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import text_sensor

# Dependencies
AUTO_LOAD = ["text_sensor"]
DEPENDENCIES = ["esp32"]

# Configuration constants
CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"
CONF_TEXT_SENSOR = "text_sensor"

# Audio output mode options
AUDIO_OUTPUT_MODES = {
    "internal_speakers": 0,
    "usb_headset": 1,
    "auto_select": 2,
}

# Namespace setup
NAMESPACE = cg.esphome_ns
usbaudio_ns = NAMESPACE.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

# Configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="auto_select"): cv.enum(AUDIO_OUTPUT_MODES),
    cv.Optional(CONF_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
}).extend(cv.COMPONENT_SCHEMA)

# Code generation
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    if CONF_AUDIO_OUTPUT_MODE in config:
        cg.add(var.set_audio_output_mode(AUDIO_OUTPUT_MODES[config[CONF_AUDIO_OUTPUT_MODE]]))
    
    if CONF_TEXT_SENSOR in config:
        sens = await cg.get_variable(config[CONF_TEXT_SENSOR])
        cg.add(var.set_text_sensor(sens))

