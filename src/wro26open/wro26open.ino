#include <EVN.h>


#define START 0
#define FIRSTWALL 1
#define TURNLEFT 2
#define TURNRIGHT 3
#define WALLTRACK 4
#define STOP 5
#define PRINT 6
#define FINDWALL 7

float leftAngle;
float rightAngle;
float zeroPosition;
float startDist;

int var = START;

bool cw = true;
bool startLeft = true;
int corner = 0;


extern volatile float stuffYaw;
extern volatile bool stuffValid;
extern volatile int leftDist;
extern volatile int rightDist;
extern volatile int frontDist;
extern volatile int backDist;
extern volatile bool leftCheck;
extern volatile bool rightCheck;
extern volatile bool frontCheck;
extern volatile bool backCheck;

volatile int pulseCount;  // Rotation step count
int SIG_A = 0;            // Pin A output
int SIG_B = 0;            // Pin B output
int lastSIG_A = 0;        // Last state of SIG_A
int lastSIG_B = 0;        // Last state of SIG_B
const int Pin_A = 14;     // Interrupt pin (digital) for A (change your pins here)
const int Pin_B = 15;     // Interrupt pin (digital) for B

void A_CHANGE() {              // Interrupt Service Routine (ISR)
  detachInterrupt(0);          // Important
  SIG_A = digitalRead(Pin_A);  // Read state of A
  SIG_B = digitalRead(Pin_B);  // Read state of B

  if ((SIG_B == SIG_A) && (lastSIG_B != SIG_B)) {
    pulseCount--;  // Counter-clockwise rotation
    lastSIG_B = SIG_B;
  }

  else if ((SIG_B != SIG_A) && (lastSIG_B == SIG_B)) {
    pulseCount++;                   // Clockwise rotation
    lastSIG_B = SIG_B > 0 ? 0 : 1;  // Save last state of B
  }
  attachInterrupt(digitalPinToInterrupt(Pin_A), A_CHANGE, CHANGE);
}


volatile int pulseCountback;  // Rotation step count
int SIG_Aback = 0;            // Pin A output
int SIG_Bback = 0;            // Pin B output
int lastSIG_Aback = 0;        // Last state of SIG_A
int lastSIG_Bback = 0;        // Last state of SIG_B
const int Pin_Aback = 18;     // Interrupt pin (digital) for A (change your pins here)
const int Pin_Bback = 19;     // Interrupt pin (digital) for B

void A_CHANGEBACK() {                  // Interrupt Service Routine (ISR)
  detachInterrupt(0);                  // Important
  SIG_Aback = digitalRead(Pin_Aback);  // Read state of A
  SIG_Bback = digitalRead(Pin_Bback);  // Read state of B

  if ((SIG_Bback == SIG_Aback) && (lastSIG_Bback != SIG_Bback)) {
    pulseCountback--;  // Counter-clockwise rotation
    lastSIG_Bback = SIG_Bback;
  }

  else if ((SIG_Bback != SIG_Aback) && (lastSIG_Bback == SIG_Bback)) {
    pulseCountback++;                       // Clockwise rotation
    lastSIG_Bback = SIG_Bback > 0 ? 0 : 1;  // Save last state of B
  }
  attachInterrupt(digitalPinToInterrupt(Pin_Aback), A_CHANGEBACK, CHANGE);
}


void runPosition(int targetPosition) {
  targetPosition = constrain(targetPosition, rightAngle - 5, leftAngle + 5);
  int positionError = pulseCount - targetPosition;  //(headingError * 5.0) - (steer.getPosition() - STEER_CENTER);
  //Serial.println(positionError);
  int pwmRun = constrain(positionError * 22, -255, 255);
  //Serial.println(pwmRun);
  if (positionError > 0) {
    // turn cw
    digitalWrite(23, HIGH);
    analogWrite(22, 255 - pwmRun);
  } else {
    //turn ccw
    digitalWrite(22, HIGH);
    analogWrite(23, 255 + pwmRun);
  }
}

volatile bool core0_ready = false;
volatile bool core1_ready = false;
int steerRightBound = 0;
int steerLeftBound = 0;

int steerOffset = 5;

void setup() {

  SIG_B = digitalRead(Pin_B);  // Current state of B
  SIG_A = SIG_B > 0 ? 0 : 1;   // Let them be different
  // Attach iterrupt for state change, not rising or falling edges
  attachInterrupt(digitalPinToInterrupt(Pin_A), A_CHANGE, CHANGE);

  pinMode(23, OUTPUT);
  pinMode(22, OUTPUT); //M3
  digitalWrite(22, LOW);
  digitalWrite(23, HIGH);
  delay(1000);
  int currPosition = 0;
  int prevPosition = 999;
  while (true) {
    currPosition = pulseCount;
    if (currPosition == prevPosition) {
      break;
    }
    prevPosition = currPosition;
  }
  steerRightBound = pulseCount;
  digitalWrite(23, LOW);


  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(24, INPUT_PULLUP);

  SIG_Bback = digitalRead(Pin_Bback);  // Current state of B
  SIG_Aback = SIG_Bback > 0 ? 0 : 1;   // Let them be different
  // Attach iterrupt for state change, not rising or falling edges
  attachInterrupt(digitalPinToInterrupt(Pin_Aback), A_CHANGEBACK, CHANGE);

  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);  // M1

  digitalWrite(20, HIGH);
  digitalWrite(21, HIGH); //M4

  digitalWrite(LED_BUILTIN, HIGH);

  unsigned long long initialTime = millis();

  while (millis() - initialTime <= 1000) {
    digitalWrite(22, HIGH);
    analogWrite(23, 0);
  }
  leftAngle = pulseCount;

  while (millis() - initialTime <= 2000) {
    digitalWrite(23, HIGH);
    analogWrite(22, 0);
  }
  rightAngle = pulseCount;


  //delay(1000);

  Serial.print(leftAngle);
  Serial.print("\t");
  Serial.println(rightAngle);
  Serial.println((leftAngle + rightAngle) / 2);
  zeroPosition = ((leftAngle + rightAngle) / 2) - steerOffset;

  while (!digitalReadFast(24)) runPosition(zeroPosition);
  while (digitalReadFast(24)) runPosition(zeroPosition);

  Serial.println("Starting");
  Serial.println(stuffYaw);

  core0_ready = true;
  
  while (!core1_ready)
    ;
}

float getError(float angle) {

  //float targetAngle = initialAngle + angle;

  while (angle >= 180) {
    angle = angle - 360;
  }

  while (angle <= -180) {
    angle = 360 + angle;
  }

  float error = angle - stuffYaw;
  while (error >= 180) {
    error = error - 360;
  }

  while (error <= -180) {
    error = 360 + error;
  }

  //Serial.println(error);

  return error;
}

int turningAngle() {
  int angle = corner * 90;
  while (angle >= 180) {
    angle = angle - 360;
  }

  while (angle <= -180) {
    angle = 360 + angle;
  } 



  return angle;
}


void leftWallTrack(int distance, int baseAngle) {

  if (leftDist > 900 || rightDist > 900 || leftDist == 0 || rightDist == 0) {
    trackHeading(baseAngle);

  } else {
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
  }

  float leftError = leftDist - distance;
  float targetAngle = constrain(leftError * 5, -22, 22) + baseAngle;
  runPosition(zeroPosition + getError(targetAngle));
}

void rightWallTrack(int distance, int baseAngle) {
  
  if (leftDist > 900 || rightDist > 900 || leftDist == 0 || rightDist == 0) {
    trackHeading(baseAngle);

  } else {
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
  }

  float rightError = rightDist - distance;
  float targetAngle = constrain(rightError * 5, -22, 22) + baseAngle;
  runPosition(zeroPosition - getError(targetAngle));
}

void trackHeading(int angle) {
  digitalWrite(28, LOW);
  digitalWrite(29, HIGH);
  float angleError = getError(angle);
  Serial.println(angleError);
  runPosition(zeroPosition - angleError);
}

void loop() {
  switch (var) {
    case PRINT:

      break;

    case START:
      while (!stuffValid) {
      }
      Serial.println("START");

      if (leftDist < rightDist) {
        startLeft = true;
        startDist = leftDist;

      } else {
        startLeft = false;
        startDist = rightDist;
      }
      corner = 0;

      var = FIRSTWALL;
      
      break;

    case FIRSTWALL:
      Serial.println("FIRSTWALL");

      if (startLeft) {
        leftWallTrack(13, 0);
      } else {
        rightWallTrack(13, 0);
      }
      Serial.print(leftDist);
      Serial.print("\t");
      Serial.print(rightDist);
      Serial.print("\t");
      Serial.println(frontDist);


      if (frontDist < 50) {
        if (leftDist > 40) {
          Serial.println("left");
          cw = false;
          corner++;
          var = TURNLEFT;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);

        } else if (rightDist > 40) {
          Serial.println("right");
          cw = true;
          corner++;
          var = TURNRIGHT;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);

        }
      }

      break;

    case TURNLEFT:
      Serial.println("TURNLEFT");

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle);


      if (abs(stuffYaw + turningAngle()) < 3) {

        var = FINDWALL;
        digitalWrite(28, LOW);
          digitalWrite(29, LOW);
         delay(1000);
      }


      break;

    case TURNRIGHT:
      Serial.println("TURNRIGHT");
      Serial.println(stuffYaw);
      Serial.println(turningAngle());

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);


      if (abs(stuffYaw - turningAngle()) < 3) {
 
        var = FINDWALL;
        digitalWrite(28, LOW);
          digitalWrite(29, LOW);
        delay(1000);
      }


      break;

    case FINDWALL:
      Serial.println("FINDWALL");

      if (cw) {
        trackHeading(turningAngle());

        if (rightDist < 40) {
          var = WALLTRACK;
        } 
      } else {
        trackHeading(-(turningAngle()));

        if (leftDist < 40) {
          var = WALLTRACK;
        }
      }

      break;

    case WALLTRACK:
      Serial.println("WALLTRACK");

      if (corner == 12) {
        Serial.println("hello");
        var = STOP;

      } else if (cw) {
        rightWallTrack(13, turningAngle());
        if (frontDist < 50 && rightDist > 40) {
          corner++;
          var = TURNRIGHT;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
           delay(1000);
        }

      } else {
        leftWallTrack(13, -(turningAngle()));
        if (frontDist < 50 && leftDist > 40) {
          corner++;
          var = TURNLEFT;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
           delay(1000);
        }

      }

      break;

    case STOP:

      digitalWrite(28, LOW);
      digitalWrite(29, LOW);
      runPosition(zeroPosition);
      break;

  }
}

void loopy() {


  switch (var) {

    case PRINT:
      Serial.print(leftDist);
      Serial.print("\t");
      Serial.print(rightDist);
      Serial.print("\t");
      Serial.print(frontDist);
      Serial.print("\t");
      Serial.println(backDist);

      Serial.println(stuffYaw);



      break;

    case START:
      while (!stuffValid);
      Serial.println("START");
      Serial.print(leftDist);
      Serial.print("\t");
      Serial.println(rightDist);


      if (leftDist < rightDist) {
        startLeft = true;
        startDist = leftDist;

      } else {
        startLeft = false;
        startDist = rightDist;
      }
      corner = 0;


      var = FIRSTWALL;

      break;

    case FIRSTWALL:

      if (startLeft) {
        Serial.println("LEFT");
        leftWallTrack(startDist, 0);
      } else {
        Serial.println("RIGHT");
        rightWallTrack(startDist, 0);
      }

      Serial.print(leftDist);
      Serial.print("\t");
      Serial.println(rightDist);

      //if (()) if both sides sum = invalid then track 0
      if (leftDist > 40 && frontDist < 25) {
        cw = false;

        //var = STOP;
        corner++;
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        //delay(5000);
          var = TURNLEFT;

      } else if (rightDist > 40 && frontDist < 25) {
        cw = true;
        //var = STOP;
        corner++;
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        //delay(5000);
          var = TURNRIGHT;
      }

      break;

    case TURNLEFT:
      Serial.println("LEFTTURN");
      //runPosition(leftAngle);

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle);
      Serial.println(leftAngle);


      if (stuffYaw <= -(turningAngle())) {
        //var = STOP;
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        //delay(5000);
          var = FINDWALL;
      }


      break;

    case TURNRIGHT:
      Serial.println("RIGHTTURN");
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);
      //zeroPosition + getError(-90));
      Serial.print("rightangle: ");
      Serial.println(rightAngle);
  
      Serial.println(stuffYaw);
      Serial.print("turning: ");
      Serial.println(turningAngle());

      if (stuffYaw >= turningAngle()) {
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        //delay(5000);
          var = FINDWALL;
        //var = STOP;
      }

      break;

    case FINDWALL:
      Serial.println("FINDWALL");
      Serial.println(turningAngle());
      Serial.println(rightDist);
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);

      if (cw) {
        trackHeading(turningAngle());

        if (rightDist <= 40) {
          //corner++;
          //var = STOP;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          //delay(5000);
          var = WALLTRACK;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (leftDist <= 40) {
          //var = STOP;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          //delay(5000);
            var = WALLTRACK;
        }
      }

      break;

    case WALLTRACK:

      Serial.println(corner);

      if (corner == 12) {
        var = STOP;
        Serial.println("hi");

      } else if (cw) {
        Serial.println("rightwalltrack");
        Serial.println(turningAngle());
        //Serial.println()
        rightWallTrack(13, turningAngle());

        if (rightDist > 40) {
          cw = true;
          corner++;
          //var = STOP;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          //delay(5000);
            var = TURNRIGHT;
        }

      } else {
        leftWallTrack(13, -(turningAngle()));

        if (leftDist > 40) {
          cw = false;
          corner++;
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          //delay(5000);
            //var = STOP;
            var = TURNLEFT;
        }
      }


      break;

    case STOP:
      digitalWrite(28, LOW);
      digitalWrite(29, LOW);
      runPosition(zeroPosition);
      break;
  }
}