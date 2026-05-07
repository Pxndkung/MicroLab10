#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "WiFi.h"
#include "DHTesp.h"
#define pinDHT22 15
DHTesp dht;
#define CLK 22
#define DT 23
int counter = 0;
int currentStateCLK;
int lastStateCLK;
unsigned long lastDebounceTime = 0;
#ifdef __cplusplus
extern "C" {
  uint8_t temprature_sens_read();
}
#endif

#define WIFI_SSID "Pxnd"
#define WIFI_PASSWORD "18122548"

#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "zFFJR4jdWNyBQ_-wDa2Db2BdrXCfT8lZrMdXU6Hm3hKpEg-ponAJZ8wOs6G2ME8hja_da0BTeLV4St120_0YDA=="
#define INFLUXDB_ORG "CPE29"
#define INFLUXDB_BUCKET "Project-MicroWith-InfluxDB"

#define TZ_INFO "UTC7"

WiFiMulti wifiMulti;
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensorNode("Sensor_Data");

void setup() {
  Serial.begin(115200);

  dht.setup(pinDHT22, DHTesp::DHT22);

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  lastStateCLK = digitalRead(CLK);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");

  // ซิงค์เวลาจาก Server (จำเป็นสำหรับการส่งข้อมูลเข้า Cloud)
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // ตรวจสอบการเชื่อมต่อ InfluxDB
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}
float degree = 0;
void loop() {
  currentStateCLK = digitalRead(CLK);


  if (currentStateCLK != lastStateCLK) {
    if (digitalRead(DT) != currentStateCLK) {
      counter++;
    } else {
      counter--;
    }
    if (counter < 0) {
      int ch = counter * -1;
      counter = ch;
    }
    degree = (counter * 360) / 30;
    if (degree > 360) {
      int ch;
      ch = degree - (counter / 30) * 360;
      degree = ch;
    }
    Serial.print("Deg ree: ");
    Serial.println(degree);
  }
  lastStateCLK = currentStateCLK;
  float ESP32_Tempp = (temprature_sens_read() - 32) / 1.8;
  float DHT22_Temp = dht.getTemperature();
  float DHT22_Humid = dht.getHumidity();
  float Rotary_Angle = degree;

  if (millis() - lastDebounceTime >= 10000) {
    lastDebounceTime = millis();

    sensorNode.clearFields();
    sensorNode.addField("ESP32_MAC_Address", WiFi.macAddress());
    sensorNode.addField("ESP32_Wifi_Name", "Pxnd");
    sensorNode.addField("ESP32_Tempp", ESP32_Tempp);
    sensorNode.addField("DHT22_Temp", DHT22_Temp);
    sensorNode.addField("DHT22_Humid", DHT22_Humid);
    sensorNode.addField("Rotary_Angle", degree);

    // ส่งข้อมูลไปที่ InfluxDB
    Serial.print("Writing: ");
    Serial.println(sensorNode.toLineProtocol());

    if (!client.writePoint(sensorNode)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}
