/*
 Name:		Arduino_Prog.ino
 Created:	11/29/2015 6:34:45 PM
 Author:	Mateusz
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MMA8451.h>
#include <LiquidCrystal.h>
#include <math.h>

LiquidCrystal lcd(7, 12, 5, 4, 3, 2);

// INITIALIZATIONS

Adafruit_MMA8451 mma = Adafruit_MMA8451();
const int numMMA = 10;
int indexMMA = 0;
float totalMMA_x = 0;
float sredMMA_x = 0;
float x[numMMA];
float totalMMA_y = 0;
float sredMMA_y = 0;
float y[numMMA];

int Sharp = A0;
const int numSharp = 10;
int indexSharp = 0;
int totalSharp = 0;
float sredSharp = 0;
int odczyty_Sharp[numMMA];

int Velocity_read = A1;

float Press = 2;

const int ledPin_red = 9;
const int ledPin_green = 10;
const int ledPin_blue = 11;
int ledState_R = LOW;
int ledState_G = LOW;
int ledState_B = LOW;

const int Button_pin = 8;
int buttonState = 0;
int lastButtonState = 0;
int button_counter = 1;

const int Button_pin2 = 13;
int buttonState2 = 0;
int lastButtonState2 = 0;
int button_counter2 = 0;


int manual = 1;
int last_manual = 0;
int manual_counter = 0;
int Velocity_range = 0;
int last_Velocity = 0;
int loop_counter;


int serial_in = 0;

const float pi = 3.1415926535897932384626433;

unsigned long prevMillis = 0;
unsigned long prevMillis2 = 0;
unsigned long prevMillis3 = 0;
float Press_time = 2;

// RGB LED - shows damper settings
void LED() 
{

	if (manual == 1)
	{
		serial_in = button_counter;
	}

	if (Serial.available() > 0)
	{
		if (button_counter == 3/*manual != 1*/)
		{
			serial_in = Serial.read() - 48;
		}
	}

	switch (serial_in) {
	case 0:
		ledState_R = 255;
		ledState_G = 255;
		ledState_B = 150;
		analogWrite(ledPin_red, ledState_R);
		analogWrite(ledPin_green, ledState_G);
		analogWrite(ledPin_blue, ledState_B);
		break;
	case 1:
	case 3:
		ledState_R = 255;
		ledState_G = 150;
		ledState_B = 255;
		analogWrite(ledPin_red, ledState_R);
		analogWrite(ledPin_green, ledState_G);
		analogWrite(ledPin_blue, ledState_B);
		break;
	case 2:
		ledState_R = 150;
		ledState_G = 255;
		ledState_B = 255;
		analogWrite(ledPin_red, ledState_R);
		analogWrite(ledPin_green, ledState_G);
		analogWrite(ledPin_blue, ledState_B);
		break;
	default:
		for (int ledPin = 9; ledPin < 12; ledPin++)
		{
			digitalWrite(ledPin, HIGH);
		}
	}

}

// Buttons operation
void Button_count() 
{
	buttonState2 = digitalRead(Button_pin2);
	if (button_counter2 == 2)
	{
		button_counter2 = 0;
	}
	if (buttonState2 != lastButtonState2)
	{
		if (buttonState2 == HIGH) {
			lcd.clear();
			button_counter2++;
		}
		delay(10);
	}
	lastButtonState2 = buttonState2;


	buttonState = digitalRead(Button_pin);
	if (button_counter == 4)
	{
		manual = 1;
		button_counter = 0;
	}
	if (buttonState != lastButtonState)
	{
		manual = 1;
		if (buttonState == HIGH) {
			button_counter++;
		}
		delay(10);
	}
	lastButtonState = buttonState;


}

// Suspension settings: height and auto-corrections dependent on speed and time
float Spring_mode(int V) 
{
	float Spring_ = 0;
	int Velocity = V;
	unsigned long curMillis = millis();

	if (manual != last_manual || button_counter != manual_counter || Velocity_range != last_Velocity)
	{
		prevMillis = curMillis;
		prevMillis2 = curMillis;
		prevMillis3 = curMillis;
	}


	if (manual == 1)
	{
		switch (button_counter) {
		case 0:
			Spring_ = 3;
			break;
		case 1:
			Spring_ = 2;
			break;
		case 2:
			Spring_ = 1.5;
			break;
		default:
			Spring_ = 2;
		}
	}

	last_Velocity = Velocity_range;

	if (Velocity < 120 && Velocity > 50)
	{
		Velocity_range = 1;
		if (curMillis - prevMillis >= 5000)
		{
			manual = 0;
			Press_time = 2;
			prevMillis = curMillis;
			prevMillis2 = curMillis;
			prevMillis3 = curMillis;
		}
	}

	if (Velocity <= 50)
	{
		Velocity_range = 0;
		if (curMillis - prevMillis2 >= 5000)
		{
			manual = 0;
			Press_time = 3;
			prevMillis = curMillis;
			prevMillis2 = curMillis;
			prevMillis3 = curMillis;
		}
	}

	if (Velocity >= 120)
	{
		Velocity_range = 2;
		if (curMillis - prevMillis3 >= 5000)
		{
			manual = 0;
			Press_time = 1.5;
			prevMillis = curMillis;
			prevMillis2 = curMillis;
			prevMillis3 = curMillis;
		}
	}


	if (manual == 0)
	{
		Spring_ = Press_time;
	}

	last_manual = manual;
	manual_counter = button_counter;

	delay(10);
	return Spring_;
}

// Second LCD Mode Function
void LCD_mode2(int V) 
{
	int Velocity = V;
	if (button_counter2 == 1)
	{
		switch (button_counter) {
		case 0:
			lcd.setCursor(15, 1);
			lcd.print("C");
			break;
		case 1:
			lcd.setCursor(15, 1);
			lcd.print("N");
			break;
		case 2:
			lcd.setCursor(15, 1);
			lcd.print("S");
			break;
		case 3:
			lcd.setCursor(15, 1);
			lcd.print("A");
			break;
		default:
			lcd.setCursor(15, 1);
			lcd.print("-");
		}

		lcd.setCursor(0, 0);
		lcd.print("V [km/h]: ");
		lcd.setCursor(10, 0);
		lcd.print(Velocity); lcd.print("   ");
	}
}

// Processing readings of an accelerometer
void MMA()
{
	if (button_counter2 == 0)
	{
		sensors_event_t event;
		mma.getEvent(&event);

		totalMMA_x = totalMMA_x - x[indexMMA];
		totalMMA_y = totalMMA_y - y[indexMMA];
		x[indexMMA] = event.acceleration.x;
		y[indexMMA] = event.acceleration.y;

		if (event.acceleration.x < -1)
		{
			x[indexMMA] = -1;
		}
		else if (event.acceleration.x > 1)
		{
			x[indexMMA] = 1;
		}

		if (event.acceleration.y < -1)
		{
			y[indexMMA] = -1;
		}
		else if (event.acceleration.y > 1)
		{
			y[indexMMA] = 1;
		}

		totalMMA_x = totalMMA_x + x[indexMMA];
		totalMMA_y = totalMMA_y + y[indexMMA];

		indexMMA++;
		if (indexMMA >= numMMA) {
			indexMMA = 0;
		}

		sredMMA_x = totalMMA_x / numMMA;
		sredMMA_y = totalMMA_y / numMMA;

		float angleX_rad = acos((sredMMA_x / 1));
		float angleX = angleX_rad * 180 / pi - 90;
		float angleY_rad = acos((sredMMA_y / 1));
		float angleY = angleY_rad * 180 / pi - 90;

		//float LP = angleX;
		//float PP = angleY;
		float PL = Press + 0.4 * (angleX + angleY) / 90;
		//float LP = angleX + angleY;
		float PP = Press + 0.4 * (angleX - angleY) / 90;
		//float LT = Press;
		//float PT = Press;
		float TL = Press + 0.4 * (-angleX + angleY) / 90;
		float TP = Press + 0.4 * (-angleX - angleY) / 90;

		if (PL < Press)
			PL = Press;
		if (PP < Press)
			PP = Press;
		if (TL < Press)
			TL = Press;
		if (TP < Press)
			TP = Press;

		lcd.setCursor(0, 0);
		lcd.print("PL:"); lcd.print(PL); lcd.print(" ");
		//lcd.print("X"); lcd.print(angleX);
		lcd.setCursor(9, 0);
		lcd.print("PP:"); lcd.print(PP); lcd.print(" ");
		lcd.setCursor(0, 1);
		lcd.print("TL:"); lcd.print(TL); lcd.print(" ");
		lcd.setCursor(9, 1);
		//lcd.print(button_counter2);
		//lcd.print("TP:"); lcd.print(TP); lcd.print(" ");

		delay(10);
	}
}

// Rangefinder readings and signal processing
float Sharp_read(float Pressure) 
{
	totalSharp = totalSharp - odczyty_Sharp[indexSharp];
	odczyty_Sharp[indexSharp] = analogRead(Sharp);
	totalSharp = totalSharp + odczyty_Sharp[indexSharp];
	indexSharp++;
	if (indexSharp >= numSharp)
	{
		indexSharp = 0;
	}

	sredSharp = totalSharp / numSharp;
	float Voltage = sredSharp * 5 / 1023;
	float L = 0;
	L = 10 * ((12.632187 / (Voltage - 0.031031)) - 0.42); // Approximation based on the datasheet
	float demand_L = 130;

	//lcd.setCursor(0, 1);
	//lcd.print(L); lcd.print(" ");
	int Pressure_switch = Pressure;
	switch (Pressure_switch/*button_counter*/) {
	case 3:
		demand_L = 170;
		break;
	case 2:
		demand_L = 130;
		break;
	case 1:
		demand_L = 100;
		break;
	default:
		Press = 2;
	}

	if ((demand_L - L) > 0)
	{
		if ((demand_L - L) <= 40)
		{
			Pressure = Pressure + (demand_L - L) * 0.5 / 40;
		}
		else
		{
			Pressure += 0.5;
		}
	}

	Press = Pressure;
	return L;
}

void Sharp_disp(float L_)
{
	float L = L_;
	if (button_counter2 == 1)
	{
		lcd.setCursor(0, 1);
		lcd.print(L); lcd.print(" ");
	}
}

int Velo()
{
	int Velocity = 0;
	Velocity = (analogRead(Velocity_read) / 5);
	return Velocity;
}

// RESET and Setup
void setup() { 
	Serial.begin(57600);
	mma.begin();
	mma.setRange(MMA8451_RANGE_2_G);

	pinMode(Button_pin, INPUT);

	pinMode(ledPin_red, OUTPUT);
	pinMode(ledPin_green, OUTPUT);
	pinMode(ledPin_blue, OUTPUT);

	for (int ledPin = 9; ledPin < 12; ledPin++)
	{
		digitalWrite(ledPin, HIGH);
	}

	lcd.begin(16, 2);
}

// the loop function runs over and over again until power down or reset
// MAIN
void loop() { 
	lcd.setCursor(10, 1);
	lcd.print(manual);
	
	LED();
	LCD_mode2(Velo());
	Sharp_disp(Sharp_read(Spring_mode(Velo())));
	MMA();
	Button_count();
	lcd.setCursor(12, 1);
	lcd.print(button_counter);
	lcd.setCursor(10, 1);
	lcd.print(manual);
	delay(20);
}
