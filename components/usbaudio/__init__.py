import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@your_github_username"]
DEPENDENCIES = ["esp32"]
MULTI_CONF = False

# Définir les modes de sortie audio
CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"
AUDIO_OUTPUT_MODES = {
    "usb_headset": "USB_HEADSET",
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

def validate_audio_output_mode(value):
    value = cv.string_strict(value)
    if value not in AUDIO_OUTPUT_MODES:
        raise cv.Invalid(f"Audio output mode must be one of {list(AUDIO_OUTPUT_MODES.keys())}")
    return value

CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(USBAudioComponent),
    cv.Required(CONF_AUDIO_OUTPUT_MODE): validate_audio_output_mode,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Enregistrer le composant
    yield cg.register_component(var, config)
    
    # Définir le mode de sortie audio
    audio_output_mode = config[CONF_AUDIO_OUTPUT_MODE]
    cg.add(var.set_audio_output_mode(AUDIO_OUTPUT_MODES[audio_output_mode]))

