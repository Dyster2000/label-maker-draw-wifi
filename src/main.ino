/*
This file is part of LabelMakerDrawBluetooth.

LabelMakerDrawBluetooth is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 or later.

LabelMakerDrawBluetooth is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LabelMakerDrawBluetooth. If not, see <https://www.gnu.org/licenses/>.
*/

//////////////////////////////////////////////////
//  LIBRARIES  //
//////////////////////////////////////////////////
#include "WebServer.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <ezButton.h>
#include <ESP32Servo.h>

// WiFi Credentials
const char *ssid = "netid";        // Your WiFi SSID
const char *password = "pwd";  // Your WiFi Password

//////////////////////////////////////////////////
//  PINS AND PARAMETERS  //
//////////////////////////////////////////////////

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 display

#define INIT_MSG           "Initializing... "  // Text to display on startup
#define APP_NAME           " BT LABELMAKER  "  // App name
#define IP_ADDR_BLANK      "                "
#define WAITING_FOR_IMAGE  "Wait for image  "
#define PRINTING_IMAGE     "Printing...     "

// printing variables
float x_scale = 14;      // Scale from 800x100 input to 960 steps for tape size
float y_scale = 14 * 2.8;
int space = 15;

// Stepper motor parameters
const int stepCount = 200;
const int stepsPerRevolution = 2048;

// initialize the stepper library for both steppers:
Stepper xStepper(stepsPerRevolution, 6, 8, 7, 9);
Stepper yStepper(stepsPerRevolution, 2, 4, 3, 5);

int xPins[4] = {6, 8, 7, 9}; // pins for x-motor coils
int yPins[4] = {2, 4, 3, 5}; // pins for y-motor coils

// Servo
const int SERVO_PIN = 13;
Servo servo;
int angle = 30;

enum State
{
  IpAddrDisplay,
  WaitingForInput,
  Printing
};
const char *StateText[] = { IP_ADDR_BLANK, WAITING_FOR_IMAGE, PRINTING_IMAGE };
State currentState = IpAddrDisplay;
State prevState = Printing; // Set different to trigger initial display

bool pPenOnPaper = false; // pen on paper in previous cycle

int xpos = 0;
int ypos = 0;

WebServer server(ssid, password);

/////////////////
//  S E T U P  //
/////////////////
void setup()
{
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print(INIT_MSG); // print start up message

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Startup");

  servo.attach(SERVO_PIN); // attaches the servo on pin 9 to the servo object
  servo.write(angle);

  plot(false); // servo to tape surface so pen can be inserted

  // set the speed of the motors
  yStepper.setSpeed(15); // set first stepper speed (these should stay the same)
  xStepper.setSpeed(15); // set second stepper speed (^ weird stuff happens when you push it too fast)

  penUp();     // ensure that the servo is lifting the pen carriage away from the tape
  homeYAxis(); // lower the Y axis all the way to the bottom

  releaseMotors();
  lcd.clear();

  // Setup BLE (bluetooth low energy)
  server.Init();
  Serial.println("Waiting for input from webserver");
}

////////////////////////////////////////////////
//  L O O P  //
////////////////////////////////////////////////
void loop()
{
  if (prevState != currentState)
  {
    updateScreen(StateText[currentState]);
    prevState = currentState;
    if (currentState == IpAddrDisplay)
    {
      // Display the IP address on screen
      lcd.setCursor(0, 1);
      lcd.print(server.GetWifiAddress());
    }
  }

  server.Loop();

  switch (currentState)
  {
    case IpAddrDisplay:
      if (server.ClientConnected())
        currentState = WaitingForInput;
      break;

    case WaitingForInput:
      if (server.HasReceivedImage())
      {
        Serial.println("State: WaitingForInput -> Printing");
        currentState = Printing;
      }
      break;

    case Printing:
    {
      // plot image
      size_t imagePointCount = server.GetImagePointCount();
      int highestX = 0;
      xpos = 0;
      ypos = 0;
      DrawPoint lastPoint;
      for (size_t i = 0; i < imagePointCount; i++)
      {
        auto nextPoint = server.GetPoint(i);
        lastPoint = nextPoint;

        if (nextPoint.X > highestX)
          highestX = nextPoint.X;

        line(nextPoint.X * x_scale, nextPoint.Y * y_scale, nextPoint.Draw);
      }

      line((highestX + space) * x_scale, 0, 0); // move to after image

      yStepper.step(-2250);
      releaseMotors();
      Serial.println("State: Printing -> WaitingForInput");
      currentState = WaitingForInput;
      server.ClearImage();
      break;
    }
  }
}

void line(int newx, int newy, bool drawing)
{
  // this function is an implementation of bresenhams line algorithm
  // this algorithm basically finds the slope between any two points, allowing us to figure out how many steps each motor should do to move smoothly to the target
  // in order to do this, we give this function our next X (newx) and Y (newy) coordinates, and whether the pen should be up or down (drawing)

  plot(drawing); // 0 = don't draw / 1 = draw

  int i;
  long over = 0;

  long dx = newx - xpos; // calculate the difference between where we are (xpos) and where we want to be (newx)
  long dy = newy - ypos;
  int dirx = dx > 0 ? -1 : 1; // this is called a ternary operator, it's basically saying: if dx is greater than 0, then dirx = -1, if dx is less than or equal to 0, dirx = 1.
  int diry = dy > 0 ? 1 : -1; // this is called a ternary operator, it's basically saying: if dy is greater than 0, then diry = 1, if dy is less than or equal to 0, diry = -1.
  // the reason one of these ^ is inverted logic (1/-1) is due to the direction these motors rotate in the system.

  dx = abs(dx); // normalize the dx/dy values so that they are positive.
  dy = abs(dy); // abs() is taking the "absolute value" - basically it removes the negative sign from negative numbers

  // the following nested If statements check which change is greater, and use that to determine which coordinate (x or y) get's treated as the rise or the run in the slope calculation
  // we have to do this because technically bresenhams only works for the positive quandrant of the cartesian coordinate grid,
  //  so we are just flipping the values around to get the line moving in the correct direction relative to it's current position (instead of just up an to the right)
  if (dx > dy)
  {
    over = dx / 2;
    for (i = 0; i < dx; i++)
    {                      // for however much our current position differs from the target,
      xStepper.step(dirx); // do a step in that direction (remember, dirx is always going to be either 1 or -1 from the ternary operator above)

      over += dy;
      if (over >= dx)
      {
        over -= dx;

        yStepper.step(diry);
      }
    }
  }
  else
  {
    over = dy / 2;
    for (i = 0; i < dy; i++)
    {
      yStepper.step(diry);
      over += dx;
      if (over >= dy)
      {
        over -= dy;
        xStepper.step(dirx);
      }
    }
  }
  xpos = newx; // store positions
  ypos = newy; // store positions
}

void plot(bool penOnPaper)
{ // used to handle lifting or lowering the pen on to the tape
  if (penOnPaper)
  { // if the pen is already up, put it down
    angle = 80;
  }
  else
  { // if down, then lift up.
    angle = 25;
  }
  servo.write(angle); // actuate the servo to either position.
  if (penOnPaper != pPenOnPaper)
    delay(50);              // gives the servo time to move before jumping into the next action
  pPenOnPaper = penOnPaper; // store the previous state.
}

void penUp()
{ // singular command to lift the pen up
  servo.write(25);
}

void penDown()
{ // singular command to put the pen down
  servo.write(80);
}

void releaseMotors()
{
  for (int i = 0; i < 4; i++)
  {                            // deactivates all the motor coils
    digitalWrite(xPins[i], 0); // just picks each motor pin and send 0 voltage
    digitalWrite(yPins[i], 0);
  }
  plot(false);
}

void homeYAxis()
{
  yStepper.step(-3000); // lowers the pen holder to it's lowest position.
}

void updateScreen(const char *text)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(APP_NAME);
  lcd.setCursor(0, 1);
  lcd.print(text);
}
