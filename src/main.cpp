#include <Wire.h>
#include <Wifi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>

#include "PowerMode.h"
#include "SystemManager.h"
#include "BatteryManager.h"

// === Display OLED (Heltec V2) ===
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);

// === Sensor BMP280 ===
Adafruit_BMP280 bmp;

// === Dados do Wifi ===
const char* ssid = "Escola_Instructiva";
const char* password = "InstructivaInterno*";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temperatura = 0;
float pressao = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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

void callback(char *topic, byte *payload, unsigned int length)
{
  // Handle incoming MQTT messages if needed
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for(int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("Connected");
      client.publish("/ThinkIOT/Publish", "Welcome");
      client.subscribe("/ThinkIOT/Subscribe");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Inicializa pinos do OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW); delay(10);
  digitalWrite(OLED_RST, HIGH); delay(10);

  // Inicia comunicação I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // Inicia display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Falha ao iniciar o display OLED!");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  // Inicia BMP280
  if (!bmp.begin(0x76)) {  // ou 0x77 dependendo do seu sensor
    Serial.println("Sensor BMP280 nao encontrado!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Erro BMP280!");
    display.display();
    while (true);
  }

  Serial.println("Sensor BMP280 iniciado.");
}

void loop() {
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;

    float temperatura = bmp.readTemperature();       // °C
    float pressao = bmp.readPressure() / 100.0F;     // hPa

    String temp = String(temperatura, 2);
    client.publish("/ThinkIOT/temp", temp.c_str());
    String press = String(pressao, 2);
    client.publish("/ThinkIOT/press", press.c_str());

    Serial.printf("Temperatura: %.2f C | Pressao: %.2f hPa\n", temperatura, pressao);
  }
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.printf("Temp: %.2f C\n", temperatura);
  display.printf("Pres: %.2f hPa\n", pressao);
  display.display();
}
