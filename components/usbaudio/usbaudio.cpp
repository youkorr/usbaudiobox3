#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "usb/usb_host.h"

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode : uint8_t {
  AUTO_SELECT = 0,
  INTERNAL_SPEAKERS = 1,
  USB_HEADSET = 2
};

class USBAudioComponent : public Component {
 public:
  float get_setup_priority() const override { return setup_priority::LATE; }
  
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  void set_audio_output_mode(AudioOutputMode mode);
  void set_audio_output_mode(int mode);
  
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { text_sensor_ = text_sensor; }
  
  // Nouvelle méthode pour gérer les changements de connexion via callback
  void handle_usb_connection_change(bool connected);
  
 protected:
  void apply_audio_output_();
  void update_text_sensor();
  
  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  text_sensor::TextSensor *text_sensor_{nullptr};
  
  // Handle pour le client USB
  usb_host_client_handle_t client_handle_;
};

}  // namespace usbaudio
}  // namespace esphome






