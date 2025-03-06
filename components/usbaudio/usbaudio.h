#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  INTERNAL_SPEAKERS = 0,
  BLUETOOTH_OUTPUT = 1,
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
  bool is_bluetooth_connected() const { return bt_connected_; }
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }

 protected:
  void apply_audio_output_();
  void update_text_sensor_();
  
  // Bluetooth
  bool detect_bluetooth_dongle_();
  void initialize_bluetooth_a2dp_();
  void start_bluetooth_pairing_();
  static void bluetooth_a2dp_callback_(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  AudioOutputMode active_output_mode_{AudioOutputMode::INTERNAL_SPEAKERS};
  
  bool bluetooth_dongle_connected_{false};
  bool bt_initialized_{false};
  bool bt_connected_{false};
  
  text_sensor::TextSensor *text_sensor_{nullptr};
};

}  // namespace usbaudio
}  // namespace esphome



