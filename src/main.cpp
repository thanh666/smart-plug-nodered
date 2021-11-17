#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include "ACS712.h"

#define LEDPIN 2

const char ssid[] = "Cong Thanh";
const char password[] = "19411940";
WiFiClient net;
PubSubClient client(net);
ACS712 sensor(ACS712_05B, 35);

#define MQTT_SERVER "13.208.211.184"
#define MQTT_PORT 1883
#define MQTT_USER "esp32"
#define MQTT_PASSWORD "Quamon"
#define MQTT_CURRENT_TOPIC "current"

unsigned long previousMillis = 0; 
const long interval = 5000;
unsigned long lastMillis = 0;
int current_ledState = LOW;
int last_ledState = LOW;

void setup_wifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void connect_to_broker()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "nodeWiFi32";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("connected");
            client.subscribe("data");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println("-------new message from broker-----");
    Serial.print("topic: ");
    Serial.println(topic);
    Serial.print("message: ");
    Serial.write(payload, length);
    Serial.println();
    if (*payload == '1')
        current_ledState = HIGH;
    if (*payload == '0')
        current_ledState = LOW;
}

void setup()
{
    Serial.begin(9600);
    Serial.setTimeout(500);
    setup_wifi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
    connect_to_broker();
    Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
    sensor.calibrate();
    Serial.println("Done!");
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, current_ledState);
}
void loop()
{
    client.loop();
    if (!client.connected())
    {
        connect_to_broker();
    }
    if (last_ledState != current_ledState)
    {
        last_ledState = current_ledState;
        digitalWrite(LEDPIN, current_ledState);
        Serial.print("LED state is ");
        Serial.println(current_ledState);
    }
    float U = 220;
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        float I = sensor.getCurrentAC();
        float P = U * I;
        if (isnan(I) || isnan(P))
        {
            Serial.println(F("Failed to read from current sensor!"));
            return;
        }
        else if (current_ledState==0)
        {
            client.publish(MQTT_CURRENT_TOPIC, String(0).c_str());
            while (current_ledState == 0)
            {
                client.loop();
            }
            
        }
        else
        {
            client.publish(MQTT_CURRENT_TOPIC, String(P).c_str());
            previousMillis = currentMillis;
        }
    }
    //digitalWrite(LEDPIN, 1);
}