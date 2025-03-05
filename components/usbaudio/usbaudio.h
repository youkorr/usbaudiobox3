#pragma once

#include "esphome/core/component.h"
#include "driver/usb_host.h"
#include "driver/i2s.h"

namespace esphome {
namespace usbaudio {

enum AudioOutputMode {
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
  bool is_usb_headset_connected();
  void play();
  void stop();

 protected:
  void initialize_internal_speakers_();
  void initialize_usb_audio_();
  void handle_usb_audio_connection_();
  bool detect_usb_audio_device_();
  void switch_audio_output_(AudioOutputMode mode);

  AudioOutputMode audio_output_mode_{AUTO_SELECT};
  bool usb_audio_connected_{false};
};

}  // namespace usbaudio
}  // namespace esphome

