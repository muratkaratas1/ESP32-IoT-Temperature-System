#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ===================
// WiFi Ayarları
// ===================
const char* ssid = "Yusuf adlı kişiye ait A32";        
const char* password = "hhaa1178"; 

// ===================
// ThingSpeak Ayarları
// ===================
String apiKey = "ZIRI0K7NPVO7K89T";   
const char* server = "http://api.thingspeak.com/update";
unsigned long channelID = 3048529; 

// ===================
// DHT11
// ===================
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===================
// LED Pinleri
// ===================
#define RED_PIN    2   // GPIO2
#define GREEN_PIN  4   // GPIO4
#define BLUE_PIN   5   // GPIO5

// ===================
// Değişkenler
// ===================
unsigned long lastAlarmTime = 0;  // son alarm zamanı
const unsigned long alarmInterval = 60000; // 1 dakika (ms)

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
  Serial.println("\n✅ WiFi bağlandı!");
  Serial.print("IP adresi: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println(" DHT okuma hatası!");
    delay(2000);
    return;
  }

  Serial.print("Sıcaklık: "); Serial.print(temp); Serial.println(" °C");
  Serial.print("Nem: "); Serial.print(hum); Serial.println(" %");

  // LED Kontrolü
  if (temp <= 27) {      
    analogWrite(BLUE_PIN, 255); // tam mavi
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);

  } else if (temp <= 29) { 
    // Mavi azalıyor, kırmızı düşük parlaklıkta yanıyor
    int blueVal = map(temp, 28, 29, 200, 50);  // 28'de parlak mavi, 29'da sönük
    int redVal  = map(temp, 28, 29, 30, 100);  // 28'de düşük kırmızı, 29'da biraz daha güçlü
    analogWrite(BLUE_PIN, blueVal);
    analogWrite(RED_PIN, redVal);
    analogWrite(GREEN_PIN, 0);

  } else if (temp >= 30) { 
    unsigned long now = millis();

    // Alarm sadece her 1 dakikada bir tetiklenir
    if (now - lastAlarmTime > alarmInterval) {
      lastAlarmTime = now;

      // 3 kere yanıp sönme
      for (int i = 0; i < 3; i++) {
        analogWrite(RED_PIN, 255);
        delay(500);
        analogWrite(RED_PIN, 0);
        delay(500);
      }
    }
    // Sonrasında kırmızı sabit yanar
    analogWrite(RED_PIN, 255);
    analogWrite(BLUE_PIN, 0);
    analogWrite(GREEN_PIN, 0);
  }

  // ThingSpeak'e veri gönderimi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temp) + "&field2=" + String(hum);
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
