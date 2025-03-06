#include "usbaudio.h"
#include "esphome/core/log.h"

// Inclusions nécessaires pour USB
#include "esp_err.h"
#include "esp_log.h"

#if defined(USE_ESP_IDF) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3))
#include "usb/usb_host.h"
#endif

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

#if defined(USE_ESP_IDF) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3))
// Variable statique pour suivre l'état de connexion USB
static bool s_usb_audio_connected = false;

// Déclaration des fonctions de callback
void client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
    USBAudioComponent *audio_comp = static_cast<USBAudioComponent*>(arg);
    
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            ESP_LOGI(TAG, "Nouveau périphérique USB détecté");
            // Ici, vous devriez idéalement vérifier si c'est un périphérique audio
            s_usb_audio_connected = true;
            if (audio_comp) {
                audio_comp->handle_usb_connection_change(true);
            }
            break;
            
        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            ESP_LOGI(TAG, "Périphérique USB déconnecté");
            s_usb_audio_connected = false;
            if (audio_comp) {
                audio_comp->handle_usb_connection_change(false);
            }
            break;
            
        default:
            break;
    }
}
#endif

void USBAudioComponent::set_audio_output_mode(AudioOutputMode mode) {
    if (audio_output_mode_ != mode) {
        audio_output_mode_ = mode;
        apply_audio_output_();
    }
}

void USBAudioComponent::set_audio_output_mode(int mode) {
    AudioOutputMode new_mode = static_cast<AudioOutputMode>(mode);
    if (new_mode != audio_output_mode_) {
        audio_output_mode_ = new_mode;
        apply_audio_output_();
    }
}

void USBAudioComponent::handle_usb_connection_change(bool connected) {
    if (usb_audio_connected_ != connected) {
        usb_audio_connected_ = connected;
        apply_audio_output_();
        update_text_sensor();
    }
}

void USBAudioComponent::apply_audio_output_() {
    AudioOutputMode effective_mode = audio_output_mode_;
    if (effective_mode == AudioOutputMode::AUTO_SELECT) {
        effective_mode = usb_audio_connected_ ? AudioOutputMode::USB_HEADSET
                                             : AudioOutputMode::INTERNAL_SPEAKERS;
    }
    
    switch (effective_mode) {
        case AudioOutputMode::INTERNAL_SPEAKERS:
            ESP_LOGD(TAG, "Activation des haut-parleurs internes");
            // Ajoutez ici le code pour basculer vers les haut-parleurs internes
            break;
            
        case AudioOutputMode::USB_HEADSET:
            ESP_LOGD(TAG, "Activation du casque USB");
            // Ajoutez ici le code pour basculer vers le casque USB
            break;
            
        default:
            ESP_LOGE(TAG, "Mode audio inconnu");
            break;
    }
}

void USBAudioComponent::setup() {
    ESP_LOGD(TAG, "Initialisation du composant USB Audio");
    
#if defined(USE_ESP_IDF) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3))
    // Configuration et initialisation du Host USB
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    
    // Démarrer la tâche client USB
    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = client_event_callback,
            .callback_arg = this,
        }
    };
    
    ESP_ERROR_CHECK(usb_host_client_register(&client_config, &client_handle_));
#else
    // Simuler une connexion pour les plateformes sans support USB
    usb_audio_connected_ = false;
#endif
    
    // Appliquer la configuration initiale
    apply_audio_output_();
}

void USBAudioComponent::loop() {
#if defined(USE_ESP_IDF) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3))
    // Les événements sont gérés par le callback
#else
    // Pour les plateformes sans support USB, rien à faire ici
#endif
}

void USBAudioComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "USB Audio:");
    ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
    ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
    if (text_sensor_ != nullptr) {
        text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
    }
}

}  // namespace usbaudio
}  // namespace esphome






