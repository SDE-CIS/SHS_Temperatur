# SHS_Temperatur

SHS_Temperatur er en IoT-enhed udviklet til Safehouse Solutions-systemet, som overvåger temperatur og luftfugtighed i realtid.  
Systemet benytter en **DHT11-sensor**, **RGB LCD-skærm** og en **DC-ventilator** til både visning og automatisk klimastyring.  
Data sendes og modtages via **MQTT**-protokollen, hvilket muliggør integration med Safehouse Solutions-dashboardet.

---

## Tekniske specifikationer
- **Platform:** Arduino Uno WiFi Rev2  
- **Sensor:** Grove DHT11 (temperatur og luftfugtighed)  
- **Display:** Grove RGB LCD (16x2)  
- **Netværk:** WiFi (WiFiNINA)  
- **Kommunikation:** MQTT via PubSubClient  
- **Ventilatorstyring:** PWM-baseret output (digital pin 4)  
- **Protokoller:** TLS, JSON-format via ArduinoJson  
- **Tidsstyring:** NTPClient til realtids-synkronisering  

---

## Funktionalitet
Enheden fungerer både som **måleenhed** og **automationsmodul** for rumtemperatur:

1. **Måling og visning**
   - Måler temperatur og fugtighed kontinuerligt.
   - Viser værdier på LCD-display med dynamisk baggrundsfarve:
     - **Blå:** Temperatur under grænseværdi (*tooLow*).  
     - **Grøn:** Normal temperatur (*justRight*).  
     - **Rød:** For høj temperatur (*tooHigh*).  
   - Justering af farve sker automatisk ved ændring af temperatur.

2. **MQTT-integration**
   - Sender sensorværdier og status til Safehouse-platformen.  
   - Lytter på MQTT-emner for ændringer i indstillinger.  
   - Understøtter to emner:
     ```
     1/temperatur/stue/1/settings
     1/fan/stue/1/settings
     ```
     Disse emner indeholder konfigurationsdata, som definerer grænseværdier og ventilatorindstillinger.

3. **Ventilatorstyring**
   - Ventilatoren tændes eller slukkes automatisk afhængigt af temperaturen.  
   - Kan også fjernstyres manuelt via dashboardet gennem MQTT.  
   - Implementerer *debounce*-logik, så ventilatoren ikke tænder/slukker gentagne gange inden for 10 sekunder.  

4. **Kalibrering**
   - Temperaturmålingen kan justeres med en kalibreringsværdi (`tempCalibration = -2`), som finjusterer sensoren lokalt.

---

## MQTT-kommunikation
Enheden sender og modtager data i JSON-format:
```json
{
  "temperature": 21.5,
  "humidity": 47,
  "fan": "on",
  "mode": "auto",
  "timestamp": "2025-11-12T14:23:00Z"
}
```

```bash
{UserID}/temperature/livingroom/1
{UserID}/temperature/livingroom/1/settings
{UserID}/fan/livingroom/1
{UserID}/fan/livingroom/1/settings
```

* temperature/... bruges til at rapportere sensorværdier og modtage grænser.
* fan/... styrer blæsertilstand og hastighed.

```
SHS_Temperatur/
├── src/
│   ├── main.cpp         # Hovedlogik (måling, MQTT, LCD og ventilator)
│   └── config.h         # Netværks- og MQTT-konfiguration
├── include/             # Headerfiler
├── lib/                 # Eventuelle tredjepartsbiblioteker
├── test/                # Testfiler
├── platformio.ini       # PlatformIO konfiguration
└── README.md            # Projektbeskrivelse
```

## Konfigurationsfil (config.h)

```cpp
#define WIFI_SSID "SafehouseNet"
#define WIFI_PASS "StrongPassword123"
#define MQTT_SERVER "mqtt.safehouse.local"
#define MQTT_PORT 1883
#define MQTT_USER "shs_user"
#define MQTT_PASS "shs_pass"
```

## Safehouse Solutions-platformen

Når systemet er aktivt, fungerer det i tæt integration med Safehouse Solutions’ API og dashboard:
1. Sensoren sender løbende temperaturmålinger via MQTT.
2. Dashboardet opdateres i realtid og kan vise grafer over historiske data.
3. Brugeren kan justere temperaturgrænser eller ventilatorindstillinger via dashboardet.
4. Ændringer sendes tilbage til enheden via MQTT og anvendes øjeblikkeligt.
5. Dette gør SHS_Temperatur til en aktiv komponent i hjemmets automatiske klimastyring, hvor både manuel og automatisk kontrol er mulig.

## Sikkerhed
* Wi-Fi-forbindelse krypteres via WPA2.
* MQTT-forbindelse kan udvides med TLS for end-to-end sikkerhed.
* JSON-validering beskytter mod ugyldige payloads.
* Platformen validerer indkommende beskeder ud fra bruger-ID og device-ID.

## Krav og opsætning

**Hardware**
* Arduino Uno WiFi Rev2
* Grove DHT11 sensor
* Grove RGB LCD
* DC ventilator

**Software**
* PlatformIO / Arduino IDE
* Biblioteker:
* WiFiNINA
* PubSubClient
* Grove_Temperature_And_Humidity_Sensor
* rgb_lcd
* ArduinoJson
* NTPClient

### Upload
```bash
pio run --target upload
```

## Fremtidige udvidelser
* Understøttelse af DHT22 for højere præcision
* MQTT over TLS (port 8883)
* Lokal datalogning på SD-kort
* Automatisk kalibrering baseret på historiske værdier
