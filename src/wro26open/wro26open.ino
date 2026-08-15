#include <EVN.h>


#define START 0
#define FIRSTWALL 1
#define TURNLEFT 2
#define TURNRIGHT 3
#define WALLTRACK 4
#define STOP 5
#define PRINT 6
#define FINDWALL 7
#define LASTWALL 8
#define OVERSHOOT 9
#define BASICOVERSHOOT 12

//to change
#define WALLTRACKDIST 13
#define FRONTTHRES 50
#define CORNERDETECT 40
#define WALLFOUND 40
#define BASICOVERSHOOTDIST 100
#define OVERSHOOTDIST 150
#define TURNTOLERANCE 3
#define DISTTOLERANCE 5



float leftAngle;
float rightAngle;
float zeroPosition;
float leftstartDist;
float rightstartDist;
float frontStartDist;
float backStartDist;
float startDist;
unsigned long long initialTime;
volatile const float gearRatio = (((40.0 / 24.0) / 1.4) / 1.67);

int var = START;

bool cw = true;
bool startLeft = true;
bool lastturn = false;
int corner = 0;
int frontDistCounter = 0;


extern volatile float stuffYaw;
extern volatile bool stuffValid;
extern volatile int leftDist;
extern volatile int checkRightDist;
extern volatile int rightDist;
extern volatile int frontDist;
extern volatile int backDist;
extern volatile int snappedHeading;
extern volatile bool lefterror;
extern volatile bool righterror;
extern volatile bool fronterror;
extern volatile bool backerror;

volatile int pulseCount;  // Rotation step count
int SIG_A = 0;            // Pin A output
int SIG_B = 0;            // Pin B output
int lastSIG_A = 0;        // Last state of SIG_A
int lastSIG_B = 0;        // Last state of SIG_B
const int Pin_A = 14;     // Interrupt pin (digital) for A (change your pins here)
const int Pin_B = 15;     // Interrupt pin (digital) for B

void A_CHANGE() {                                 // Interrupt Service Routine (ISR)
  detachInterrupt(digitalPinToInterrupt(Pin_A));  // Important
  SIG_A = digitalRead(Pin_A);                     // Read state of A
  SIG_B = digitalRead(Pin_B);                     // Read state of B

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
volatile int initialpulseCountback;
int SIG_Aback = 0;         // Pin A output
int SIG_Bback = 0;         // Pin B output
int lastSIG_Aback = 0;     // Last state of SIG_A
int lastSIG_Bback = 0;     // Last state of SIG_B
const int Pin_Aback = 19;  // Interrupt pin (digital) for A (change your pins here)
const int Pin_Bback = 18;  // Interrupt pin (digital) for B

void A_CHANGEBACK() {                                 // Interrupt Service Routine (ISR)
  detachInterrupt(digitalPinToInterrupt(Pin_Aback));  // Important
  SIG_Aback = digitalRead(Pin_Aback);                 // Read state of A
  SIG_Bback = digitalRead(Pin_Bback);                 // Read state of B

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

  SIG_Bback = digitalRead(Pin_Bback);  // Current state of B
  SIG_Aback = SIG_Bback > 0 ? 0 : 1;   // Let them be different
  // Attach iterrupt for state change, not rising or falling edges
  attachInterrupt(digitalPinToInterrupt(Pin_Aback), A_CHANGEBACK, CHANGE);

  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);  // M1

  SIG_B = digitalRead(Pin_B);  // Current state of B
  SIG_A = SIG_B > 0 ? 0 : 1;   // Let them be different
  // Attach iterrupt for state change, not rising or falling edges
  attachInterrupt(digitalPinToInterrupt(Pin_A), A_CHANGE, CHANGE);

  pinMode(23, OUTPUT);
  pinMode(22, OUTPUT);  //M3
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

  Serial.println(gearRatio);

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

  if (leftDist > 900 || leftDist == 0 || lefterror ) {
    Serial.println("trackheading");
    /*
    //Serial.print(leftDist);
    //Serial.print("\t");
    Serial.println(lefterror);
    runPosition(zeroPosition);
    digitalWrite(28, HIGH);
    digitalWrite(29, HIGH);
    */

    trackHeading(baseAngle);

  } else {
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
    float leftError = distance - leftDist; //swapped
    float targetAngle = constrain(leftError * 5, -22, 22) + baseAngle;
    Serial.print(stuffYaw);
    Serial.print("\t");
    Serial.print(leftError);
    Serial.print("\t");
    Serial.println(targetAngle);
    runPosition(zeroPosition - getError(targetAngle));
    //Serial.print(leftError);
  }


  //Serial.print("\t");
  //Serial.println(getError(targetAngle));
}

void rightWallTrack(int distance, int baseAngle) {
  Serial.println("rightwalltrack");

  if (rightDist > 900 || rightDist == 0 || righterror) {
    trackHeading(baseAngle);

  } else {
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
    float rightError = rightDist - distance;
    float targetAngle = constrain(rightError * 5, -22, 22) + baseAngle;
    Serial.print(stuffYaw);
    Serial.print("\t");
    Serial.print(rightError);
    Serial.print("\t");
    Serial.println(targetAngle);
    runPosition(zeroPosition - getError(targetAngle));
  }
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

      if (leftDist < rightDist && !lefterror && !righterror) {

        startLeft = true;
        startDist = leftDist;


      } else {
        startLeft = false;
        startDist = rightDist;
      }

      corner = 0;
      frontStartDist = frontDist;
      backStartDist = backDist;
      leftstartDist = leftDist * 10;
      rightstartDist = rightDist * 10;

      var = FIRSTWALL;

      break;

    case FIRSTWALL:
      //Serial.print("FIRSTWALL \t");
      //Serial.println(pulseCountback);

      if (startLeft) {
        leftWallTrack(startDist, 0);
      } else {
        rightWallTrack(startDist, 0);
      }


      if (frontDist < FRONTTHRES) {
        if (leftDist > CORNERDETECT) {
          Serial.println("left");
          cw = false;
          initialpulseCountback = pulseCountback;
          var = BASICOVERSHOOT;

          /*
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);
          */

        } else if (rightDist > CORNERDETECT) {
          Serial.println("right");
          cw = true;
          initialpulseCountback = pulseCountback;
          var = BASICOVERSHOOT;

          /*
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);
          */
        }
      }

      break;

    case BASICOVERSHOOT:

      if (cw) {
        trackHeading(turningAngle());

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= BASICOVERSHOOTDIST) {
          corner++;
          
          var = TURNRIGHT;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= BASICOVERSHOOTDIST) {
          corner++;

          var = TURNLEFT;
        }
      }

      break;
      

    case TURNLEFT:
      frontDistCounter = 0;
      Serial.println("TURNLEFT");
      Serial.println(stuffYaw);
      Serial.println(-(turningAngle()));

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle);


      if (abs(stuffYaw + turningAngle()) < TURNTOLERANCE) {

        if (lastturn) {
          var = LASTWALL;

        } else {
          initialTime = millis();
          var = FINDWALL;
        }

        /*
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        delay(1000);
        */
      }


      break;

    case TURNRIGHT:
      frontDistCounter = 0;
      Serial.println("TURNRIGHT");
      Serial.println(stuffYaw);
      Serial.println(turningAngle());

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);


      if (abs(stuffYaw - turningAngle()) < TURNTOLERANCE) {

        if (lastturn) {
          var = LASTWALL;

        } else {
          initialTime = millis();
          var = FINDWALL;
        }

        /*
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        delay(1000);
        */
      }


      break;

    case FINDWALL:
      Serial.println("FINDWALL");

      if (cw) {
        trackHeading(turningAngle());

        if (rightDist < WALLFOUND && millis() - initialTime >= 500) {

          var = WALLTRACK;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (leftDist < WALLFOUND && millis() - initialTime >= 500) {

          var = WALLTRACK;
        }
      }

      break;

    case WALLTRACK:
      Serial.println("WALLTRACK");

      if (cw) {
        //trackHeading(turningAngle());
        rightWallTrack(WALLTRACKDIST, turningAngle());
        if (frontDist < FRONTTHRES) {
          frontDistCounter++;
        }

        if (frontDistCounter > 50 && rightDist > CORNERDETECT) {

          if (corner == 11) {  //11
            Serial.println("hello");
            initialpulseCountback = pulseCountback;

            var = OVERSHOOT;

          } else {
            initialpulseCountback = pulseCountback;
            var = BASICOVERSHOOT;
          }

          /*
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);
          */
        }

      } else {
        leftWallTrack(WALLTRACKDIST, -(turningAngle()));
        if (frontDist < FRONTTHRES) {
          frontDistCounter++;
        }

        if (frontDistCounter > 50 && leftDist > CORNERDETECT) {

          if (corner == 11) {  //11
            Serial.println("hello");
            initialpulseCountback = pulseCountback;

            var = OVERSHOOT;
          } else {
            initialpulseCountback = pulseCountback;
            var = BASICOVERSHOOT;
          }

          /*
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);
          */
        }
      }

      break;

    case OVERSHOOT:
      Serial.println("OVERSHOOT");

      Serial.print(abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))));
      Serial.print("\t");
      Serial.println(rightstartDist - 10);

      
      if (cw) {
        trackHeading(turningAngle());

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= (rightstartDist - OVERSHOOTDIST)) {

          lastturn = true;
          corner++;
          /*
          runPosition(zeroPosition);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          */
          var = TURNRIGHT;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= (leftstartDist - OVERSHOOTDIST)) {

          lastturn = true;
          corner++;
          /*
          runPosition(zeroPosition);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          */
          var = TURNLEFT;
        }
      }

      break;

    case LASTWALL:
      Serial.println("LASTWALL");
      Serial.println(snappedHeading);

      if (startLeft) {
        leftWallTrack(startDist, 0);
      } else {
        rightWallTrack(startDist, 0);
      }

      if (frontDist <= frontStartDist + DISTTOLERANCE && backDist >= backStartDist - DISTTOLERANCE) { //CHANGE TOLERANCE
        
        var = STOP;
        /*
        if (cw) {
          if (abs(snappedHeading - turningAngle()) <= 5) {
            Serial.println("yay");
            var = STOP;
          }

        } else {
          if (abs(snappedHeading + turningAngle()) <= 5) {
            Serial.println("yay");
            var = STOP;
          }
        }
        */
      }

      break;


    case STOP:

      digitalWrite(28, LOW);
      digitalWrite(29, LOW);
      runPosition(zeroPosition);
      break;
  }
}
