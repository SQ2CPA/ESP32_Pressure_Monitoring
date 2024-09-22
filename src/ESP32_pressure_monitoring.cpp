#include "FS.h"
#include <Wifi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#define WIFI_DEFAULT_SSID "BalloonMeterAP"

#define OUT_PIN 3
#define SCK_PIN 2

#define BUZZER_PIN 9
#define PUMP_PIN 10

extern const char web_index_html[] asm("_binary_data_embed_index_html_gz_start");
extern const char web_index_html_end[] asm("_binary_data_embed_index_html_gz_end");
extern const size_t web_index_html_len = web_index_html_end - web_index_html;

extern const char web_script_js[] asm("_binary_data_embed_script_js_gz_start");
extern const char web_script_js_end[] asm("_binary_data_embed_script_js_gz_end");
extern const size_t web_script_js_len = web_script_js_end - web_script_js;

long zeroOffset = 0;

long readSingle()
{
    long result = 0;

    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(1);

    while (digitalRead(OUT_PIN) == HIGH)
        ;

    for (int i = 0; i < 24; i++)
    {
        digitalWrite(SCK_PIN, HIGH);
        result = result << 1;

        if (digitalRead(OUT_PIN) == HIGH)
        {
            result++;
        }
        digitalWrite(SCK_PIN, LOW);
    }

    digitalWrite(SCK_PIN, HIGH);
    result = result ^ 0x800000;
    digitalWrite(SCK_PIN, LOW);

    return result;
}

long readMultiple(int readings = 5)
{
    long sum = 0;

    for (int i = 0; i < readings; i++)
    {
        sum += readSingle();
        delay(100);
    }

    return sum / readings;
}

void calibrate()
{
    zeroOffset = readMultiple(15);

    Serial.print("Calibration done. Offset: ");
    Serial.println(zeroOffset);
}

AsyncWebServer server(80);
Preferences preferences;

void handleHome(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", (const uint8_t *)web_index_html, web_index_html_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void handleScript(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", (const uint8_t *)web_script_js, web_script_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void handleNotFound(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", "Not found");
    response->addHeader("Cache-Control", "max-age=3600");
    request->send(response);
}

long value;

void handleData(AsyncWebServerRequest *request)
{
    String type = request->getParam("type", false)->value();

    if (type == "offset")
        request->send(200, "text/plain", String(zeroOffset));
    else if (type == "value")
    {
        request->send(200, "text/plain", String(value));
    }
    else
        request->send(400, "text/plain", "Bad request");
}

void handleOutputPump(AsyncWebServerRequest *request)
{
    String value = request->getParam("value", false)->value();

    digitalWrite(PUMP_PIN, value == "1" ? HIGH : LOW);

    request->send(200, "text/plain", "OK");
}

void handleOutputBuzzer(AsyncWebServerRequest *request)
{
    String value = request->getParam("value", false)->value();

    digitalWrite(BUZZER_PIN, value == "1" ? HIGH : LOW);

    request->send(200, "text/plain", "OK");
}

void handleWriteConfigurationAP(AsyncWebServerRequest *request)
{
    String ssid = request->getParam("ssid", true)->value();
    String password = request->getParam("password", true)->value();

    preferences.putString("AP.ssid", ssid);
    preferences.putString("AP.password", password);

    preferences.end();

    request->send(200, "text/plain", "ESP32 will reboot now");

    ESP.restart();
}

void setup()
{
    pinMode(SCK_PIN, OUTPUT);
    pinMode(OUT_PIN, INPUT);

    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(PUMP_PIN, HIGH);

    delay(5000);

    calibrate();

    digitalWrite(PUMP_PIN, LOW);

    preferences.begin("my-app", false);

    WiFi.mode(WIFI_MODE_NULL);
    WiFi.hostname("Balloon-Meter");

    String ssid = preferences.getString("AP.ssid", "");
    String password = preferences.getString("AP.password", "");

    bool wifiAP = true;

    if (ssid != "")
    {
        Serial.print("Connecting to: ");
        Serial.println(ssid);

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        if (password != "")
        {
            WiFi.begin(ssid, password);
        }
        else
        {
            WiFi.begin(ssid);
        }

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
        {
            delay(500);
            Serial.print(".");
        }

        Serial.println("");

        if (WiFi.status() == WL_CONNECTED)
        {
            wifiAP = false;
        }
        else
        {
            Serial.println("WiFi not connected");

            digitalWrite(BUZZER_PIN, HIGH);
            delay(200);
            digitalWrite(BUZZER_PIN, LOW);
        }
    }

    if (wifiAP)
    {
        Serial.println("Starting WiFi AP");

        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_DEFAULT_SSID, NULL);
    }

    Serial.print("WiFi connected at: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleHome);
    server.on("/script.js", HTTP_GET, handleScript);
    server.on("/data", HTTP_GET, handleData);
    server.on("/output/pump", HTTP_GET, handleOutputPump);
    server.on("/output/buzzer", HTTP_GET, handleOutputBuzzer);
    server.on("/config/AP", HTTP_POST, handleWriteConfigurationAP);

    server.onNotFound(handleNotFound);

    server.begin();
}

void loop()
{
    value = readMultiple();

    delay(250);
}
