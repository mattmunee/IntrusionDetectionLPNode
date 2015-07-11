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
	Serial.println("Start...");
	
	//led.Strobe(10,100);

	radio.initialize(FREQUENCY,NODEID,NETWORKID);
	radio.encrypt(ENCRYPTKEY);
	radio.sleep(); // MOTEINO: sleep right away to save power

	accelerometer.init(SCALE_2G, ODR_800);
	accelerometer.setupMotionDetection(XY, 0.63, 0, INT_PIN2);
	accelerometer.setupAutoSleep(ODR_SLEEP_50,NORMAL, 0x08, 5.0);
	accelerometer.clearFFMotionInterrupt();

	attachInterrupt(1,interrupt1Caught,FALLING);
	cli();
	PCICR |= 0x04;		//enable pin change interrupt 2 pins [23:16]
	PCMSK2 |= 0x10;		//enable indiviual pins on PCI2, pin 20 = bit 4
	sei();

	//led.Strobe(10,100);

}

void loop() 
{
	Serial.print("Interrupt");
	Serial.println(accelerometer.getInterruptSources());
	if (accelerometer.getSystemMode() == 0)Serial.println("STANDBY");
	else if (accelerometer.getSystemMode() == 1)Serial.println("WAKE");
	else Serial.println("SLEEP");
	if(interruptCaught){
		Serial.println("\t\t\t\t\tcaught!");
		interruptCaught=false;
		numints++;
		delay(100);
		accelerometer.clearFFMotionInterrupt();
	}
	Serial.println(numints);
	delay(100);
	if(accelerometer.available()){
		Serial.print("Data Available");
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
}

void interrupt1Caught(){
	interruptCaught=true;
}

ISR(PCINT2_vect)
{
	interruptCaught = true;
}