import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor, usb_host
from esphome.const import CONF_ID

DEPENDENCIES = ["usb_host"]  # Re-add usb_host dependency

CONF_DP = "dp"  # Broche D+ (USB)
CONF_DM = "dm"  # Broche D- (USB)
CONF_TEXT_SENSOR = "text_sensor"

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Required(CONF_DP): cv.int_,  # Broche D+ (USB)
    cv.Required(CONF_DM): cv.int_,  # Broche D- (USB)
    cv.Optional(CONF_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
}).extend(cv.COMPONENT_SCHEMA).extend(usb_host.USB_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Configuration des broches GPIO pour USB
    cg.add(var.set_dp_pin(config[CONF_DP]))
    cg.add(var.set_dm_pin(config[CONF_DM]))
    
    # Integrate with USB host component
    await usb_host.setup_usb_host(var, config)
    
    if CONF_TEXT_SENSOR in config:
        sens = await cg.get_variable(config[CONF_TEXT_SENSOR])
        cg.add(var.set_text_sensor(sens))



