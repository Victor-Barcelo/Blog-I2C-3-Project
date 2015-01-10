#include <Wire.h>
#include <Time.h>
#include <StackArray.h> // http://playground.arduino.cc/Code/StackArray

#define SLAVE_ADDRESS 0x05
#define DEBUG true

const int PIN_TEMPSENSOR = 0;
const int PIN_PUSHBUTTON = 7;

// Button debounce: (http://arduino.cc/en/Tutorial/Debounce)
int buttonState;
int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 50; 
//-------------------------

enum Command {None, SendSensorData, SendBtnDataLen, SendBtnData};
Command command = None;

StackArray <unsigned long> btnStack;

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(recieveCommand);
  Wire.onRequest(sendData);
  if(DEBUG) Serial.println("Ready!");
}

void loop() {
  delay(100);
  checkBtnState();
}

void recieveCommand(int byteCount){
  int cmd = Wire.read();
  if(cmd == 0) configureTime();
  if(cmd == 1) command = SendSensorData;
  if(cmd == 2) command = SendBtnDataLen;  
  if(cmd == 3) command = SendBtnData;
}

void sendData(){
  switch(command) {
    case SendSensorData: 
      sendTempData();
      command = None;
      break;
    case SendBtnDataLen:
      sendBtnDataLen();
      command = None;       
      break;
    case SendBtnData:
      sendBtnData();
      command = None;       
      break;
  }
}

void sendBtnDataLen(){
  Wire.write(btnStack.count()*4);
}

void sendBtnData(){
  unsigned long btnTime = 0;
  int numBytes = btnStack.count()*4;
  byte dataSend[numBytes];
  int i = 0;
  while (!btnStack.isEmpty()){
    btnTime = btnStack.pop();
    dataSend[i+(3*i)]   = btnTime & 0x000000ff;
    dataSend[i+1+(3*i)] = (btnTime & 0x0000ff00) >> 8;
    dataSend[i+2+(3*i)] = (btnTime & 0x00ff0000) >> 16;
    dataSend[i+3+(3*i)] = (btnTime & 0xff000000) >> 24;
    i++;
  }
  Wire.write(dataSend, numBytes);
}

void sendTempData(){
  int temperature = (125*analogRead(PIN_TEMPSENSOR))>>8; // raw to Celsius
  if(DEBUG){
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }
  Wire.write(temperature);
}

void checkBtnState(){
  int reading = digitalRead(PIN_PUSHBUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        btnStack.push(now());
      }
    }
  }
  lastButtonState = reading;
}

void configureTime(){
  unsigned long bytes[4];
  int i = 0;
  while(Wire.available()){
    bytes[i] = Wire.read();
    i++;
  }
  unsigned long result = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
  setTime(result);
}