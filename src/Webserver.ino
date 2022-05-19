
#include <WebServer.h>
#include "Config.h"

WebServer webserver(80);

#define min(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define max(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

void www_setup()
{
    webserver.on("/", handle_OnConnect);
    webserver.on("/set_parm", handle_set_parm);
    webserver.on("/ota", handle_ota);
    webserver.on("/reset", handle_reset);
    webserver.onNotFound(handle_NotFound);

    webserver.begin();
    Serial.println("HTTP server started");

    if (!MDNS.begin(current_config.hostname))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    MDNS.addService("http", "tcp", 80);
}

bool www_loop()
{
    webserver.handleClient();
    return false;
}

void handle_OnConnect()
{
    webserver.send(200, "text/html", SendHTML());
}


void handle_ota()
{
    ota_setup();
    webserver.send(200, "text/html", SendHTML());
}

void handle_plot()
{
    File dataFile = SPIFFS.open("/plot.html", "r");
    webserver.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handle_reset()
{
    webserver.send(200, "text/html", SendHTML());
    ESP.restart();
}

void handle_set_parm()
{
    current_config.rf1_cfg = max(0, min(3, webserver.arg("rf1_cfg").toInt()));
    current_config.rf2_cfg = max(0, min(3, webserver.arg("rf2_cfg").toInt()));
    current_config.rf3_cfg = max(0, min(3, webserver.arg("rf3_cfg").toInt()));
    current_config.rf4_cfg = max(0, min(3, webserver.arg("rf4_cfg").toInt()));
    current_config.rfo1_amp = max(0, min(1, webserver.arg("rfo1_amp").toInt()));
    current_config.rfo2_amp = max(0, min(1, webserver.arg("rfo2_amp").toInt()));
    current_config.verbose = max(1, min(99, webserver.arg("verbose").toInt()));

    strncpy(current_config.hostname, webserver.arg("hostname").c_str(), sizeof(current_config.hostname));

    cfg_save();

    if (current_config.verbose)
    {
        Serial.printf("Config:\n");
        Serial.printf("  verbose:          %d\n", current_config.verbose);
    }
    webserver.send(200, "text/html", SendHTML());
}

void handle_NotFound()
{
    webserver.send(404, "text/plain", "Not found");
}

String SendHTML()
{
    char buf[1024];

    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";

    sprintf(buf, "<title>AntennaSwitch Control</title>\n");

    ptr += buf;
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    ptr += ".button-on {background-color: #3498db;}\n";
    ptr += ".button-on:active {background-color: #2980b9;}\n";
    ptr += ".button-off {background-color: #34495e;}\n";
    ptr += ".button-off:active {background-color: #2c3e50;}\n";
    ptr += ".toggle-buttons input[type=\"radio\"] {visibility: hidden;}\n";
    ptr += ".toggle-buttons label { border: 1px solid #333; border-radius: 0.5em; padding: 0.3em; } \n";
    ptr += ".toggle-buttons input:checked + label { background: #40ff40; color: #116600; box-shadow: none; }\n";
    ptr += "input:hover + label, input:focus + label { background: #ff4040; } \n";
    ptr += ".together { position: relative; } \n";
    ptr += ".together input { position: absolute; width: 1px; height: 1px; top: 0; left: 0; } \n";
    ptr += ".together label { margin: 0.5em 0; border-radius: 0; } \n";
    ptr += ".together label:first-of-type { border-radius: 0.5em 0 0 0.5em; } \n";
    ptr += ".together label:last-of-type { border-radius: 0 0.5em 0.5em 0; } \n";
    ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n"; 
    ptr += "td {padding: 0.3em}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";

    sprintf(buf, "<h1>AntennaSwitch</h1>\n");

    ptr += buf;
    if (!ota_enabled())
    {
        ptr += "<a href=\"/ota\">[Enable OTA]</a> ";
    }
    ptr += buf;
    ptr += "<br><br>\n";

    ptr += "<form action=\"/set_parm\">\n";
    ptr += "<table>";

#define ADD_CONFIG(name, value, fmt, desc)                                                                                \
    do                                                                                                                    \
    {                                                                                                                     \
        ptr += "<tr><td><label for=\"" name "\">" desc ":</label></td>";                                                  \
        sprintf(buf, "<td><input type=\"text\" id=\"" name "\" name=\"" name "\" value=\"" fmt "\"></td></tr>\n", value); \
        ptr += buf;                                                                                                       \
    } while (0)

#define ADD_CONFIG_RADIO4(name, value, fmt, desc, text0, text1, text2, text3) \
    do \
    { \
        ptr += "<tr><td>" desc ":</td><td><div class=\"toggle-buttons together\">"; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c0\" name=\"" name "\" value=\"0\" %s>\n", (value==0)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c0\">" text0 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c1\" name=\"" name "\" value=\"1\" %s>\n", (value==1)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c1\">" text1 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c2\" name=\"" name "\" value=\"2\" %s>\n", (value==2)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c2\">" text2 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c3\" name=\"" name "\" value=\"3\" %s>\n", (value==3)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c3\">" text3 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "</div></td></tr>\n"); \
        ptr += buf; \
    } while (0)
#define ADD_CONFIG_RADIO2(name, value, fmt, desc, text0, text1) \
    do \
    { \
        ptr += "<tr><td>" desc ":</td><td><div class=\"toggle-buttons together\">"; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c0\" name=\"" name "\" value=\"0\" %s>\n", (value==0)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c0\">" text0 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "<input type=\"radio\" id=\"" name "_c1\" name=\"" name "\" value=\"1\" %s>\n", (value==1)?"checked":""); \
        ptr += buf; \
        sprintf(buf, "<label for=\"" name "_c1\">" text1 "</label>\n"); \
        ptr += buf; \
        sprintf(buf, "</div></td></tr>\n"); \
        ptr += buf; \
    } while (0)

    ADD_CONFIG("hostname", current_config.hostname, "%s", "Hostname");
    ADD_CONFIG("verbose", current_config.verbose, "%d", "Verbosity (1=log)");
    ADD_CONFIG_RADIO4("rf1_cfg", current_config.rf1_cfg, "%d", "RF1 path", "Open", "RFO1", "50 &#8486;", "RFO2");
    ADD_CONFIG_RADIO4("rf2_cfg", current_config.rf2_cfg, "%d", "RF2 path", "Open", "RFO1", "50 &#8486;", "RFO2");
    ADD_CONFIG_RADIO4("rf3_cfg", current_config.rf3_cfg, "%d", "RF3 path", "Open", "RFO1", "50 &#8486;", "RFO2");
    ADD_CONFIG_RADIO4("rf4_cfg", current_config.rf4_cfg, "%d", "RF4 path", "Open", "RFO1", "50 &#8486;", "RFO2");
    ADD_CONFIG_RADIO2("rfo1_amp", current_config.rfo1_amp, "%d", "RFO1 Amplifier", "Off", "On");
    ADD_CONFIG_RADIO2("rfo2_amp", current_config.rfo2_amp, "%d", "RFO2 Amplifier", "Off", "On");

    ptr += "<td></td><td><input type=\"submit\" value=\"Write\"></td></table></form>\n";

    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}
