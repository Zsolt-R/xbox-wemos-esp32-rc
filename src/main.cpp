#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <SparkFun_TB6612.h>
#include <Servo.h>

// Setup Servo
Servo myservo = Servo();

// Required to replace with your xbox address
// XboxSeriesXControllerESP32_asukiaaa::Core
// xboxController("44:16:22:5e:b2:d4");

// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define PWMA 4
#define AIN2 5
#define AIN1 6

#define PWMB 16
#define BIN1 7
#define BIN2 15

#define STBY 11

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

const int potPin1 = 0, servoPin1 = 12;

// Initialize motors
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// Returns true if x is in range [low..high], else false    
bool inRange(unsigned low, unsigned high, unsigned x)          
{          
 return (low <= x && x <= high);          
}    


int calculateSpeed(int inputValue) {

  // Map the input range to the output range
  int outputValue = 0;  

  // Forward
  if(inputValue > 350){
    outputValue = map(inputValue, 350, 655, 0, 255);
  }

  // Backward
  if(320 > inputValue){
    outputValue = map(inputValue, 320, 0, 0, 255);
  }

  // Ensure that the output value is within the range of 0 to 255
  outputValue = constrain(outputValue, 0, 255);
 
  return outputValue;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");
  xboxController.begin();
}


void loop() {
  xboxController.onLoop();

  if (xboxController.isConnected()) {
    if (xboxController.isWaitingForFirstNotification()) {
      Serial.println("waiting for first notification");
    } else {

      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy;

      // Serial.println("Address: " + xboxController.buildDeviceAddressStr());      
      // unsigned long receivedAt = xboxController.getReceiveNotificationAt();
      // Serial.print("joyLHori rate: ");
      // Serial.println((float)xboxController.xboxNotif.joyLHori / joystickMax);
      // Serial.print("joyLVert rate: ");
      // Serial.println((float)xboxController.xboxNotif.joyLVert / joystickMax);
      // Serial.println("battery " + String(xboxController.battery) + "%");
      // Serial.println("received at " + String(receivedAt));
      // Serial.print((int)xboxController.xboxNotif.joyRVert / 100);

      int controllerRVSpeed = (int)xboxController.xboxNotif.joyRVert / 100;
      int controllerRHSpeed = (int)xboxController.xboxNotif.joyRHori / 100;

      int speed = calculateSpeed(controllerRVSpeed);
      int speedTurn = calculateSpeed(controllerRHSpeed);

      int directionMotor1 = 1;
      int directionMotor2 = 1;

      bool isTurning = speedTurn > speed;

      // Use of the drive function which takes as arguements the speed
      // and optional duration.  A negative speed will cause it to go
      // backwards.  Speed can be from -255 to 255.  Also use of the 
      // brake function which takes no arguements.
      // ex motor2.drive(255.1000);       

      if(isTurning){
        speed = speedTurn;  

        // Turn left
        if(!inRange(350, 655, controllerRHSpeed) && isTurning ){       
          directionMotor1 = -1;
          directionMotor2 = 1;

        // Turn right
        }else{
          directionMotor1 = 1;
          directionMotor2 = -1;
        }

      }else{
        speed = speed;
        // Backward
        if(!inRange(320, 655, controllerRVSpeed) )
        {
          directionMotor1 = 1;
          directionMotor2 = 1;
        }else{
          directionMotor1 = -1;
          directionMotor2 = -1;
        }
      }

      motor1.drive(speed * directionMotor1);  
      motor2.drive(speed * directionMotor2);       

      // Servo
      int val1 = (float)xboxController.xboxNotif.joyLHori / 100;
      int servoVal = map(val1, 655, 0, 0, 80);  // align pot pointer to servo arm 
      myservo.write(servoPin1, servoVal);      // set the servo position (degrees)

      // Serial.print("Speed: ");
      // Serial.print(speed );
      // Serial.println("");
      // Serial.print("Is Turning: ");
      // Serial.print(isTurning ? "true" : "false");
      // Serial.println("");
      // Serial.print("Motor 1: ");
      // Serial.print(directionMotor1);
      // Serial.println("");
      // Serial.print("Motor 2: ");
      // Serial.print(directionMotor2);
      // Serial.println("");
      // Serial.print("Servo: ");
      // Serial.print(val1);
      // Serial.println("");
      // Serial.print("ServoVal: ");
      // Serial.print(servoVal);
      // Serial.println("");
    }
  } else {
    Serial.println("not connected");
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
  Serial.println("at " + String(millis()));
  delay(10);
}
