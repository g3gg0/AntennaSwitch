
#include <FS.h>
#include <SPIFFS.h>

#include "Config.h"

t_cfg current_config;

void cfg_save()
{
    File file = SPIFFS.open("/config.dat", "w");
    if (!file || file.isDirectory())
    {
        Serial.printf("[cfg] Failed to write\n");
        return;
    }

    if (strlen(current_config.hostname) < 2)
    {
        strcpy(current_config.hostname, "AntennaSwitch");
    }

    file.write((uint8_t *)&current_config, sizeof(current_config));
    file.close();
    Serial.printf("[cfg] Written config\n");
}

void cfg_reset()
{
    memset(&current_config, 0x00, sizeof(current_config));

    current_config.magic = CONFIG_MAGIC;
    strcpy(current_config.hostname, "AntennaSwitch");

    current_config.rf1_cfg = 0;
    current_config.rf2_cfg = 0;
    current_config.rf3_cfg = 0;
    current_config.rf4_cfg = 0;
    current_config.rfo1_amp = 0;
    current_config.rfo2_amp = 0;
    current_config.verbose = 7;

    cfg_save();
}

void cfg_read()
{
    File file = SPIFFS.open("/config.dat", "r");

    if (!file || file.isDirectory())
    {
        Serial.printf("[cfg] Failed to read, resetting\n");
        cfg_reset();
    }
    else
    {
        file.read((uint8_t *)&current_config, sizeof(current_config));
        file.close();

        if (current_config.magic != CONFIG_MAGIC)
        {
            Serial.printf("[cfg] Invalid magic, resetting\n");
            cfg_reset();
        }
    }
}
