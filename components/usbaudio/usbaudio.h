#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  INTERNAL_SPEAKERS = 0,
  USB_HEADSET = 1,
  AUTO_SELECT = 2
};

class USBAudioComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_audio_output_mode(AudioOutputMode mode);
  AudioOutputMode get_audio_output_mode() const { return audio_output_mode_; }
  bool is_usb_headset_connected() const { return usb_audio_connected_; }

 protected:
  void handle_usb_audio_connection_();
  void apply_audio_output_();
  bool detect_usb_audio_device_();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
};

}  // namespace usbaudio
}  // namespace esphome

