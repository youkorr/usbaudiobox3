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

  void set_audio_output_mode(AudioOutputMode mode) { audio_output_mode_ = mode; }
  void set_audio_output_mode(int mode) { audio_output_mode_ = static_cast<AudioOutputMode>(mode); }
  bool is_usb_headset_connected() const { return usb_audio_connected_; }

 protected:
  void handle_usb_audio_connection_();
  bool detect_usb_audio_device_();
  void switch_audio_output_(AudioOutputMode mode);

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
};

}  // namespace usbaudio
}  // namespace esphome

