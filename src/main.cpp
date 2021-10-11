#include <Arduino.h>
#include <EVShield.h>
#include <EVs_EV3Gyro.h>
#include <EVs_EV3Ultrasonic.h>
#include <util/delay.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>
#include <EVs_EV3Touch.h>
#include <EVs_UIModule.h>
#include <EVs_EV3Color.h>

#define GYRO_SAMPLES 20

EVShield evshield(0x34, 0x36);
EVs_EV3Gyro gyro;
EVs_EV3Ultrasonic ultrasonic;
EVs_EV3Touch touch;
EVs_EV3Color colorSensor;
EVs_UIModule uim( 7, 8, 9 );

void zero_gyro() {
	int angle = gyro.getRefAngle();
	while(angle != 0){
		gyro.setRef();
		_delay_ms(100);
		angle = gyro.getRefAngle();
	}
}

void setup() {
  	Serial.begin(115200);
  	_delay_ms(500);
  	Serial.println(F("Hello there!"));
  	evshield.init( SH_HardwareI2C );

	gyro.init( &evshield, SH_BAS1 );
	gyro.setMode(MODE_Gyro_Angle);
	zero_gyro();

	ultrasonic.init(&evshield, SH_BAS2);
	colorSensor.init(&evshield, SH_BBS2);
	touch.init(&evshield, SH_BBS1);

	uim.begin();
	uim.setCursor(0, 0);	
	uim.clearScreen();
    uim.setTextColor(EVs_UIM_WHITE);  
	uim.setTextSize(2);

	evshield.bank_a.motorReset();

	uim.println(F("Press Go button to start the program"));
	while(!evshield.getButtonState(BTN_GO));
	uim.clearScreen();
	evshield.bank_a.ledSetRGB(255, 0, 0);
	_delay_ms(500);
	evshield.bank_a.ledSetRGB(0, 255, 0);
	_delay_ms(500);
	zero_gyro();
}

int expected_value = 90;

void loop() {
	int angle;
	angle = gyro.getRefAngle();
	uim.clearScreen();
	uim.setCursor(0, 0);
	char buf[20];
	snprintf_P(buf, 20, PSTR("Angle: %d\n"), angle);
	uim.println(buf);
	if (angle < expected_value) {
		evshield.bank_a.motorRunUnlimited(SH_Motor_1, SH_Direction_Reverse, 1);
		evshield.bank_a.motorRunUnlimited(SH_Motor_2, SH_Direction_Forward, 1);
	} else if(angle > expected_value) {
		evshield.bank_a.motorRunUnlimited(SH_Motor_1, SH_Direction_Forward, 1);
		evshield.bank_a.motorRunUnlimited(SH_Motor_2, SH_Direction_Reverse, 1);
	} else {
		evshield.bank_a.motorStop(SH_Motor_Both, SH_Next_Action_BrakeHold);
		expected_value += 90;
		if (expected_value >= 360) {
			expected_value = 0;
		}
		evshield.bank_a.motorRunRotations(SH_Motor_Both, SH_Direction_Reverse, SH_Speed_Slow, 3, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
	}
}