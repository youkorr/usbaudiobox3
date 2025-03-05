#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/uac_host/uac_host.h"  // Assurez-vous que cette ligne est présente

namespace esphome {
namespace usbaudio {

enum class AudioOutputMode {
  INTERNAL_SPEAKERS = 0,
  USB_HEADSET = 1,
  AUTO_SELECT = 2
};

class USBAudioComponent : public Component, public uac_host::UACHostListener {  // Ajoutez UACHostListener
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_audio_output_mode(AudioOutputMode mode);
  AudioOutputMode get_audio_output_mode() const { return audio_output_mode_; }
  bool is_usb_headset_connected() const { return usb_audio_connected_; }

  // Méthodes de UACHostListener
  void on_uac_connected() override;
  void on_uac_disconnected() override;
  void on_uac_streaming_started() override;
  void on_uac_streaming_stopped() override;

 protected:
  void handle_usb_audio_connection_();
  void apply_audio_output_();

  AudioOutputMode audio_output_mode_{AudioOutputMode::AUTO_SELECT};
  bool usb_audio_connected_{false};
  bool usb_audio_streaming_{false};
};

}  // namespace usbaudio
}  // namespace esphome

