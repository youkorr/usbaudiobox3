import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["text_sensor"]

usbaudio_ns = cg.esphome_ns.namespace("usbaudio")
USBAudioComponent = usbaudio_ns.class_("USBAudioComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(USBAudioComponent),
        cv.Optional("text_sensor"): cv.use_id(text_sensor.TextSensor),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    if "text_sensor" in config:
        sens = await cg.get_variable(config["text_sensor"])
        cg.add(var.set_text_sensor(sens))


