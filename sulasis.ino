#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

const char *ssid = "SULASiS";
const char *password = "98709870";

ESP8266WebServer server(80);
RTC_DS3231 rtc;

const int vanaPin = D5;
const int lambaPin = D6;
const int buzzerPin = D3;

struct SulamaZamani {
  int startHour;
  int startMin;
  int endHour;
  int endMin;
  bool aktif;
};

SulamaZamani zamanlar[3];
bool otomatikAktif = false;

// EEPROM Adresleri ve Boyutu
const int EEPROM_ZAMANLAR_ADDR = 0;
const int EEPROM_OTOMATIK_ADDR = sizeof(zamanlar);
const int EEPROM_SIZE = sizeof(zamanlar) + sizeof(bool);

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sulama Sistemi Kontrolü</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f0f8ff;
        }
        .container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        h1 {
            color: #2c5aa0;
            text-align: center;
            margin-bottom: 30px;
        }
        h2 {
            color: #2c5aa0;
            border-bottom: 2px solid #2c5aa0;
            padding-bottom: 5px;
        }
        .button {
            background-color: #4CAF50;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
            margin: 5px;
            min-width: 120px;
        }
        .button:hover {
            background-color: #45a049;
        }
        .button.red {
            background-color: #f44336;
        }
        .button.red:hover {
            background-color: #da190b;
        }
        .button.blue {
            background-color: #2196F3;
        }
        .button.blue:hover {
            background-color: #0b7dda;
        }
        .control-group {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
            flex-wrap: wrap;
        }
        .time-setting {
            background-color: #f9f9f9;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 15px;
        }
        .time-input {
            display: inline-block;
            margin: 5px 10px;
        }
        .time-input label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
            color: #333;
        }
        input[type="number"] {
            width: 60px;
            padding: 8px;
            border: 2px solid #ddd;
            border-radius: 4px;
            text-align: center;
            font-size: 16px;
        }
        input[type="number"]:focus {
            border-color: #2c5aa0;
            outline: none;
        }
        .status {
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-weight: bold;
        }
        .status.success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .time-label {
            font-weight: bold;
            color: #2c5aa0;
            margin-right: 15px;
        }
        .status-panel {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }
        .status-item {
            background: rgba(255,255,255,0.2);
            padding: 15px;
            border-radius: 10px;
            text-align: center;
            backdrop-filter: blur(10px);
        }
        .status-label {
            font-size: 14px;
            opacity: 0.9;
            margin-bottom: 5px;
        }
        .status-value {
            font-size: 18px;
            font-weight: bold;
        }
        .status-active {
            background: rgba(76, 175, 80, 0.3);
            border: 2px solid rgba(76, 175, 80, 0.5);
        }
        .status-inactive {
            background: rgba(244, 67, 54, 0.3);
            border: 2px solid rgba(244, 67, 54, 0.5);
        }
        .current-time {
            text-align: center;
            font-size: 24px;
            font-weight: bold;
            margin-bottom: 10px;
        }
        .switch { position: relative; display: inline-block; width: 60px; height: 34px; margin-left: 20px; vertical-align: middle;}
        .switch input {opacity: 0; width: 0; height: 0;}
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
        .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: #4CAF50; }
        input:checked + .slider:before { transform: translateX(26px); }
        @media (max-width: 600px) {
            body {
                padding: 10px;
            }
            h1 {
                font-size: 24px;
            }
            h2 {
                font-size: 20px;
            }
            .container {
                padding: 15px;
            }
            .control-group {
                flex-direction: column;
            }
            .button {
                width: 100%;
                box-sizing: border-box;
            }
            .time-setting {
                padding: 10px;
            }
            .time-label {
                display: block;
                margin-bottom: 10px;
            }
            .time-input {
                display: flex;
                justify-content: space-between;
                align-items: center;
                margin: 5px 0;
            }
            .time-input label {
                flex-basis: 50%;
            }
            input[type="number"] {
                width: 80px;
            }
            .switch {
                margin-left: 0;
                margin-bottom: 10px;
            }
        }
    </style>
</head>
<body>
    <h1>🌱 Sulama Sistemi Kontrolü</h1>
    
    <!-- Durum Paneli -->
    <div class="status-panel">
        <div class="current-time" id="currentTime">--:--</div>
        <div class="status-grid">
            <div class="status-item" id="vanaStatus">
                <div class="status-label">💧 Sulama</div>
                <div class="status-value" id="vanaValue">KAPALI</div>
            </div>
            <div class="status-item" id="lambaStatus">
                <div class="status-label">💡 Lamba</div>
                <div class="status-value" id="lambaValue">KAPALI</div>
            </div>
            <div class="status-item" id="otomatikStatus">
                <div class="status-label">🔄 Otomatik</div>
                <div class="status-value" id="otomatikValue">KAPALI</div>
            </div>
        </div>
    </div>
    
    <div class="container">
        <h2>Manuel Kontrol</h2>
        <div class="control-group">
            <button class="button" onclick="kontrolEt('/vanayon')">💧 Vana Aç</button>
            <button class="button red" onclick="kontrolEt('/vanakapat')">❌ Vana Kapat</button>
        </div>
        <div class="control-group">
            <button class="button blue" onclick="kontrolEt('/lambaon')">💡 Lamba Aç</button>
            <button class="button red" onclick="kontrolEt('/lambakapat')">🔌 Lamba Kapat</button>
        </div>
    </div>

    <div class="container">
        <h2>Otomatik Mod</h2>
        <div class="control-group">
            <button class="button" onclick="kontrolEt('/otomatikac')">🔄 Otomatik Aç</button>
            <button class="button red" onclick="kontrolEt('/otomatikkapat')">⏹️ Otomatik Kapat</button>
        </div>
    </div>

    <div class="container">
        <h2>Zaman Ayarları</h2>
        <form onsubmit="zamanKaydet(); return false;">
            <div class="time-setting">
                <span class="time-label">1. Sulama Zamanı:</span>
                <label class="switch"><input type="checkbox" id="aktif1"><span class="slider"></span></label>
                <div class="time-input">
                    <label>Başlangıç Saat</label>
                    <input type="number" style="width:50px;" id="s1h" min="0" max="23" value="6">
                </div>
                <div class="time-input">
                    <label>Başlangıç Dakika</label>
                    <input type="number" style="width:50px;" id="s1m" min="0" max="59" value="0">
                </div>
                <div class="time-input">
                    <label>Bitiş Saat</label>
                    <input type="number" style="width:50px;" id="e1h" min="0" max="23" value="6">
                </div>
                <div class="time-input">
                    <label>Bitiş Dakika</label>
                    <input type="number" style="width:50px;" id="e1m" min="0" max="59" value="5">
                </div>
            </div>

            <div class="time-setting">
                <span class="time-label">2. Sulama Zamanı:</span>
                <label class="switch"><input type="checkbox" id="aktif2"><span class="slider"></span></label>
                <div class="time-input">
                    <label>Başlangıç Saat</label>
                    <input type="number" style="width:50px;" id="s2h" min="0" max="23" value="12">
                </div>
                <div class="time-input">
                    <label>Başlangıç Dakika</label>
                    <input type="number" style="width:50px;" id="s2m" min="0" max="59" value="0">
                </div>
                <div class="time-input">
                    <label>Bitiş Saat</label>
                    <input type="number" style="width:50px;" id="e2h" min="0" max="23" value="12">
                </div>
                <div class="time-input">
                    <label>Bitiş Dakika</label>
                    <input type="number" style="width:50px;" id="e2m" min="0" max="59" value="5">
                </div>
            </div>

            <div class="time-setting">
                <span class="time-label">3. Sulama Zamanı:</span>
                <label class="switch"><input type="checkbox" id="aktif3"><span class="slider"></span></label>
                <div class="time-input">
                    <label>Başlangıç Saat</label>
                    <input type="number" style="width:50px;" id="s3h" min="0" max="23" value="18">
                </div>
                <div class="time-input">
                    <label>Başlangıç Dakika</label>
                    <input type="number" style="width:50px;" id="s3m" min="0" max="59" value="0">
                </div>
                <div class="time-input">
                    <label>Bitiş Saat</label>
                    <input type="number" style="width:50px;" id="e3h" min="0" max="23" value="18">
                </div>
                <div class="time-input">
                    <label>Bitiş Dakika</label>
                    <input type="number" style="width:50px;" id="e3m" min="0" max="59" value="5">
                </div>
            </div>

            <button type="submit" class="button">💾 Zamanları Kaydet</button>
        </form>
    </div>

    <div class="container">
        <h2>Saat ve Tarih Ayarı</h2>
        <div class="control-group">
            <button onclick="syncTimeWithBrowser()" class="button blue" style="width:100%;">📅 Tarayıcı Saati ile Eşitle</button>
        </div>
    </div>

    <div id="durum" class="status" style="display: none;"></div>

    <script>
        function kontrolEt(url) {
            fetch(url)
                .then(response => response.text())
                .then(data => {
                    durumGoster(data, 'success');
                })
                .catch(error => {
                    durumGoster('Hata: Bağlantı sorunu', 'error');
                });
        }

        function zamanKaydet() {
            // GET metoduyla parameter string oluştur
            const params = [
                's1h=' + document.getElementById('s1h').value,
                's1m=' + document.getElementById('s1m').value,
                'e1h=' + document.getElementById('e1h').value,
                'e1m=' + document.getElementById('e1m').value,
                'aktif1=' + (document.getElementById('aktif1').checked ? 1 : 0),

                's2h=' + document.getElementById('s2h').value,
                's2m=' + document.getElementById('s2m').value,
                'e2h=' + document.getElementById('e2h').value,
                'e2m=' + document.getElementById('e2m').value,
                'aktif2=' + (document.getElementById('aktif2').checked ? 1 : 0),

                's3h=' + document.getElementById('s3h').value,
                's3m=' + document.getElementById('s3m').value,
                'e3h=' + document.getElementById('e3h').value,
                'e3m=' + document.getElementById('e3m').value,
                'aktif3=' + (document.getElementById('aktif3').checked ? 1 : 0)
            ].join('&');

            const url = '/settime?' + params;
            
            fetch(url)
            .then(response => response.text())
            .then(data => {
                durumGoster(data, 'success');
                console.log('Zaman kaydı başarılı:', data);
                // Zamanları kaydettikten sonra sayfadaki değerleri güncelle
                setTimeout(loadTimes, 500);
            })
            .catch(error => {
                console.error('Zaman kaydetme hatası:', error);
                durumGoster('Hata: Zaman kaydedilemedi', 'error');
            });
        }

        function durumGoster(mesaj, tip) {
            const durumDiv = document.getElementById('durum');
            durumDiv.textContent = mesaj;
            durumDiv.className = 'status ' + tip;
            durumDiv.style.display = 'block';
            
            setTimeout(() => {
                durumDiv.style.display = 'none';
            }, 3000);
        }

        // Sayfa yüklendiğinde mevcut zamanları al
        window.onload = function() {
            loadTimes();
            loadStatus();
            // Her 2 saniyede bir durumu güncelle
            setInterval(loadStatus, 2000);
        };

        function loadStatus() {
            fetch('/getstatus')
                .then(response => response.json())
                .then(data => {
                    // Saat güncelle
                    document.getElementById('currentTime').textContent = 
                        data.saat + ' - ' + data.tarih;
                    
                    // Vana durumu
                    const vanaStatus = document.getElementById('vanaStatus');
                    const vanaValue = document.getElementById('vanaValue');
                    if(data.vana) {
                        vanaStatus.className = 'status-item status-active';
                        vanaValue.textContent = 'AÇIK';
                    } else {
                        vanaStatus.className = 'status-item status-inactive';
                        vanaValue.textContent = 'KAPALI';
                    }
                    
                    // Lamba durumu
                    const lambaStatus = document.getElementById('lambaStatus');
                    const lambaValue = document.getElementById('lambaValue');
                    if(data.lamba) {
                        lambaStatus.className = 'status-item status-active';
                        lambaValue.textContent = 'AÇIK';
                    } else {
                        lambaStatus.className = 'status-item status-inactive';
                        lambaValue.textContent = 'KAPALI';
                    }
                    
                    // Otomatik mod durumu
                    const otomatikStatus = document.getElementById('otomatikStatus');
                    const otomatikValue = document.getElementById('otomatikValue');
                    if(data.otomatik) {
                        otomatikStatus.className = 'status-item status-active';
                        otomatikValue.textContent = 'AÇIK';
                    } else {
                        otomatikStatus.className = 'status-item status-inactive';
                        otomatikValue.textContent = 'KAPALI';
                    }
                })
                .catch(error => {
                    console.error('Durum yüklenirken hata:', error);
                });
        }

        function loadTimes() {
            fetch('/gettimes')
                .then(response => response.json())
                .then(data => {
                    // Input değerlerini güncelle
                    document.getElementById('s1h').value = data.s1h || 6;
                    document.getElementById('s1m').value = data.s1m || 0;
                    document.getElementById('e1h').value = data.e1h || 6;
                    document.getElementById('e1m').value = data.e1m || 5;
                    document.getElementById('aktif1').checked = data.aktif1;
                    
                    document.getElementById('s2h').value = data.s2h || 12;
                    document.getElementById('s2m').value = data.s2m || 0;
                    document.getElementById('e2h').value = data.e2h || 12;
                    document.getElementById('e2m').value = data.e2m || 5;
                    document.getElementById('aktif2').checked = data.aktif2;
                    
                    document.getElementById('s3h').value = data.s3h || 18;
                    document.getElementById('s3m').value = data.s3m || 0;
                    document.getElementById('e3h').value = data.e3h || 18;
                    document.getElementById('e3m').value = data.e3m || 5;
                    document.getElementById('aktif3').checked = data.aktif3;
                    
                    console.log('Kayıtlı zamanlar yüklendi:', data);
                })
                .catch(error => {
                    console.error('Zamanlar yüklenirken hata:', error);
                    durumGoster('Kayıtlı zamanlar yüklenemedi', 'error');
                });
        }

        function syncTimeWithBrowser() {
            const now = new Date();
            const params = [
                'year=' + now.getFullYear(),
                'month=' + (now.getMonth() + 1), // JS aylar 0-11 arasıdır
                'day=' + now.getDate(),
                'hour=' + now.getHours(),
                'minute=' + now.getMinutes()
            ].join('&');

            const url = '/setdatetime?' + params;
            fetch(url)
                .then(response => response.text())
                .then(data => {
                    durumGoster(data, 'success');
                    // Durum panelini hemen güncelle
                    setTimeout(loadStatus, 500);
                })
                .catch(error => {
                    durumGoster('Hata: Saat eşitlenemedi', 'error');
                });
        }
    </script>
</body>
</html>
)rawliteral";

void zamanlariYukleEEPROM() {
  EEPROM.get(EEPROM_ZAMANLAR_ADDR, zamanlar);
  
  // Geçersiz veri kontrolü
  for (int i = 0; i < 3; i++) {
    if (zamanlar[i].startHour < 0 || zamanlar[i].startHour > 23 ||
        zamanlar[i].startMin < 0 || zamanlar[i].startMin > 59 ||
        zamanlar[i].endHour < 0 || zamanlar[i].endHour > 23 ||
        zamanlar[i].endMin < 0 || zamanlar[i].endMin > 59 ||
        (zamanlar[i].aktif != true && zamanlar[i].aktif != false)) {
      // Öntanımlı zamanlar: 06:00-06:05, 12:00-12:05, 18:00-18:05, hepsi aktif
      zamanlar[i] = {6 + i * 6, 0, 6 + i * 6, 5, true};
    }
  }
}

void zamanlariKaydetEEPROM() {
  EEPROM.put(EEPROM_ZAMANLAR_ADDR, zamanlar);
  EEPROM.commit();
  Serial.println("Zamanlar EEPROM'a kaydedildi");
}

void setup() {
  // Pin modlarını ayarla
  pinMode(vanaPin, OUTPUT);
  pinMode(lambaPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Başlangıçta tüm çıkışları kapat
  digitalWrite(vanaPin, LOW);
  digitalWrite(lambaPin, LOW);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Sulama Sistemi Başlatılıyor...");
  
  // WiFi AP modunu başlat
  WiFi.softAP(ssid, password);
  Serial.print("AP Modu Aktif: ");
  Serial.println(ssid);
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.softAPIP());
  
  // RTC'yi başlat
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC bulunamadı!");
    // RTC yoksa buzzer ile uyar
    for(int i = 0; i < 3; i++) {
      tone(buzzerPin, 2000, 200);
      delay(300);
    }
  } else {
    Serial.println("RTC başarıyla başlatıldı");
    DateTime now = rtc.now();
    Serial.printf("Mevcut zaman: %02d:%02d:%02d %02d/%02d/%04d\n", 
                  now.hour(), now.minute(), now.second(),
                  now.day(), now.month(), now.year());
  }
  
  // EEPROM'u başlat ve durumları yükle
  EEPROM.begin(EEPROM_SIZE);
  zamanlariYukleEEPROM();
  
  // Kayıtlı otomatik mod durumunu yükle
  otomatikAktif = EEPROM.read(EEPROM_OTOMATIK_ADDR);
  Serial.printf("Otomatik Mod durumu yüklendi: %s\n", otomatikAktif ? "Aktif" : "Pasif");
  
  // Yüklenen zamanları seri monitöre yazdır
  Serial.println("Kayıtlı sulama zamanları:");
  for(int i = 0; i < 3; i++) {
    Serial.printf("Zaman %d: %02d:%02d - %02d:%02d, Aktif: %s\n", 
                  i+1, zamanlar[i].startHour, zamanlar[i].startMin,
                  zamanlar[i].endHour, zamanlar[i].endMin,
                  zamanlar[i].aktif ? "Evet" : "Hayır");
  }
  
  // Web sunucu rotalarını tanımla
  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });
  
  server.on("/vanayon", []() {
    digitalWrite(vanaPin, HIGH);
    tone(buzzerPin, 1000, 500);  // 1 saniye buzzer sesi
    Serial.println("Vana açıldı (Manuel)");
    server.send(200, "text/plain", "Vana Açıldı");
  });
  
  server.on("/vanakapat", []() {
    digitalWrite(vanaPin, LOW);
    Serial.println("Vana kapatıldı (Manuel)");
    server.send(200, "text/plain", "Vana Kapatıldı");
  });
  
  server.on("/lambaon", []() {
    digitalWrite(lambaPin, HIGH);
    Serial.println("Lamba açıldı");
    server.send(200, "text/plain", "Lamba Açıldı");
  });
  
  server.on("/lambakapat", []() {
    digitalWrite(lambaPin, LOW);
    Serial.println("Lamba kapatıldı");
    server.send(200, "text/plain", "Lamba Kapatıldı");
  });
  
  server.on("/otomatikac", []() {
    otomatikAktif = true;
    EEPROM.write(EEPROM_OTOMATIK_ADDR, true);
    EEPROM.commit();
    Serial.println("Otomatik mod açıldı ve durum kaydedildi.");
    server.send(200, "text/plain", "Otomatik Mod Açık");
  });
  
  server.on("/otomatikkapat", []() {
    otomatikAktif = false;
    EEPROM.write(EEPROM_OTOMATIK_ADDR, false);
    EEPROM.commit();
    digitalWrite(vanaPin, LOW);  // Otomatik kapatıldığında vana da kapansın
    Serial.println("Otomatik mod kapatıldı ve durum kaydedildi.");
    server.send(200, "text/plain", "Otomatik Mod Kapalı");
  });
  
  server.on("/settime", []() {
    // Zaman parametrelerini al ve doğrula
    for(int i = 0; i < 3; i++) {
      String prefix = "s" + String(i+1);
      int startH = server.arg(prefix + "h").toInt();
      int startM = server.arg(prefix + "m").toInt();
      prefix = "e" + String(i+1);
      int endH = server.arg(prefix + "h").toInt();
      int endM = server.arg(prefix + "m").toInt();
      bool aktif = server.arg("aktif" + String(i+1)).toInt() == 1;
      
      // Geçerlilik kontrolü
      if(startH >= 0 && startH <= 23 && startM >= 0 && startM <= 59 &&
         endH >= 0 && endH <= 23 && endM >= 0 && endM <= 59) {
        zamanlar[i] = {startH, startM, endH, endM, aktif};
      }
    }
    
    zamanlariKaydetEEPROM();
    
    // Yeni zamanları seri monitöre yazdır
    Serial.println("Yeni sulama zamanları kaydedildi:");
    for(int i = 0; i < 3; i++) {
      Serial.printf("Zaman %d: %02d:%02d - %02d:%02d, Aktif: %s\n", 
                    i+1, zamanlar[i].startHour, zamanlar[i].startMin,
                    zamanlar[i].endHour, zamanlar[i].endMin,
                    zamanlar[i].aktif ? "Evet" : "Hayır");
    }
    
    server.send(200, "text/plain", "Zamanlar Başarıyla Kaydedildi");
  });
  
  // Mevcut zamanları JSON formatında döndür
  server.on("/gettimes", []() {
    String json = "{";
    for(int i = 0; i < 3; i++) {
      if(i > 0) json += ",";
      json += "\"s" + String(i+1) + "h\":" + String(zamanlar[i].startHour) + ",";
      json += "\"s" + String(i+1) + "m\":" + String(zamanlar[i].startMin) + ",";
      json += "\"e" + String(i+1) + "h\":" + String(zamanlar[i].endHour) + ",";
      json += "\"e" + String(i+1) + "m\":" + String(zamanlar[i].endMin) + ",";
      json += "\"aktif" + String(i+1) + "\":" + String(zamanlar[i].aktif ? "true" : "false");
    }
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/setdatetime", []() {
    if (server.hasArg("year") && server.hasArg("month") && server.hasArg("day") &&
        server.hasArg("hour") && server.hasArg("minute")) {
      
      int year = server.arg("year").toInt();
      int month = server.arg("month").toInt();
      int day = server.arg("day").toInt();
      int hour = server.arg("hour").toInt();
      int minute = server.arg("minute").toInt();

      // Basit doğrulama
      if (year >= 2000 && year <= 2099 && month >= 1 && month <= 12 && day >= 1 && day <= 31 &&
          hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
        
        rtc.adjust(DateTime(year, month, day, hour, minute, 0));
        Serial.println("RTC zamanı web arayüzünden ayarlandı.");
        server.send(200, "text/plain", "Saat ve Tarih Başarıyla Ayarlandı");
      } else {
        server.send(400, "text/plain", "Geçersiz tarih/saat değerleri");
      }
    } else {
      server.send(400, "text/plain", "Eksik parametreler");
    }
  });
  
  // Sistem durumunu JSON formatında döndür
  server.on("/getstatus", []() {
    DateTime now = rtc.now();
    char saat_str[9]; // HH:MM:SS
    sprintf(saat_str, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    
    char tarih_str[11]; // DD/MM/YYYY
    sprintf(tarih_str, "%02d/%02d/%04d", now.day(), now.month(), now.year());

    String json = "{";
    json += "\"vana\":" + String(digitalRead(vanaPin) ? "true" : "false") + ",";
    json += "\"lamba\":" + String(digitalRead(lambaPin) ? "true" : "false") + ",";
    json += "\"otomatik\":" + String(otomatikAktif ? "true" : "false") + ",";
    json += "\"saat\":\"" + String(saat_str) + "\",";
    json += "\"tarih\":\"" + String(tarih_str) + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });
  
  // 404 hatası için
  server.onNotFound([]() {
    server.send(404, "text/plain", "Sayfa bulunamadı");
  });
  
  server.begin();
  Serial.println("Web sunucu başlatıldı");
  Serial.println("Sistem hazır!");
  
  // Başlangıç sinyali
  tone(buzzerPin, 800, 200);
  delay(250);
  tone(buzzerPin, 1200, 200);
}

void loop() {
  server.handleClient();
  
  static unsigned long lastCheck = 0;
  unsigned long currentTime = millis();
  
  // Her saniye kontrol et (performans için)
  if(currentTime - lastCheck >= 1000) {
    lastCheck = currentTime;
    
    if(otomatikAktif) {
      DateTime now = rtc.now();
      bool sulamaAktif = false;
      
      // Her bir sulama zamanını kontrol et
      for (int i = 0; i < 3; i++) {
        if (!zamanlar[i].aktif) continue; // Sadece aktif olanları kontrol et

        // Zaman aralığını kontrol et
        int currentMinutes = now.hour() * 60 + now.minute();
        int startMinutes = zamanlar[i].startHour * 60 + zamanlar[i].startMin;
        int endMinutes = zamanlar[i].endHour * 60 + zamanlar[i].endMin;
        
        // Aynı gün içinde zaman aralığı kontrolü
        if(startMinutes <= endMinutes) {
          if(currentMinutes >= startMinutes && currentMinutes <= endMinutes) {
            sulamaAktif = true;
            break;
          }
        } 
        // Gece yarısını geçen zaman aralığı (örn: 23:00-01:00)
        else {
          if(currentMinutes >= startMinutes || currentMinutes <= endMinutes) {
            sulamaAktif = true;
            break;
          }
        }
      }
      
      // Vana durumunu ayarla
      static bool oncekiDurum = false;
      if(sulamaAktif != oncekiDurum) {
        digitalWrite(vanaPin, sulamaAktif ? HIGH : LOW);
        if(sulamaAktif) {
          tone(buzzerPin, 1500, 300);  // Sulama başladığında ses ver
          Serial.println("Otomatik sulama başladı");
        } else {
          Serial.println("Otomatik sulama durdu");
        }
        oncekiDurum = sulamaAktif;
      }
    }
  }
}
