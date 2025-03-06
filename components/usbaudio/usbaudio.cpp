#include "usbaudio.h"
#include "esphome/core/log.h"

// Inclusions nécessaires pour USB/UAC Host
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "usb/usb_types_stack.h"
#include "usb/usb_helpers.h"
#include "usb/uac_host.h" // Assurez-vous que ce fichier existe dans votre environnement ESP-IDF

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Gestionnaire d'événements USB
static bool s_usb_audio_connected = false;

// Callback pour les événements UAC
void uac_event_callback(const uac_host_event_t *event, void *arg) {
    USBAudioComponent *audio_comp = static_cast<USBAudioComponent*>(arg);
    
    switch (event->event_type) {
        case UAC_HOST_EVENT_DEVICE_CONNECTED:
            ESP_LOGI(TAG, "Périphérique audio USB connecté");
            s_usb_audio_connected = true;
            if (audio_comp) {
                audio_comp->handle_usb_connection_change(true);
            }
            break;
            
        case UAC_HOST_EVENT_DEVICE_DISCONNECTED:
            ESP_LOGI(TAG, "Périphérique audio USB déconnecté");
            s_usb_audio_connected = false;
            if (audio_comp) {
                audio_comp->handle_usb_connection_change(false);
            }
            break;
            
        default:
            break;
    }
}

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
    ESP_LOGD(TAG, "Initialisation du composant USB Audio avec UAC Host");
    
    // Configuration et initialisation du Host USB
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    
    // Configuration UAC
    uac_host_config_t uac_config = {
        .conn_callback = uac_event_callback,
        .conn_callback_arg = this,  // Passer this comme argument pour le callback
    };
    
    ESP_ERROR_CHECK(uac_host_install(&uac_config));
    
    // Démarrer la tâche client USB
    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = nullptr,  // UAC gère les événements
            .callback_arg = nullptr,
        }
    };
    
    ESP_ERROR_CHECK(usb_host_client_register(&client_config, &client_handle_));
    
    // État initial
    usb_audio_connected_ = s_usb_audio_connected;
    apply_audio_output_();
}

void USBAudioComponent::loop() {
    // Plus besoin de polling, les événements sont gérés par les callbacks
}

void USBAudioComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "USB Audio (UAC Host):");
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






