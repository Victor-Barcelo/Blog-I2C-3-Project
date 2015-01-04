#include <Wire.h>
#include <Time.h>

#define SLAVE_ADDRESS 0x05
#define DEBUG true

const int PIN_TEMPSENSOR = 0;
const int PIN_PUSHBUTTON = 7;

// Debounce para el botón: (http://arduino.cc/en/Tutorial/Debounce)
int buttonState;
int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 50; 
//-------------------------

enum Command {SendSensorData, SendButtonInfo};
Command command = SendSensorData;

unsigned long pushButtonTimes[10];

void setup() {
    Serial.begin(9600);
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(recieveCommand);
	Wire.onRequest(sendData);
    if(DEBUG) Serial.println("Ready!");
}

void loop() {
  	delay(100);
 	checkForButtonPress();
}

void recieveCommand(int byteCount){
	int cmd = Wire.read();
	if(cmd == 0) configureTime();
	if(cmd == 1) command = SendSensorData;
	if(cmd == 2) command = SendButtonInfo;
}

void sendData(){
	if(command == SendSensorData){
		sendTempData();
	} else if(command == SendButtonInfo){
		sendButtonInfo();
	}
}

void sendTempData(){
	byte data[2];
	int temperature = (125*analogRead(PIN_TEMPSENSOR))>>8; // raw to Celsius
  	if(DEBUG){
  		Serial.print("Temperatura: ");
  		Serial.println(temperature);
  	}
  	data[0] = lowByte(temperature);	
  	data[1] = highByte(temperature);	
	byte dataSend[] = {data[0],data[1]};
	Wire.write(dataSend, 2);
}

void sendButtonInfo(){
	//send pushButtonTimes array
	//clear pushButtonTimes array
}

void checkForButtonPress(){
  	int reading = digitalRead(PIN_PUSHBUTTON);
  	if (reading != lastButtonState) {
    	lastDebounceTime = millis();
  	} 
  	if ((millis() - lastDebounceTime) > debounceDelay) {
    	if (reading != buttonState) {
      		buttonState = reading;
    		if (buttonState == HIGH) {
        		//Insert entry to pushButtonTimes
        		
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