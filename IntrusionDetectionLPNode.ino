#include <ArduinoLED.h>
#include <SPI.h>
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <SPIFlash.h>
#include <RFM69.h>
#include <LowPower.h>

#define SERIAL_BAUD      115200
#define LED  9            // LED is D9 on Motetino

RFM69 radio;       

#define NODEID        18    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!

typedef struct{
	float accelMag2;
}Payload;
Payload payLoad;

MMA8452Q accelerometer;

ArduinoLED led(LED);

int numints=0;
bool interruptCaught=false;

void setup()
{
	Serial.begin(SERIAL_BAUD);
	Serial.print("Start...");
	
	delay(100);          // wait for serial xmit
	pinMode(LED, OUTPUT);
	digitalWrite(LED,LOW);
	delay(1000);
	digitalWrite(LED,HIGH);    // flash LED for 1 msec (still quite visible)
	delay(1000);
	digitalWrite(LED,LOW);
	delay(1000);

	led.Strobe(10,100);

	radio.initialize(FREQUENCY,NODEID,NETWORKID);
	radio.encrypt(ENCRYPTKEY);
	radio.sleep(); // MOTEINO: sleep right away to save power

	accelerometer.init();
	//accelerometer.standby();
	//accelerometer.setupAutoWakeSleep();
	accelerometer.setupMotionDetection();
	accelerometer.clearFFMotionInterrupt();
	attachInterrupt(1,interrupt1Caught,FALLING);

	led.Strobe(10,100);

	
}

void loop() 
{
	if(interruptCaught){
		interruptCaught=false;
		numints++;
		Serial.println("\t\t\t\t\tcaught!");
		delay(100);
		accelerometer.clearFFMotionInterrupt();
	}
	Serial.println(numints);
	if(accelerometer.available()){
		accelerometer.read();
		Serial.print("\t");
		Serial.print(accelerometer.cx);
		Serial.print("\t");
		Serial.print(accelerometer.cy);
		Serial.print("\t");
		Serial.println(accelerometer.cz);
	}else{
		Serial.println("\t None available");
	}
	delay(100);
    /*led.Strobe(10,100);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
    led.Strobe(10,100);
    while(1)
    {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
    }*/
}
void interrupt1Caught(){
	interruptCaught=true;
}
