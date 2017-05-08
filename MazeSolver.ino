#include <QTRSensors.h>
#include "Direction.h"

const int numberOfSensors = 6;
// pid loop vars
const float proportionalConst = 0.1f;
const float derivateConst = 1.0f;
const int calibrationSeconds = 4;
const int twoHundred = 200;

int speed_a = 3;  //speed control for motor outputs 1 and 2 is on digital pin 10  (Right motor)
int speed_b = 11; //speed control for motor outputs 3 and 4 is on digital pin 11  (Left motor)
int dir_a = 12;  //direction control for motor outputs 1 and 2 is on digital pin 12  (Right motor)
int dir_b = 13;  //direction control for motor outputs 3 and 4 is on digital pin 13  (Left motor)

// motor tuning vars for maze navigating
int turnSpeed = 200;  // tune value motors will run while turning (0-255) NOT TESTED
int turnSpeedSlow = 125;  // tune value motors will run as they slow down from turning cycle to avoid overrun (0-255) NOT TESTED
int drivePastDelay = 300; // tune value in mseconds motors will run past intersection to align wheels for turn NOT TESTED


unsigned char pins[6] = { 0, 1, 2, 3, 4, 5 };
QTRSensorsAnalog qtra(pins, numberOfSensors, 4, 2);
unsigned int sensorValues[numberOfSensors];

Direction direction = forward;

unsigned int position;
int lastError;
int loopIndex;

void setup()
{
	pinMode(12, OUTPUT);
	pinMode(13, OUTPUT);

	Serial.begin(9600);

	delay(500);

	calibrate();

	Serial.println('\n');
	delay(1000);
}

void loop()
{
	switch (Serial.read())
	{
	case '1':
		direction = none;
		break;
	case '2':
		direction = forward;
		break;
	default:
		break;
	}

	if (loopIndex % 100 == 0)
	{
		printSensorValues();
	}

	drive();

	loopIndex++;
}

void calibrate()
{
	for (int i = 0; i < calibrationSeconds / 0.025; i++)
	{
		qtra.calibrate();
	}

	// print the calibration minimum values measured when emitters were on
	for (int i = 0; i < numberOfSensors; i++)
	{
		Serial.print(qtra.calibratedMinimumOn[i]);
		Serial.print(' ');
	}
	Serial.println();

	// print the calibration maximum values measured when emitters were on
	for (int i = 0; i < numberOfSensors; i++)
	{
		Serial.print(qtra.calibratedMaximumOn[i]);
		Serial.print(' ');
	}
}

void drive()
{
	if (direction == none)
	{
		moveMotorOnSide(left, 0);
		moveMotorOnSide(right, 0);
		return;
	}

	position = qtra.readLine(sensorValues);
	int posPropotionalToMid = position - 2500;

	int motorSpeed = proportionalConst * posPropotionalToMid + derivateConst * (posPropotionalToMid - lastError);
	lastError = posPropotionalToMid;

	moveMotorOnSide(left, twoHundred - motorSpeed);
	//Serial.print(' ');
	moveMotorOnSide(right, twoHundred + motorSpeed);

}

void moveMotorOnSide(Direction side, int speed)
{
	speed = max(min(speed, 180), 0);
	//Serial.print("Motor ");
	//Serial.print(side == left ? "left " : "right ");
	//Serial.print(' ');
	//Serial.print(speed);
	digitalWrite(side == left ? 13 : 12, HIGH);
	analogWrite(side == left ? 3 : 11, speed);
}

void printSensorValues()
{
	for (unsigned char i = 0; i < numberOfSensors; i++)
	{
		Serial.print(sensorValues[i]);
		Serial.print('\t');
	}

	Serial.println((int)position - 2500);
}
// Turns to the sent variable of
// 'L' (left), 'R' (right), 'S' (straight), or 'B' (back)
// To Do: Tune 'turnSpeed'
void turn(char dir)
{
  switch(dir)
  {
    // Turn left 90deg
    case 'L':    
      digitalWrite(dir_a, HIGH); 
      analogWrite(speed_a, turnSpeed);
      digitalWrite(dir_b, LOW);  
      analogWrite(speed_b, turnSpeed);
      
      position = qtrra.readLine(sensorValues);
      
      
      while (sensorValues[5] <300)  // wait for outer most sensor to find the line
      {
        position = qtrra.readLine(sensorValues);
      }
  
      // slow down speed
      analogWrite(speed_a, turnSpeedSlow);
      analogWrite(speed_b, turnSpeedSlow); 
      
      // find center
      while (position > 3000)  // tune - wait for line position to find near center
      {
        position = qtrra.readLine(sensorValues); //possible bug: libary uses position to calculate the next val 
      }
     
      // stop both motors
      analogWrite(speed_b, 0);  // stop right motor first to better avoid over run
      analogWrite(speed_a, 0);  
      break;
      
    // Turn right 90deg
    case 'R':        
      digitalWrite(dir_a, LOW); 
      analogWrite(speed_a, turnSpeed);
      digitalWrite(dir_b, HIGH);  
      analogWrite(speed_b, turnSpeed);
           
      line_position = qtrra.readLine(sensorValues);
      
      while (sensorValues[0] <300)  // wait for outer most sensor to find the line
      {
        position = qtrra.readLine(sensorValues);
      }
    
      // slow down speed
      analogWrite(speed_a, turnSpeedSlow);
      analogWrite(speed_b, turnSpeedSlow); 
      
      // find center
      while (position < 3000)  // tune - wait for line position to find near center
      {
        position = qtrra.readLine(sensorValues);
      }
     
      // stop both motors
      analogWrite(speed_a, 0);  
      analogWrite(speed_b, 0);      
      break;
    
    // Turn right 180deg to go back
    case 'B':    
      digitalWrite(dir_a, LOW); 
      analogWrite(speed_a, turnSpeed);
      digitalWrite(dir_b, HIGH);  
      analogWrite(speed_b, turnSpeed);
      
      position = qtrrc.readLine(sensorValues);
  
      while (sensorValues[0] <300)  // wait for outer most sensor to find the line
      {
        position = qtrra.readLine(sensorValues);
      }
       
      // slow down speed
      analogWrite(speed_a, turnSpeedSlow);
      analogWrite(speed_b, turnSpeedSlow); 
      
      // find center
      while (position < 300)  // tune - wait for line position to find near center
      {
        position = qtrrc.readLine(sensorValues);
      }
     
      // stop both motors
      analogWrite(speed_a, 0);  
      analogWrite(speed_b, 0);           
      break;

    // Straight ahead
    case 'S':
      // do nothing
      break;
  }
} // end turn

void MazeSolve()
{
  while(1)
  {
    
    follow_line();

    // Drive straight a bit.
    digitalWrite(dir_a, LOW);  
    analogWrite(pwm_a, 200);
    digitalWrite(dir_b, LOW);  
    analogWrite(pwm_b, 200);   
    delay(25); 

    // These variables record whether the robot has seen a line to the
    // left, straight ahead, and right, whil examining the current
    // intersection.
    unsigned char found_left=0;
    unsigned char found_straight=0;
    unsigned char found_right=0;
    
    // Now read the sensors and check the intersection type.
    line_position = qtrra.readLine(sensorValues);

    // Check for left and right exits.
    if(sensorValues[0] > 400)
    found_right = 1;
    if(sensorValues[5] > 400)
    found_left = 1;

    // Drive straight a bit more 
    digitalWrite(dir_a, LOW);  
    analogWrite(pwm_a, 200);
    digitalWrite(dir_b, LOW);  
    analogWrite(pwm_b, 200);
    delay(drivePastDelay); 
  
    line_position = qtrra.readLine(sensorValues);
   // if(sensorValues[1] > 200 || sensorValues[2] > 200 || sensorValues[3] > 200 || sensorValues[4] > 200) //To Do: Which sensor values indicate a straight
   // found_straight = 1;
    
   // unsigned char dir = select_turn(found_left, found_straight, found_right); //To Do: this method

    // Make the turn indicated by the path.
    turn(dir);
    //To Do: End of the maze
  }
