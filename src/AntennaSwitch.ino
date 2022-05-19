
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>

#include "Config.h"


int led_r = 0;
int led_g = 0;
int led_b = 0;
uint8_t switches[] = { 4, 18, 19, 21, 22, 23, 32, 33, 25, 13, 26, 14 };

#define SW1 0
#define SW2 1
#define SW3 2
#define SW4 3
#define SW5 4
#define SW6 5
#define SW7 6
#define SW8 7
#define SW9 8
#define SW10 9
#define SW11 10
#define SW12 11


#define SW1_1 SW8
#define SW1_2 SW7
#define SW2_1 SW6
#define SW2_2 SW5
#define SW3_1 SW4
#define SW3_2 SW3
#define SW4_1 SW1
#define SW4_2 SW2
#define SW5_1 SW11
#define SW5_2 SW10
#define SW6_1 SW12
#define SW6_2 SW9

void setup()
{
    Serial.begin(115200);
    Serial.printf("\n\n\n");

    Serial.printf("[i] SDK:          '%s'\n", ESP.getSdkVersion());
    Serial.printf("[i] CPU Speed:    %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("[i] Chip Id:      %06llX\n", ESP.getEfuseMac());
    Serial.printf("[i] Flash Mode:   %08X\n", ESP.getFlashChipMode());
    Serial.printf("[i] Flash Size:   %08X\n", ESP.getFlashChipSize());
    Serial.printf("[i] Flash Speed:  %d MHz\n", ESP.getFlashChipSpeed() / 1000000);
    Serial.printf("[i] Heap          %d/%d\n", ESP.getFreeHeap(), ESP.getHeapSize());
    Serial.printf("[i] SPIRam        %d/%d\n", ESP.getFreePsram(), ESP.getPsramSize());
    Serial.printf("\n");
    Serial.printf("[i] Starting\n");

    Serial.printf("[i]   Setup LEDs\n");
    led_setup();
    Serial.printf("[i]   Setup SPIFFS\n");
    if (!SPIFFS.begin(true))
    {
        Serial.println("[E]   SPIFFS Mount Failed");
    }
    cfg_read();
    Serial.printf("[i]   Setup WiFi\n");
    wifi_setup();
    Serial.printf("[i]   Setup Webserver\n");
    www_setup();
    Serial.printf("[i]   Setup Time\n");
    time_setup();
    Serial.printf("[i]   Setup MQTT\n");
    mqtt_setup();

    for(int sw = 0; sw < sizeof(switches); sw++)
    {
        pinMode(switches[sw], OUTPUT);
        digitalWrite(switches[sw], LOW);
    }

    Serial.println("Setup done");
}

void mux_write(int pin1, int pin2, int mode)
{
    digitalWrite(switches[pin1], (mode & 2) ? HIGH : LOW);
    digitalWrite(switches[pin2], (mode & 1) ? HIGH : LOW);
}

void amp_write(int pin1, int pin2, int enabled)
{
    digitalWrite(switches[pin1], enabled ? LOW : HIGH);
    digitalWrite(switches[pin2], enabled ? HIGH : LOW);
}

void loop()
{
    bool hasWork = false;

    hasWork |= led_loop();
    hasWork |= wifi_loop();
    hasWork |= www_loop();
    hasWork |= time_loop();
    hasWork |= mqtt_loop();
    hasWork |= ota_loop();

    mux_write(SW1_1, SW1_2, current_config.rf1_cfg);
    mux_write(SW2_1, SW2_2, current_config.rf2_cfg);
    mux_write(SW3_1, SW3_2, current_config.rf3_cfg);
    mux_write(SW4_1, SW4_2, current_config.rf4_cfg);
    amp_write(SW5_1, SW5_2, current_config.rfo1_amp);
    amp_write(SW6_1, SW6_2, current_config.rfo2_amp);
}
