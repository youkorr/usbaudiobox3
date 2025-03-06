#include "usbaudio.h"
#include "esphome/core/log.h"

#include "esp_log.h"
#include "usb/usb_host.h"


#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_gap_bt_api.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

static const uint8_t USB_CLASS_WIRELESS = 0xE0;
static const uint8_t USB_SUBCLASS_BLUETOOTH = 0x01;

void USBAudioComponent::setup() {
    ESP_LOGD(TAG, "Configuration du composant Audio Bluetooth");

    if (USB.begin()) {
        ESP_LOGI(TAG, "USB Host initialisé avec succès");
    } else {
        ESP_LOGE(TAG, "Échec de l'initialisation de USB Host");
    }

    bluetooth_dongle_connected_ = detect_bluetooth_dongle_();

    if (bluetooth_dongle_connected_) {
        ESP_LOGI(TAG, "Initialisation du Bluetooth A2DP");
        initialize_bluetooth_a2dp_();
    }
    apply_audio_output_();
}

bool USBAudioComponent::detect_bluetooth_dongle_() {
    ESP_LOGD(TAG, "Vérification du dongle Bluetooth USB");
    if (USB.connected()) {
        uint8_t dev_count = USBH.getDeviceCount();
        for (uint8_t i = 0; i < dev_count; i++) {
            uint8_t dev_addr = USBH.getAddress(i);
            if (USBH.getDeviceClass(dev_addr) == USB_CLASS_WIRELESS &&
                USBH.getDeviceSubClass(dev_addr) == USB_SUBCLASS_BLUETOOTH) {
                ESP_LOGI(TAG, "Dongle Bluetooth USB détecté");
                return true;
            }
        }
    }
    return false;
}

void USBAudioComponent::initialize_bluetooth_a2dp_() {
    esp_err_t ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "Erreur init BT: %s", esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "Erreur enable BT: %s", esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "Erreur init Bluedroid: %s", esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "Erreur enable Bluedroid: %s", esp_err_to_name(ret));
        return;
    }

    esp_a2d_register_callback(bluetooth_a2dp_callback_);
    esp_a2d_source_init();
    esp_bt_dev_set_device_name("ESP32-S3-BOX-AUDIO");
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
}

void USBAudioComponent::apply_audio_output_() {
    if (bluetooth_dongle_connected_ && bt_connected_) {
        ESP_LOGI(TAG, "Utilisation du Bluetooth");
    } else {
        ESP_LOGI(TAG, "Utilisation des haut-parleurs internes");
    }
}

void USBAudioComponent::start_bluetooth_pairing_() {
    ESP_LOGI(TAG, "Mode appairage activé");
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
}

void USBAudioComponent::bluetooth_a2dp_callback_(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    if (event == ESP_A2D_CONNECTION_STATE_EVT) {
        bt_connected_ = (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED);
        ESP_LOGI(TAG, "État A2DP: %s", bt_connected_ ? "Connecté" : "Déconnecté");
    }
}

}  // namespace usbaudio
}  // namespace esphome







