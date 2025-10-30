# Temperatur- og Fugtighedsovervågning med Arduino

Dette projekt anvender en Arduino til at overvåge temperatur og fugtighed og viser dataene på en LCD-skærm med RGB-baggrundsbelysning. Enheden tilsluttes et WiFi-netværk og sender data til en server og opdaterer automatisk sine indstillinger via serveren.

## Hardware

- **Arduino-kompatibel enhed** med WiFi-kompatibilitet (WiFiNINA understøttet).
- **Grove Temperature and Humidity Sensor (DHT22)** til måling af temperatur og fugtighed.
- **RGB LCD** til visning af målinger og baggrundsfarve baseret på temperatur.
- **Blæserstyring** til regulering af ventilator afhængigt af temperaturen.

## Krævede Arduino-biblioteker

Installer følgende biblioteker gennem Arduino Library Manager:

- **SPI**
- **WiFiNINA**
- **rgb_lcd**
- **Grove_Temperature_And_Humidity_Sensor**
- **ArduinoJson**
- **NTPClient**

## Installation

1. **Klon projektet** eller download koden som ZIP, og tilføj den til dit Arduino IDE-projekt.
2. **Opdater WiFi-oplysninger** i en `secrets.h`-fil:
   ```cpp
   #define SECRET_SSID "din_SSID"
   #define SECRET_PASS "dit_password"
### Serverdetaljer
Indsæt `API_ADDRESS` og `PORT` i koden eller `secrets.h`, som angiver serverens IP og portnummer.

### Funktionalitet

- **WiFi-Forbindelse**: Enheden opretter forbindelse til WiFi ved opstart.
- **Datavisning**: Temperatur og fugtighed vises på LCD'en. Baggrundsfarven skifter afhængigt af temperaturen.
- **Blæserstyring**: En ventilator aktiveres, når temperaturen overstiger den indstillede grænse.
- **Dataoverførsel**: Sender temperatur- og fugtighedsdata til en ekstern server med jævne mellemrum.
- **Automatisk opdatering af indstillinger**: Henter indstillinger fra serveren hver 10. minut (kan justeres).

### Funktioner i Koden

- **getSettings()**: Henter temperaturgrænser og måleinterval fra serveren.
- **sendData(float temperature, float humidity)**: Sender sensorværdier til serveren som JSON.
- **calcBG(float temp, int &bgR, int &bgG, int &bgB)**: Justerer LCD-baggrundsfarven baseret på temperaturen.
