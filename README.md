# Sulasis - Akıllı Sulama Sistemi - Smart Irrigation System

ESP8266 tabanlı, zamanlayıcı kontrollü ve Wi-Fi ap üzerinden uzaktan erişilebilen bir **bahçe sulama otomasyonu** projesidir.

# Özellikler

-  Zamanlı otomatik sulama (DS3231 RTC modülü ile)
-  Manuel vana ve lamba kontrolü (2'li röle modülü üzerinden)
-  Wi-Fi bağlantısı (ESP8266 ile arayüz kontrolü)
-  Buzzer ile sesli uyarı
-  Web arayüzü ile uzaktan kontrol

# Kullanılan Donanımlar

| Parça              | Açıklama                          |
|-------------------|-----------------------------------|
| ESP8266 (NodeMCU) | Ana denetleyici, Wi-Fi bağlantısı |
| 2’li Röle Modülü  | Vana ve lamba kontrolü            |
| DS3231            | Gerçek zaman saati (RTC)          |
| 12V Selenoid Vana | Su akış kontrolü                  |
| LM2596            | Voltaj düşürücü modül             |
| Buzzer            | Sesli uyarı                       |

# Pin Bağlantıları

| Modül/Bileşen     | ESP8266 Bağlantı (NodeMCU Pinleri) |
|-------------------|-------------------------------------|
| Röle 1 (Lamba)     | D6                          |
| Röle 2 (Vana)      | D5                          |
| DS3231 SDA         | D2                          |
| DS3231 SCL         | D1                          |
| Buzzer             | D3                          |
| GND                | GND                                 |
| VCC (Modüller)     | VIN veya 3V (gereksinime göre)      |

> Not: Röle modülü 5V ile çalışıyorsa harici bir 5V güç kaynağı ve optoizolasyon önerilir.
> Geri bildirim ve katkılarınızı bekliyoruz!!!



![1](https://github.com/user-attachments/assets/13317b6b-8d34-48ba-90d2-5a532c83c6d1)



