#pragma once

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"
#include "driver/i2s.h"
#include "driver/usb_host.h"

namespace esphome {
namespace usbaudio {

enum AudioOutputMode {
    INTERNAL_SPEAKERS,
    USB_HEADSET,
    AUTO_SELECT
};

class USBAudioComponent : public media_player::MediaPlayer, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Media Player Interface Methods
  media_player::MediaPlayerState get_state() override;
  void play() override;
  void pause() override;
  void stop() override;
  void set_volume(float volume) override;
  void set_media_url(const std::string &url) override;

  // ESP-BOX Specific Methods
  void set_audio_output_mode(AudioOutputMode mode);
  bool is_usb_headset_connected();

 private:
  // Internal state tracking
  media_player::MediaPlayerState current_state_ = media_player::MEDIA_PLAYER_STATE_IDLE;
  float current_volume_ = 0.5f;
  std::string current_media_url_;
  AudioOutputMode audio_output_mode_ = AUTO_SELECT;

  // Audio Hardware Interfaces
  void initialize_internal_speakers_();
  void initialize_usb_audio_();
  void switch_audio_output_(AudioOutputMode mode);

  // USB Audio Device Detection
  bool detect_usb_audio_device_();
  void handle_usb_audio_connection_();

  // I2S Configuration for Internal Speakers
  i2s_config_t i2s_config_ = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  // USB Host Configuration
  usb_host_config_t usb_host_config_ = {
    .skip_phy_setup = false,
    .intr_flags = ESP_INTR_FLAG_LEVEL2,
  };

  // USB Audio Device Handle
  usb_device_handle_t usb_audio_device_ = nullptr;
  bool usb_audio_connected_ = false;
};

}  // namespace usbaudio
}  // namespace esphome
