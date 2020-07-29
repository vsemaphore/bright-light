#include "config.h"

#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <EEPROM.h>

int useHighBeam = false;
int lowBeamPower = 0;
int highBeamPower = 0;

bool previousTouch = false;
long touchInRow = 0;

void readLowBeam() {
  lowBeamPower = EEPROM.readInt(0); 
  Serial.printf("lowBeam loaded: %i\n", lowBeamPower); 
}

void writeLowBeam() {
  EEPROM.writeInt(0, lowBeamPower);
  EEPROM.commit();
}

void readHighBeam() {
  highBeamPower = EEPROM.readInt(1 * sizeof(int));  
  Serial.printf("highBeam loaded: %i\n", highBeamPower); 
  Serial.printf("highBeam size: %i\n", sizeof(highBeamPower)); 
}

void writeHighBeam() {
  EEPROM.writeInt(1 * sizeof(int), highBeamPower);
  EEPROM.commit();
}

void updateLight() {
  int power = useHighBeam ? highBeamPower : lowBeamPower;
  ledcWrite(BL_PWM_CH, power);
  Serial.printf("Updating to %s beam with %i power.\n", useHighBeam ? "high" : "low", power);
}

void handleTouch() {
  int touchValue = touchRead(BL_TOUCH_PIN);
  bool touch = (touchValue < BL_TOUCH_THR);

  if ( touch ) {
    if ( ++touchInRow >= BL_TOUCH_IN_ROW) {
      Serial.printf("Touch detected with last value of %i\n", touchValue);
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

void setup()
{
  Serial.begin(115200);
  Serial.printf("%s. Waiting for connections\n", BL_DEVICE_NAME);
  Blynk.setDeviceName(BL_DEVICE_NAME);
  Blynk.begin(BL_BLYNK_AUTH_KEY);

  EEPROM.begin(BL_EEPROM_SIZE);
  readLowBeam();
  readHighBeam();

  ledcSetup(BL_PWM_CH, BL_PWM_FREQ, BL_PWM_RES);
  ledcAttachPin(BL_LED_PIN, BL_PWM_CH);

  updateLight();
}

BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, useHighBeam);
  Serial.printf("READ useHighBeam: %i\n", useHighBeam);
}

BLYNK_WRITE(V0)
{
  useHighBeam = param.asInt();
  Serial.printf("WRITE useHighBeam: %i\n", useHighBeam);
  updateLight();
}

BLYNK_READ(V1)
{
  Blynk.virtualWrite(V1, lowBeamPower);
  Serial.printf("READ lowBeamPower: %i\n", lowBeamPower);
}

BLYNK_WRITE(V1)
{
  lowBeamPower = param.asInt();
  writeLowBeam();
  Serial.printf("WRITE lowBeamPower: %i\n", lowBeamPower);
  updateLight();
}

BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, highBeamPower);
  Serial.printf("READ highBeamPower: %i\n", highBeamPower);
}

BLYNK_WRITE(V2)
{
  highBeamPower = param.asInt();
  writeHighBeam();
  Serial.printf("WRITE highBeamPower: %i\n", highBeamPower);
  updateLight();
}

BLYNK_CONNECTED()
{
  Serial.println("Device connected. ");
  Blynk.virtualWrite(V0, useHighBeam);
  Blynk.virtualWrite(V1, lowBeamPower);
  Blynk.virtualWrite(V2, highBeamPower);
}



void loop()
{
  handleTouch();
  Blynk.run();
}
