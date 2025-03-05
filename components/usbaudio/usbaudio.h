pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_usb_serial_jtag.h"

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  INTERNAL_SPEAKERS = 0,
  USB_HEADSET = 1,
  AUTO_SELECT = 2
};

class USBAudioComponent : public Component {
  // ... (rest of the class definition)
};

}  // namespace usbaudio
}  // namespace esphome

