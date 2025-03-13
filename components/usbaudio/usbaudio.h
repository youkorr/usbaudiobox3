#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "usb/usb_host.h"

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

  void set_dminus_pin(GPIOPin *pin) { dminus_pin_ = pin; }
  void set_dplus_pin(GPIOPin *pin) { dplus_pin_ = pin; }
  void set_audio_output_mode(AudioOutputMode mode) { audio_output_mode_ = mode; }
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }

 protected:
  void handle_usb_audio_connection_();
  void apply_audio_output_();
  bool detect_usb_audio_device_();
  void update_text_sensor();
  void switch_to_usb_audio();
  void switch_to_speaker();
  void configure_gpio();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  bool last_state_{false};
  text_sensor::TextSensor *text_sensor_{nullptr};
  GPIOPin *dminus_pin_{nullptr};
  GPIOPin *dplus_pin_{nullptr};
};

}  // namespace usbaudio
}  // namespace esphome













