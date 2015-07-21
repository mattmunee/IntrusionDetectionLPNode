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

#define NUMMEAS		100

typedef struct{
	byte nodeID;
	byte numMeas;
	float accelMag2[NUMMEAS];
}Payload;
Payload payLoad;

MMA8452Q accelerometer;

ArduinoLED led(LED);

int numints=0;
bool interruptCaught = false;

void setup()
{
	payLoad.nodeID = NODEID;
	payLoad.numMeas = NUMMEAS;
	Serial.begin(SERIAL_BAUD);
	Serial.println("Start...");
	
	led.Strobe(10,100);

	// Setup radio
	radio.initialize(FREQUENCY,NODEID,NETWORKID);
	radio.encrypt(ENCRYPTKEY);
	radio.sleep(); // MOTEINO: sleep right away to save power

	// Setup accelerometer
	accelerometer.init(SCALE_2G, ODR_50);
	accelerometer.setupMotionDetection(XY, 0.63, 0, INT_PIN2);
	accelerometer.setupAutoSleep(ODR_SLEEP_1,LOW_POWER, 0x08, 5.0);
	accelerometer.clearFFMotionInterrupt();

	// Setup Interrupts
	// Use hardware interrupt 1 for motion interrupt (hardware 0 is tied to radio)
	attachInterrupt(1,motionInterruptCaught,RISING);
	// Use pin change interrupt to catch sleep/wake interrupt
	cli();
	PCICR |= 0x04;		//enable pin change interrupt PCI2 pins [23:16]
	PCMSK2 |= 0x10;		//enable indiviual pins on PCI2, pin 20 = bit 4
	sei();

}

void loop()
{
	if (interruptCaught){
		byte numMeasurements = 0;
		while (numMeasurements < NUMMEAS){
			if (accelerometer.available()){
				accelerometer.read();
				payLoad.accelMag2[numMeasurements] = accelerometer.cx*accelerometer.cx
					+ accelerometer.cy*accelerometer.cy
					+ accelerometer.cz*accelerometer.cz;
				numMeasurements++;
			}
		}
		for (byte i = 0; i < NUMMEAS; i++){
			Serial.print(payLoad.accelMag2[i]);
			Serial.print(" ");
		}
		Serial.println("");
		Serial.println("");
		delay(100);
		accelerometer.clearFFMotionInterrupt();
		accelerometer.getSystemMode();
		interruptCaught = false;
	}
	else{
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
		radio.sleep();
	}

}

void motionInterruptCaught(){
	interruptCaught=true;
}

ISR(PCINT2_vect)
{
	interruptCaught = true;
}