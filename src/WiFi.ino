
const char *ssid = "xxx";
const char *password = "xxx";
bool connecting = false;

void wifi_setup()
{
    Serial.printf("[WiFi] Connecting...\n");
    WiFi.begin(ssid, password);
    connecting = true;
    led_set(0, 8, 8, 0);
}

void wifi_off()
{
    connecting = false;
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}

bool wifi_loop(void)
{
    int status = WiFi.status();
    int curTime = millis();
    static int nextTime = 0;
    static int stateCounter = 0;

    if (nextTime > curTime)
    {
        return false;
    }

    /* standard refresh time */
    nextTime = curTime + 500;

    /* when stuck at a state, disconnect */
    if (++stateCounter > 40)
    {
        Serial.printf("[WiFi] Timeout, aborting\n");
        led_set(0, 255, 0, 255);
        wifi_off();
        stateCounter = 0;
        return false;
    }

    switch (status)
    {
        case WL_CONNECTED:
            if (connecting)
            {
                led_set(0, 0, 4, 0);
                connecting = false;
                Serial.print("[WiFi] Connected, IP address: ");
                Serial.println(WiFi.localIP());
                stateCounter = 0;
            }
            else
            {
                static int last_rssi = -1;
                int rssi = WiFi.RSSI();

                if (last_rssi != rssi)
                {
                    float maxRssi = -40;
                    float minRssi = -90;
                    float strRatio = (rssi - minRssi) / (maxRssi - minRssi);
                    float strength = min(1, max(0, strRatio));
                    float brightness = 0.08f;
                    int r = brightness * 255.0f * (1.0f - strength);
                    int g = brightness * 255.0f * strength;

                    led_set(0, r, g, 0);
                    //Serial.printf("[WiFi] RSSI %d, strength: %1.2f, r: %d, g: %d\n", rssi, strength, r, g);

                    last_rssi = rssi;
                }

                /* happy with this state, reset counter */
                stateCounter = 0;
            }
            break;

        case WL_CONNECTION_LOST:
            Serial.printf("[WiFi] Connection lost\n");
            led_set(0, 32, 8, 0);
            wifi_off();
            break;

        case WL_CONNECT_FAILED:
            Serial.printf("[WiFi] Connection failed\n");
            led_set(0, 255, 0, 0);
            wifi_off();
            break;

        case WL_NO_SSID_AVAIL:
            Serial.printf("[WiFi] No SSID\n");
            led_set(0, 32, 0, 32);
            wifi_off();
            break;

        case WL_SCAN_COMPLETED:
            Serial.printf("[WiFi] Scan completed\n");
            wifi_off();
            break;

        case WL_DISCONNECTED:
            if (!connecting)
            {
                Serial.printf("[WiFi] Disconnected\n");
                led_set(0, 255, 0, 255);
                wifi_off();
            }
            break;

        case WL_IDLE_STATUS:
            if (!connecting)
            {
                connecting = true;
                Serial.printf("[WiFi]  Idle, connect to %s\n", ssid);
                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid, password);
            }
            else
            {
                Serial.printf("[WiFi]  Idle, connecting...\n");
            }
            break;

        case WL_NO_SHIELD:
            if (!connecting)
            {
                connecting = true;
                Serial.printf("[WiFi]  Disabled (%d), connecting to %s\n", status, ssid);
                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid, password);
            }
            break;

        default:
            Serial.printf("[WiFi]  unknown (%d), disable\n", status);
            wifi_off();
            break;
    }

    return false;
}
