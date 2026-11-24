#include <Arduino.h>

// Libraries
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Global variables
  // RFID 
    #define RFID_SS  5
    #define RFID_RST 22
    String authorizedUIDs[] = {"53 34 DA 56"};
    int numAuthorized = 1;
    #define GREEN_LED 25
    #define RED_LED   26
    #define BUZZER    27
  // BMP
    float temperature;
    float pressure;
    float altitude;
    #define SCREEN_WIDTH 128
    #define SCREEN_HEIGHT 64
    #define OLED_MOSI 11
    #define OLED_CLK 13
    #define OLED_DC   9
    #define OLED_CS   10
    #define OLED_RST 8 
  // POTENTIOMETER
    int potValue;
    int potPin = 34; 
    


// Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

unsigned long indicatorStart = 0;
unsigned long indicatorTime = 0;
bool indicatorOn = false;
bool roomOccupied = false;

String lastUID = "";
unsigned long lastScanTime = 0;

// Objects
Adafruit_BMP280 bmp; 


MFRC522 rfid(RFID_SS, RFID_RST);

// Functions

// getUID() turns the card's UID bytes into text, for example "4A 1F 9C 02"
String getUID()
{
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
    {
      uid = uid + "0";
    }
    uid = uid + String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1)
    {
      uid = uid + " ";
    }
  }
  uid.toUpperCase();
  return uid;
}

bool isAuthorized(String uid)
{
  for (int i = 0; i < numAuthorized; i++)
  {
    if (uid == authorizedUIDs[i])
    {
      return true;
    }
  }
  return false;
}

void startIndicatorTimer(unsigned long duration)
{
  indicatorStart = millis();
  indicatorTime = duration;
  indicatorOn = true;
}

void allIndicatorsOff()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void onAccessGranted()
{
  Serial.println("ACCESS GRANTED");
  roomOccupied = true;
  allIndicatorsOff();
  digitalWrite(GREEN_LED, HIGH);
  startIndicatorTimer(2000);
}

void onAccessDenied()
{
  Serial.println("ACCESS DENIED");
  allIndicatorsOff();
  digitalWrite(RED_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(RED_LED, LOW);  
  startIndicatorTimer(1000);
}

void onTimerExpired()
{
  allIndicatorsOff();
  indicatorOn = false;
  Serial.println("Ready");
}

void onCardScanned(String uid)
{
  if (uid == lastUID && millis() - lastScanTime < 2000)
  {
    Serial.println("Repeat ignored");
    return;
  }
  lastUID = uid;
  lastScanTime = millis();

  Serial.print("Card UID: ");
  Serial.println(uid);

  if (isAuthorized(uid) == true)
  {
    onAccessGranted();
  }
  else
  {
    onAccessDenied();
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Program Started");

  // RFID code
  SPI.begin();
  rfid.PCD_Init();
  delay(10);
  rfid.PCD_DumpVersionToSerial();
  Serial.println("Tap a card...");

  // LED and buzzer code
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

}

void loop()
{
  // Card detection
  if (rfid.PICC_IsNewCardPresent() == true && rfid.PICC_ReadCardSerial() == true)
  {
    String uid = getUID();
    onCardScanned(uid);
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Timer
  if (indicatorOn == true && millis() - indicatorStart >= indicatorTime)
  {
    onTimerExpired();
  }

  temperature = bmp.readTemperature();
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" C");
  delay(2000);




}