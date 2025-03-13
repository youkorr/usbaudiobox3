#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/text_sensor/text_sensor.h"

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
  void set_audio_output_mode(int mode);
  AudioOutputMode get_audio_output_mode() const { return audio_output_mode_; }
  bool is_usb_headset_connected() const { return usb_audio_connected_; }
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }

 protected:
  void handle_usb_audio_connection_();
  void apply_audio_output_();
  bool detect_usb_audio_device_();
  void update_text_sensor();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  text_sensor::TextSensor *text_sensor_{nullptr};
};

}  // namespace usbaudio
}  // namespace esphome















