#include "config.h"

#include <BlynkSimpleEsp32_BLE.h>
#include <EEPROM.h>
#include "BLE.h"

#define LOG(str) terminal.print(str);Serial.print(str);
#define LOGLN(str) terminal.println(str);Serial.println(str);
#define LOGVAL(str, val) terminal.print(str);terminal.println(val);Serial.print(str);Serial.println(val);

BLE BT;

bool useHighBeam = false;
int lowBeamPower = 0;
int highBeamPower = 0;
int beamThr = BL_TOUCH_THR;

bool previousTouch = false;
long touchInRow = 0;

bool OTAmode = false;

WidgetTerminal terminal(V9);

void readLowBeam() {
  lowBeamPower = EEPROM.readInt(0); 
  LOGVAL("lowBeam loaded: ", lowBeamPower); 
}

void writeLowBeam() {
  EEPROM.writeInt(0, lowBeamPower);
  EEPROM.commit();
}

void readHighBeam() {
  highBeamPower = EEPROM.readInt(1 * sizeof(int));  
  LOGVAL("highBeam loaded: ", highBeamPower);
}

void writeHighBeam() {
  EEPROM.writeInt(1 * sizeof(int), highBeamPower);
  EEPROM.commit();
}

void readOTAmode() {
  OTAmode = (bool)EEPROM.readInt(2 * sizeof(int));  
}

void writeOTAmode() {
  EEPROM.writeInt(2 * sizeof(int), OTAmode);
  EEPROM.commit();
}

void overwriteOTAmode(bool value) {
  LOGVAL("overwriteOTAmode: ", value);
  EEPROM.writeInt(2 * sizeof(int), value);
  EEPROM.commit();
}

void readBeamThr() {
  beamThr = EEPROM.readInt(3* sizeof(int));  
  LOGVAL("beamThr loaded: ", beamThr);
}

void writeBeamThr() {
  EEPROM.writeInt(3 * sizeof(int), beamThr);
  EEPROM.commit();
}

void updateLight() {
  int power = useHighBeam ? highBeamPower : lowBeamPower;
  ledcWrite(BL_PWM_CH, power);
  LOG("Updating beam: "); LOG(useHighBeam ? "high:  " : "low: "); LOGLN(power);
}

void handleTouch() {
  int touchValue = touchRead(BL_TOUCH_PIN);
  bool touch = (touchValue < BL_TOUCH_THR);

  if ( touch ) {
    if ( ++touchInRow >= BL_TOUCH_IN_ROW) {
      LOGVAL("Touch detected: ", touchValue);
      
      useHighBeam = !useHighBeam;
      touchInRow = 0;
      Blynk.virtualWrite(V0, useHighBeam);
      updateLight();
      delay(BL_TOUCH_CHANGE_TIMEOUT); 
    }
  } else {
    touchInRow = 0;
  }

  delay(BL_TOUCH_TIMEOUT);
}

BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, useHighBeam);
  LOGVAL("READ useHighBeam: ", useHighBeam);
}

BLYNK_WRITE(V0)
{
  useHighBeam = param.asInt();
  LOGVAL("WRITE useHighBeam: ", useHighBeam);
  updateLight();
}

BLYNK_READ(V1)
{
  Blynk.virtualWrite(V1, lowBeamPower);
  LOGVAL("READ lowBeamPower: ", lowBeamPower);
}

BLYNK_WRITE(V1)
{
  lowBeamPower = param.asInt();
  writeLowBeam();
  LOGVAL("WRITE lowBeamPower: ", lowBeamPower);
  updateLight();
}

BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, highBeamPower);
  LOGVAL("READ highBeamPower: ", highBeamPower);
}

BLYNK_WRITE(V2)
{
  highBeamPower = param.asInt();
  writeHighBeam();
  LOGVAL("WRITE highBeamPower: ", highBeamPower);
  updateLight();
}

BLYNK_WRITE(V3)
{
  overwriteOTAmode(true);
  LOGLN("WRITE OTAmode\nRebooting");
  ESP.restart();
}

BLYNK_READ(V4)
{
  Blynk.virtualWrite(V4, beamThr);
  LOGVAL("READ beamThr: ", beamThr);
}

BLYNK_WRITE(V4)
{
  beamThr = param.asInt();
  writeBeamThr();
  LOGVAL("WRITE beamThr: ", beamThr);
  updateLight();
}

BLYNK_CONNECTED()
{
  LOGLN("Device connected. ");
  Blynk.virtualWrite(V0, useHighBeam);
  Blynk.virtualWrite(V1, lowBeamPower);
  Blynk.virtualWrite(V2, highBeamPower);
}

void setup()
{
  Serial.begin(115200);
  Serial.printf("%s. Booting up :)\n", BL_DEVICE_NAME);

  EEPROM.begin(BL_EEPROM_SIZE);
  readLowBeam();
  readHighBeam();
  readOTAmode();

  ledcSetup(BL_PWM_CH, BL_PWM_FREQ, BL_PWM_RES);
  ledcAttachPin(BL_LED_PIN, BL_PWM_CH);

  updateLight();

  if (OTAmode) {
    LOGLN("MODE: OTA");
    
    BT.begin(BL_DEVICE_NAME " OTA");
    overwriteOTAmode(false);
  } else {
    LOGLN("MODE: Blynk");

    Blynk.setDeviceName(BL_DEVICE_NAME);
    Blynk.begin(BL_BLYNK_AUTH_KEY);
  }

}

void loop()
{
  if ( ! OTAmode) {
    handleTouch();
    Blynk.run();  
  }
}
