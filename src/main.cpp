#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <SparkFun_TB6612.h>
#include <Servo.h>
#include <WS2812FX.h>

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

// LED
#define LED_COUNT 84
#define LED_PIN 42
#define COLOR_ORDER GRB
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Effects, list all the avaliable effects
#define NUM_EFFECTS 56

int lastButtonStateRB = 0; // the last state of the button
int lastButtonStateLB = 0; // the last state of the button
int lastButtonStateB = 0; // the last state of the button
int lastButtonStateX = 0; // the last state of the button

int ledStatus = 0; // Enable or disable the led

int currentLedEffect = 0;

const uint8_t ledEffects[] = {
  FX_MODE_STATIC,
  FX_MODE_BLINK,
  FX_MODE_BREATH,
  FX_MODE_COLOR_WIPE,
  FX_MODE_COLOR_WIPE_INV,
  FX_MODE_COLOR_WIPE_REV,
  FX_MODE_COLOR_WIPE_REV_INV,
  FX_MODE_COLOR_WIPE_RANDOM,
  FX_MODE_RANDOM_COLOR,
  FX_MODE_SINGLE_DYNAMIC,
  FX_MODE_MULTI_DYNAMIC,
  FX_MODE_RAINBOW,
  FX_MODE_RAINBOW_CYCLE,
  FX_MODE_SCAN,
  FX_MODE_DUAL_SCAN,
  FX_MODE_FADE,
  FX_MODE_THEATER_CHASE,
  FX_MODE_THEATER_CHASE_RAINBOW,
  FX_MODE_RUNNING_LIGHTS,
  FX_MODE_TWINKLE,
  FX_MODE_TWINKLE_RANDOM,
  FX_MODE_TWINKLE_FADE,
  FX_MODE_TWINKLE_FADE_RANDOM,
  FX_MODE_SPARKLE,
  FX_MODE_FLASH_SPARKLE,
  FX_MODE_HYPER_SPARKLE,
  FX_MODE_STROBE,
  FX_MODE_STROBE_RAINBOW,
  FX_MODE_MULTI_STROBE,
  FX_MODE_BLINK_RAINBOW,
  FX_MODE_CHASE_WHITE,
  FX_MODE_CHASE_COLOR,
  FX_MODE_CHASE_RANDOM,
  FX_MODE_CHASE_RAINBOW,
  FX_MODE_CHASE_FLASH,
  FX_MODE_CHASE_FLASH_RANDOM,
  FX_MODE_CHASE_RAINBOW_WHITE,
  FX_MODE_CHASE_BLACKOUT,
  FX_MODE_CHASE_BLACKOUT_RAINBOW,
  FX_MODE_COLOR_SWEEP_RANDOM,
  FX_MODE_RUNNING_COLOR,
  FX_MODE_RUNNING_RED_BLUE,
  FX_MODE_RUNNING_RANDOM,
  FX_MODE_LARSON_SCANNER,
  FX_MODE_COMET,
  FX_MODE_FIREWORKS,
  FX_MODE_FIREWORKS_RANDOM,
  FX_MODE_MERRY_CHRISTMAS,
  FX_MODE_FIRE_FLICKER,
  FX_MODE_FIRE_FLICKER_SOFT,
  FX_MODE_FIRE_FLICKER_INTENSE,
  FX_MODE_CIRCUS_COMBUSTUS,
  FX_MODE_HALLOWEEN,
  FX_MODE_BICOLOR_CHASE,
  FX_MODE_TRICOLOR_CHASE,
  FX_MODE_ICU,
};


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

  // Led setup
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(200);
  // ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  // ws2812fx.start();
}


void loop() { 
  ws2812fx.service();
  xboxController.onLoop();

  if (xboxController.isConnected()) {
    if (xboxController.isWaitingForFirstNotification()) {
      Serial.println("waiting for first notification");
    } else {

      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy;

      //Serial.println("Address: " + xboxController.buildDeviceAddressStr());      
      unsigned long receivedAt = xboxController.getReceiveNotificationAt();
      // Serial.print("joyLHori rate: ");
      // Serial.println((float)xboxController.xboxNotif.joyLHori / joystickMax);
      // Serial.print("joyLVert rate: ");
      // Serial.println((float)xboxController.xboxNotif.joyLVert / joystickMax);
     //  Serial.println("battery " + String(xboxController.battery) + "%");
      // Serial.println("received at " + String(receivedAt));      
      // Serial.print((int)xboxController.xboxNotif.joyRVert / 100);


      // Info Debug all values
      // Serial.print(xboxController.xboxNotif.toString());


      int controllerRVSpeed = (int)xboxController.xboxNotif.joyRVert / 100;
      // int controllerRHSpeed = (int)xboxController.xboxNotif.joyRHori / 100;
      
      // Buttons
      int btnRB = (int)xboxController.xboxNotif.btnRB;
      int btnLB = (int)xboxController.xboxNotif.btnLB;     
      int btnB = (int)xboxController.xboxNotif.btnB; 
      int btnX = (int)xboxController.xboxNotif.btnX;

      if(1 == btnB && 0 == lastButtonStateB){
        ledStatus = !ledStatus;
        if(ledStatus){
          ws2812fx.start();
        }else{          
          ws2812fx.stop();
        }
      }     

      if(1 == btnX && 0 == lastButtonStateX){
        ws2812fx.setMode(ledEffects[0]); 
      }

     
      if( 1 == btnRB && 0 == lastButtonStateRB && ledStatus ){      
          
          currentLedEffect++;
          if(currentLedEffect > NUM_EFFECTS - 1){
            currentLedEffect = 0;
          }          
          ws2812fx.setMode(ledEffects[currentLedEffect]);   
             
      }

      
      if( 1 == btnLB && 1 != btnRB && currentLedEffect > 0 && 0 == lastButtonStateLB && ledStatus ){              
          currentLedEffect--;
          ws2812fx.setMode(ledEffects[currentLedEffect]);      
      }

      lastButtonStateRB = btnRB; 
      lastButtonStateLB = btnLB;  
      lastButtonStateB = btnB;
      lastButtonStateX = btnX;

      Serial.print("Effect:" + String(currentLedEffect));
      Serial.print(" BTN RB:" + String(btnRB));
      Serial.print(" BTN RB Last:" + String(lastButtonStateRB));
      Serial.print(" BTN LB:" + String(btnLB));
      Serial.print(" BTN LB Last:" + String(lastButtonStateLB));
      Serial.println("");

      int speed = calculateSpeed(controllerRVSpeed);
      // int speedTurn = calculateSpeed(controllerRHSpeed);

      int directionMotor1 = 1;
      int directionMotor2 = 1;

      // Use of the drive function which takes as arguements the speed
      // and optional duration.  A negative speed will cause it to go
      // backwards.  Speed can be from -255 to 255.  Also use of the 
      // brake function which takes no arguements.
      // ex motor2.drive(255.1000);       
        // Backward
        if(!inRange(320, 655, controllerRVSpeed) )
        {
          directionMotor1 = 1;
          directionMotor2 = 1;
        }else{
          directionMotor1 = -1;
          directionMotor2 = -1;
        }

      motor1.drive(speed * directionMotor1);  
      motor2.drive(speed * directionMotor2);       

      // Servo
      int val1 = (float)xboxController.xboxNotif.joyLHori / 100;
      int servoVal = map(val1, 655, 0, 0, 60);  // align pot pointer to servo arm 
      myservo.write(servoPin1, servoVal);      // set the servo position (degrees)
    }    
  } else {
    Serial.println("not connected");
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
  
  delay(10);
}