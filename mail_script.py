import requests
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import time
from datetime import datetime

# =========================
# ThingSpeak Ayarları
# =========================
THINGSPEAK_CHANNEL_ID = "YOUR_CHANNEL_ID"
THINGSPEAK_API_KEY = "YOUR_READ_API_KEY"

THINGSPEAK_API_URL = f"https://api.thingspeak.com/channels/{THINGSPEAK_CHANNEL_ID}/fields/1/last.json"

# =========================
# Kritik Sıcaklık Eşiği
# =========================
KRITIK_SICAKLIK = 30.0

# =========================
# E-Posta Ayarları
# =========================
SMTP_SERVER = "smtp.gmail.com"
SMTP_PORT = 587

EMAIL = "your_email@gmail.com"
PASSWORD = "your_app_password"  # Gmail uygulama şifresi
TO_EMAIL = "receiver_email@gmail.com"

# =========================
# Zaman Kontrolleri
# =========================
SON_MAIL_ZAMANI = 0
MAIL_GECIKME_SURESI = 600      # 10 dakika

SON_BASARISIZ_VERI_ZAMANI = 0
VERI_UYARI_SURESI = 300        # 5 dakika


def send_email(subject, body):
    """Mail gönderme işlemi"""
    try:
        msg = MIMEMultipart()
        msg["From"] = EMAIL
        msg["To"] = TO_EMAIL
        msg["Subject"] = subject
        msg.attach(MIMEText(body, "plain"))

        server = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
        server.starttls()
        server.login(EMAIL, PASSWORD)
        server.send_message(msg)
        server.quit()

        print(f"Mail gönderildi: {subject}")

    except Exception as e:
        print("E-posta gönderim hatası:", e)


def get_temperature():
    """ThingSpeak'ten sıcaklık verisini çeker"""
    try:
        response = requests.get(
            THINGSPEAK_API_URL,
            params={"api_key": THINGSPEAK_API_KEY},
            timeout=10
        )

        data = response.json()
        temp = float(data["field1"])
        return temp

    except Exception as e:
        print("Veri alınamadı:", e)
        return None


def main():
    global SON_MAIL_ZAMANI, SON_BASARISIZ_VERI_ZAMANI

    print("Sistem başlatıldı. Sıcaklık ve bağlantı izleniyor...")

    while True:
        now = time.time()
        temp = get_temperature()

        if temp is not None:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Güncel sıcaklık: {temp} °C")

            # Kritik sıcaklık kontrolü
            if temp >= KRITIK_SICAKLIK and (now - SON_MAIL_ZAMANI) > MAIL_GECIKME_SURESI:
                send_email(
                    "Kritik Sıcaklık Uyarısı",
                    f"Sıcaklık kritik seviyeye ulaştı.\nAnlık sıcaklık: {temp} °C"
                )
                SON_MAIL_ZAMANI = now

            SON_BASARISIZ_VERI_ZAMANI = 0

        else:
            # Veri alınamadığında süre takibi
            if SON_BASARISIZ_VERI_ZAMANI == 0:
                SON_BASARISIZ_VERI_ZAMANI = now

            elif (now - SON_BASARISIZ_VERI_ZAMANI) > VERI_UYARI_SURESI:
                send_email(
                    "Veri Alınamıyor Uyarısı",
                    "Son 5 dakikadır ThingSpeak üzerinden veri alınamıyor.\n"
                    "Bağlantı veya cihaz kontrol edilmelidir."
                )
                SON_BASARISIZ_VERI_ZAMANI = now

        time.sleep(60)


if __name__ == "__main__":
    main()
