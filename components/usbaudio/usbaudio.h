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

  void set_audio_output_mode(AudioOutputMode mode);
  void set_audio_output_mode(int mode);
  AudioOutputMode get_audio_output_mode() const { return audio_output_mode_; }
  bool is_usb_headset_connected() const { return usb_audio_connected_; }
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }

  void set_dp_pin(int dp_pin) { dp_pin_ = dp_pin; }  // Broche D+ (USB)
  void set_dm_pin(int dm_pin) { dm_pin_ = dm_pin; }  // Broche D- (USB)

 protected:
  void apply_audio_output_();
  bool detect_usb_audio_device_();
  void update_text_sensor();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  text_sensor::TextSensor *text_sensor_{nullptr};
  int dp_pin_{-1};  // Broche D+ (USB)
  int dm_pin_{-1};  // Broche D- (USB)
};

}  // namespace usbaudio
}  // namespace esphome









