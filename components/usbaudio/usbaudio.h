#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  AUTO_SELECT = 0,
  INTERNAL_SPEAKERS = 1,
  USB_HEADSET = 2,
};

class USBAudioComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  void set_audio_output_mode(AudioOutputMode mode);
  void set_audio_output_mode(int mode);
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }
  bool is_usb_audio_connected() const { return usb_audio_connected_; }
  void update_text_sensor();
  
  // Handlers for USB events
  void handle_device_connection();
  void handle_device_disconnection();

 protected:
  bool detect_usb_audio_device_();
  void apply_audio_output_();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  text_sensor::TextSensor *text_sensor_{nullptr};
};

}  // namespace usbaudio
}  // namespace esphome

