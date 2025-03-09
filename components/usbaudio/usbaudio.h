#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h" // Include TextSensor

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  AUTO_SELECT,
  INTERNAL_SPEAKERS,
  USB_HEADSET
};

class USBAudioComponent : public Component {
 public:
  void set_audio_output_mode(AudioOutputMode mode);
  void set_audio_output_mode(int mode); // Pour les automations utilisant des entiers

  void set_text_sensor(esphome::text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  void handle_device_connection();
  void handle_device_disconnection();

 protected:
  bool detect_usb_audio_device_();
  void apply_audio_output_();
  void enable_internal_speaker_(bool enable);
  void update_text_sensor();

  AudioOutputMode audio_output_mode_ = AudioOutputMode::AUTO_SELECT;
  bool usb_audio_connected_ = false;
  esphome::text_sensor::TextSensor *text_sensor_ = nullptr;
};

}  // namespace usbaudio
}  // namespace esphome








