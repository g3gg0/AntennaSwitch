#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_MAGIC 0xE1AAFF00
typedef struct
{
    uint32_t magic;
    char hostname[32];
    uint32_t verbose;
    uint8_t rf1_cfg;
    uint8_t rf2_cfg;
    uint8_t rf3_cfg;
    uint8_t rf4_cfg;
    uint8_t rfo1_amp;
    uint8_t rfo2_amp;
} t_cfg;


extern t_cfg current_config;


#endif