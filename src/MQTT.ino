#define MQTT_DEBUG

#include <PubSubClient.h>

#define ARB_SERVER "xxx"
#define ARB_CLIENT "AntennaSwitch"
#define ARB_SERVERPORT 11883
#define ARB_USERNAME "xxx"
#define ARB_PW "xxx"

WiFiClient client;
PubSubClient mqtt(client);

extern uint32_t lon_rx_count;
extern uint32_t lon_crc_errors;

int mqtt_last_publish_time = 0;
int mqtt_lastConnect = 0;
int mqtt_retries = 0;
bool mqtt_fail = false;

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void mqtt_setup()
{
    mqtt.setServer(ARB_SERVER, ARB_SERVERPORT);
    mqtt.setCallback(callback);
}

void mqtt_publish_float(char *name, float value)
{
    char buffer[32];

    sprintf(buffer, "%0.2f", value);
    if (!mqtt.publish(name, buffer))
    {
        mqtt_fail = true;
    }
    Serial.printf("Published %s : %s\n", name, buffer);
}

void mqtt_publish_int(char *name, uint32_t value)
{
    char buffer[32];

    if (value == 0x7FFFFFFF)
    {
        return;
    }
    sprintf(buffer, "%d", value);
    mqtt.publish(name, buffer);
    Serial.printf("Published %s : %s\n", name, buffer);
}

bool mqtt_loop()
{
    uint32_t time = millis();
    static int nextTime = 0;

    if (mqtt_fail)
    {
        mqtt_fail = false;
        mqtt.disconnect();
    }

    MQTT_connect();

    if (!mqtt.connected())
    {
        return false;
    }

    mqtt.loop();

    if (time >= nextTime)
    {
        bool do_publish = false;

        if ((time - mqtt_last_publish_time) > 60000)
        {
            do_publish = true;
        }

        if (do_publish)
        {
            mqtt_last_publish_time = time;
            //mqtt_publish_int((char*)"feeds/integer/geiger/ticks", counts);
            //mqtt_publish_float((char*)"feeds/float/geiger/voltage", adc_voltage_avg);
        }
        nextTime = time + 1000;
    }

    return false;
}

void MQTT_connect()
{
    int curTime = millis();
    int8_t ret;

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if (mqtt.connected())
    {
        return;
    }

    if ((mqtt_lastConnect != 0) && (curTime - mqtt_lastConnect < (1000 << mqtt_retries)))
    {
        return;
    }

    mqtt_lastConnect = curTime;

    Serial.print("MQTT: Connecting to MQTT... ");
    ret = mqtt.connect(ARB_CLIENT, ARB_USERNAME, ARB_PW);

    if (ret == 0)
    {
        mqtt_retries++;
        if (mqtt_retries > 8)
        {
            mqtt_retries = 8;
        }
        Serial.printf("MQTT: (%d) ", mqtt.state());
        Serial.println("MQTT: Retrying MQTT connection");
        mqtt.disconnect();
    }
    else
    {
        Serial.println("MQTT Connected!");
    }
}
