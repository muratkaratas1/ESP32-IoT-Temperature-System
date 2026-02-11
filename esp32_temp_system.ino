#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ===================
// WiFi Ayarları
// ===================
const char* ssid = "YOUR_WIFI_NAME";        
const char* password = "YOUR_WIFI_PASSWORD"; 

// ===================
// ThingSpeak Ayarları
// ===================
String apiKey = "YOUR_THINGSPEAK_WRITE_API_KEY";   
const char* server = "http://api.thingspeak.com/update";
unsigned long channelID = 0000000;  // Kendi Channel ID'nizi giriniz

// ===================
// DHT11
// ===================
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================
// LED Pinleri
// ===================
#define RED_PIN    2
#define GREEN_PIN  4
#define BLUE_PIN   5

// ===================
// Alarm Ayarları
// ===================
unsigned long lastAlarmTime = 0;
const unsigned long alarmInterval = 60000; // 1 dakika

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // WiFi Bağlantısı
  WiFi.begin(ssid, password);
  Serial.print("WiFi'ye bağlanıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi bağlandı!");
  Serial.print("IP adresi: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT okuma hatası!");
    delay(2000);
    return;
  }

  Serial.print("Sıcaklık: "); 
  Serial.print(temp); 
  Serial.println(" °C");

  Serial.print("Nem: "); 
  Serial.print(hum); 
  Serial.println(" %");

  // ===================
  // LED Kontrol
  // ===================

  if (temp <= 27) {

    analogWrite(BLUE_PIN, 255);
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);

  } 
  else if (temp <= 29) {

    int blueVal = map(temp, 28, 29, 200, 50);
    int redVal  = map(temp, 28, 29, 30, 100);

    analogWrite(BLUE_PIN, blueVal);
    analogWrite(RED_PIN, redVal);
    analogWrite(GREEN_PIN, 0);

  } 
  else if (temp >= 30) {

    unsigned long now = millis();

    if (now - lastAlarmTime > alarmInterval) {
      lastAlarmTime = now;

      for (int i = 0; i < 3; i++) {
        analogWrite(RED_PIN, 255);
        delay(500);
        analogWrite(RED_PIN, 0);
        delay(500);
      }
    }

    analogWrite(RED_PIN, 255);
    analogWrite(BLUE_PIN, 0);
    analogWrite(GREEN_PIN, 0);
  }

  // ===================
  // ThingSpeak Veri Gönderimi
  // ===================

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = String(server) +
                 "?api_key=" + apiKey +
                 "&field1=" + String(temp) +
                 "&field2=" + String(hum);

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("Veri ThingSpeak'e gönderildi.");
    } else {
      Serial.println("Veri gönderilemedi!");
    }

    http.end();
  }

  delay(20000);
}
