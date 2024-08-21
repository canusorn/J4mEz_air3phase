/*
  -PZEM004T-
  5V - 5V
  GND - GND
  D3 - TX
  D4 - RX

*/

#include <ESP8266WiFi.h>
#include <PZEM004Tv30.h>
#include <Wire.h>          // Include Wire if you're using I2C
#include <SFE_MicroOLED.h> // Include the SFE_MicroOLED library
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

SoftwareSerial pzemSWSerial(D3, D4);

#define PIN_RESET -1
#define DC_JUMPER 0

#define PHASE 3
MicroOLED oled(PIN_RESET, DC_JUMPER); // Example I2C declaration, uncomment if using I2C

unsigned long previousMillis = 0;
uint8_t phase_display;

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    //------Display LOGO at start------
    oled.begin();
    oled.clear(PAGE);
    oled.clear(ALL);
    oled.display();

    Serial.println("ESPID: " + String(ESP.getChipId()));
    Serial.println("Ready.");

    //  pzem.resetEnergy(); //reset energy
}

void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000)
    { // run every 1 second
        previousMillis = currentMillis;

        phase_display++;
        if (phase_display >= 3)
            phase_display = 0;

        displayValue(); // update OLED

    }
}

void displayValue()
{

    PZEM004Tv30 pzems[] = {PZEM004Tv30(pzemSWSerial, 0x01), PZEM004Tv30(pzemSWSerial, 0x02), PZEM004Tv30(pzemSWSerial, 0x03)}; // array of pzem 3 phase
    float voltage[3], current[3], power[3], energy[3], frequency[3], pf[3];

    for (int i = 0; i < PHASE; i++)
    {
        //------read data------
        voltage[i] = pzems[i].voltage();
        if (!isnan(voltage[i]))
        { // ถ้าอ่านค่าได้
            current[i] = pzems[i].current();
            power[i] = pzems[i].power();
            energy[i] = pzems[i].energy();
            frequency[i] = pzems[i].frequency();
            pf[i] = pzems[i].pf();
        }
        else
        { // ถ้าอ่านค่าไม่ได้ให้ใส่ค่า NAN(not a number)
            current[i] = NAN;
            power[i] = NAN;
            energy[i] = NAN;
            frequency[i] = NAN;
            pf[i] = NAN;
        }

        //------Serial display------
        Serial.print(F("PZEM "));
        Serial.print(i);
        Serial.print(F(" - Address:"));
        Serial.println(pzems[i].getAddress(), HEX);
        Serial.println(F("==================="));
        if (!isnan(voltage[i]))
        {
            Serial.print(F("Voltage: "));
            Serial.print(voltage[i]);
            Serial.println("V");
            Serial.print(F("Current: "));
            Serial.print(current[i]);
            Serial.println(F("A"));
            Serial.print(F("Power: "));
            Serial.print(power[i]);
            Serial.println(F("W"));
            Serial.print(F("Energy: "));
            Serial.print(energy[i], 3);
            Serial.println(F("kWh"));
            Serial.print(F("Frequency: "));
            Serial.print(frequency[i], 1);
            Serial.println(F("Hz"));
            Serial.print(F("PF: "));
            Serial.println(pf[i]);
        }
        else
        {
            Serial.println("No sensor detect");
        }
        Serial.println(F("-------------------"));
        Serial.println();
    }


    //------Update OLED display------
    oled.clear(PAGE);
    oled.setFontType(0);

    oled.setCursor(0, 0);
    oled.println("Phase " + String(phase_display + 1));

    // display voltage
    oled.setCursor(3, 12);
    oled.print(voltage[phase_display], 1);
    oled.setCursor(42, 12);
    oled.println("V");

    // display current
    if (current[phase_display] < 10)
        oled.setCursor(9, 21);
    else
        oled.setCursor(3, 21);
    oled.print(current[phase_display], 2);
    oled.setCursor(42, 21);
    oled.println("A");

    // display power
    if (power[phase_display] < 10)
        oled.setCursor(26, 30);
    else if (power[phase_display] < 100)
        oled.setCursor(20, 30);
    else if (power[phase_display] < 1000)
        oled.setCursor(14, 30);
    else if (power[phase_display] < 10000)
        oled.setCursor(8, 30);
    else
        oled.setCursor(2, 30);
    oled.print(power[phase_display], 0);
    oled.setCursor(42, 30);
    oled.println("W");

    // display energy
    oled.setCursor(3, 40);
    if (energy[phase_display] < 10)
        oled.print(energy[phase_display], 3);
    else if (energy[phase_display] < 100)
        oled.print(energy[phase_display], 2);
    else if (energy[phase_display] < 1000)
        oled.print(energy[phase_display], 1);
    else
    {
        oled.setCursor(8, 40);
        oled.print(energy[phase_display], 0);
    }
    oled.setCursor(42, 40);
    oled.println("kWh");

    // on error
    if (isnan(voltage[phase_display]))
    {
        oled.clear(PAGE);
        oled.setCursor(0, 0);
        oled.println("Phase " + String(phase_display + 1));
        oled.printf("\nno sensor\ndetect!");
    }

    oled.display();
}
