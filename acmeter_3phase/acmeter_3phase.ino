/*
  -PZEM004T-
  5V - 5V
  GND - GND
  D3 - TX
  D4 - RX
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PZEM004Tv30.h>
#include <Wire.h>          // Include Wire if you're using I2C
#include <SFE_MicroOLED.h> // Include the SFE_MicroOLED library
#include <SoftwareSerial.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = "G6PD";   // your network SSID (name)
char pass[] = "570610193";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = 2552953;
const char * myWriteAPIKey = "56RK2JZSRULTPWH9";

SoftwareSerial pzemSWSerial(D3, D4);

#define PIN_RESET -1
#define DC_JUMPER 0

#define PHASE 3
MicroOLED oled(PIN_RESET, DC_JUMPER); // Example I2C declaration, uncomment if using I2C

unsigned long previousMillis = 0;
uint8_t phase_display, update_sample;
uint32_t loadTime;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  //------Display LOGO at start------
  oled.begin();
  oled.clear(PAGE);
  oled.clear(ALL);
  oled.print("Connecting to wifi");
  oled.display();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  //  pzem.resetEnergy(); //reset energy
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000)
  { // run every 1 second
    previousMillis = currentMillis;
    loadTime += 5;
    update_sample += 5;

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

    //dummy data
//    voltage[i] = random(225, 230);
//    current[i] = random(1500, 2000) / 1000;
//    power[i] = voltage[i] * current[i] * 0.8;
//    energy[i] = 2;
//    frequency[i] = 50;
//    pf[i] = 0.8;


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

  // if no load (Ptotal less than 100 watt) clear time
  if (!isnan(power[0])) {
    if (power[0] + power[1] + power[2] < 100) {
      loadTime = 0;
    }
  }


    int hours = loadTime / 3600;
    int minutes = (loadTime % 3600) / 60;
    int seconds = (loadTime % 3600) % 60;

    float loadTimebase60 = hours + (minutes / 100.0);
    
  Serial.println("Load Time is : " + String(loadTimebase60));
  Serial.println(F("-------------------"));


  if (!isnan(power[0]) && !isnan(power[1]) && !isnan(power[2]) ) {
    if ( update_sample >= 30) {    // update every 30 second
      update_sample = 0;

      ThingSpeak.setField(1, (voltage[0] + voltage[1] + voltage[2]) / 3);
      ThingSpeak.setField(2, current[0] + current[1] + current[2]);
      ThingSpeak.setField(3, power[0] + power[1] + power[2]);
      ThingSpeak.setField(4, energy[0] + energy[1] + energy[2]);
      ThingSpeak.setField(5, (pf[0] + pf[1] + pf[2]) / 3);
      ThingSpeak.setField(6, loadTimebase60);

      // write to the ThingSpeak channel
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if (x == 200) {
        Serial.println("Channel update successful.");
      }
      else {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
    }
  }

}
