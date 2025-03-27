usbaudio.h Header File

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"

#ifdef CONFIG_ESP32_S3_USB_OTG

namespace esphome {
namespace usbaudio {

// Enum to represent audio player types
enum audio_player_t {
    AUDIO_PLAYER_I2S = 0,
    AUDIO_PLAYER_USB
};

// Audio player callback context
struct audio_player_cb_ctx_t {
    enum {
        AUDIO_PLAYER_CALLBACK_EVENT_IDLE = 0,
        AUDIO_PLAYER_CALLBACK_EVENT_PLAYING,
        AUDIO_PLAYER_CALLBACK_EVENT_PAUSE
    } audio_event;
};

// Mute settings enum
enum AUDIO_PLAYER_MUTE_SETTING {
    AUDIO_PLAYER_UNMUTE = 1,
    AUDIO_PLAYER_MUTE = 0
};

// Function declarations
audio_player_t get_audio_player_type(void);
void *get_audio_player_handle(void);
uint8_t get_sys_volume(void);

// USB Audio Component
class USBAudioComponent : public Component {
public:
    void setup() override;
    void loop() override;
    void dump_config() override;

    // Optional: Media player interface methods
    void play();
    void pause();
    void stop();
    void set_volume(float volume);

private:
    // Internal state tracking
    bool is_usb_connected_ = false;
    float current_volume_ = 1.0;
};

} // namespace usbaudio
} // namespace esphome

#endif // CONFIG_ESP32_S3_USB_OTG

