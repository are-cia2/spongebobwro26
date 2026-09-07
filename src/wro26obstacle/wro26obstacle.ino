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
#define LEFTOBSTACLE 10
#define RIGHTOBSTACLE 11
#define BASICOVERSHOOT 12
#define REVERSEBACK 13
#define SMALLLEFTTURN 14
#define SMALLRIGHTTURN 15
#define REVERSETURN 16

//to change
#define WALLTRACKDIST 41
#define FRONTTHRES 30
#define BACKTHRES 100
#define CORNERDETECT 88
#define WALLFOUND 60
#define BASICOVERSHOOTDIST 150
#define OVERSHOOTDIST 150
#define TURNTOLERANCE 3
#define DISTTOLERANCE 5
#define WALLAFTOBS 25
#define BIGTURNANGLE 80
#define SMALLTURNANGLE 52

char yippee;

float leftAngle;
float rightAngle;
float zeroPosition;
float leftStartDist;
float rightStartDist;
float frontStartDist;
float backStartDist;
float startDist;
float oriWallDist;
float obstacleDist;
unsigned long long initialTime;
volatile const float gearRatio = (((40.0 / 24.0) / 1.4) / 1.67);
extern volatile bool rangeDataReady;
float leftWallDist;
float rightWallDist;
float lastReverse = -999;

int var = START;
int prevvar = WALLTRACK;
int leftobstaclevar = 0;
int rightobstaclevar = 0;

bool cw = true;
bool startLeft = true;
bool lastturn = false;
int corner = 0;
int frontDistCounter = 0;
int obstacleTurnAngle = 52;
int wallCounter = 0;
int leftCounter = 0;
int rightCounter = 0;


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

extern volatile bool leftGreen;
extern volatile bool rightGreen;
extern volatile bool leftRed;
extern volatile bool rightRed;

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
  Serial2.begin(115200);

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
  //Serial.println(stuffYaw);

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

int miniLeftAngle() {
  int angle = 0;
  if (cw) {
    angle = turningAngle() - obstacleTurnAngle;
  } else {
    angle = turningAngle() + obstacleTurnAngle;
  }

  while (angle >= 180) {
    angle = angle - 360;
  }

  while (angle <= -180) {
    angle = 360 + angle;
  }


  return angle;
}

int miniRightAngle() {
  int angle = 0;
  if (cw) {
    angle = turningAngle() + obstacleTurnAngle;
  } else {
    angle = turningAngle() - obstacleTurnAngle;
  }
  while (angle >= 180) {
    angle = angle - 360;
  }

  while (angle <= -180) {
    angle = 360 + angle;
  }


  return angle;
}


void leftWallTrack(int distance, int baseAngle) {


  if (leftDist > 900 || leftDist == 0 || lefterror) {
    Serial.println("trackheading");
    trackHeading(baseAngle);

  } else {
    if (righterror || fronterror || backerror) {
      analogWrite(28, 50);
      digitalWrite(29, HIGH);
    } else {
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
    }

    float leftError = distance - leftDist;
    float targetAngle = constrain(leftError * 5, -15, 15) + baseAngle;


    runPosition(zeroPosition - getError(targetAngle));
  }
}

void rightWallTrack(int distance, int baseAngle) {
  //Serial.println("rightwalltrack");

  if (rightDist > 900 || rightDist == 0 || righterror) {
    trackHeading(baseAngle);

  } else {
    if (lefterror || fronterror || backerror) {
      analogWrite(28, 50);
      digitalWrite(29, HIGH);
    } else {
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
    }
    digitalWrite(28, LOW);
    digitalWrite(29, HIGH);
    float rightError = rightDist - distance;
    float targetAngle = constrain(rightError * 5, -15, 15) + baseAngle;

    runPosition(zeroPosition - getError(targetAngle));
  }
}

void bothWallTrack(int baseAngle) {


  if (leftDist == 0 || lefterror || rightDist == 0 || righterror) {
    Serial.println("trackheading");
    trackHeading(baseAngle);

  } else {
    if (fronterror || backerror) {
      analogWrite(28, 50);
      digitalWrite(29, HIGH);
    } else {
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
    }
    float wallError = rightDist - leftDist;
    float targetAngle = constrain(wallError * 2, -15, 15) + baseAngle;

    runPosition(zeroPosition - getError(targetAngle));
  }
}



void trackHeading(int angle) {
  if (righterror || fronterror || backerror || lefterror) {
      analogWrite(28, 50);
      digitalWrite(29, HIGH);
    } else {
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
    }
  float angleError = getError(angle);
  //Serial.println(angleError);
  runPosition(zeroPosition - angleError);
}

void reverseHeading(int angle) {
  digitalWrite(28, HIGH);
  digitalWrite(29, LOW);
  float angleError = -(getError(angle));
  runPosition(zeroPosition - angleError);
}

void loop() {

  if (Serial2.available()) {
    char yippee = Serial2.read();
    Serial.println(yippee);

    if (yippee == 'a') {  //leftgreen
      leftGreen = true;
      rightGreen = false;
      leftRed = false;
      rightRed = false;

    } else if (yippee == 'b') {  //rightgreen
      leftGreen = false;
      rightGreen = true;
      leftRed = false;
      rightRed = false;

    } else if (yippee == 'c') {  //leftRed
      leftGreen = false;
      rightGreen = false;
      leftRed = true;
      rightRed = false;

    } else if (yippee == 'd') {  //rightRed
      leftGreen = false;
      rightGreen = false;
      leftRed = false;
      rightRed = true;
    }
  }

  //Serial.println(var);
  switch (var) {
    case PRINT:
      Serial.print(leftDist);
      Serial.print("\t");
      Serial.println(rightDist);
      break;

    case START:

      while (!stuffValid || !rangeDataReady) {
        runPosition(zeroPosition);
      }
      Serial.println("START");

      corner = 0;
      leftStartDist = leftDist;
      rightStartDist = rightDist;
      frontStartDist = frontDist;
      backStartDist = backDist;
      var = WALLTRACK;

      break;

    case WALLTRACK:

      if (leftGreen) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("leftgreen");

      } else if (rightGreen) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("rightgreen");

      } else if (leftRed) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("leftred");

      } else if (rightRed) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("rightred");
      }

      if (!lefterror && !righterror) {
        if (cw) {

          if (leftDist < CORNERDETECT && rightDist < CORNERDETECT) {
            //Serial.println("bothwall");
            bothWallTrack(turningAngle());

          } else if (leftDist < rightDist) {
            rightCounter++;
            //Serial.println("leftwall");
            leftWallTrack(WALLTRACKDIST, turningAngle());

          } else if (corner == 0) {
            leftCounter++;
            trackHeading(0);
          }

        } else {
          if (leftDist < CORNERDETECT && rightDist < CORNERDETECT) {
            //Serial.println("bothwall");
            bothWallTrack(-turningAngle());

          } else if (leftDist > rightDist) {
            leftCounter++;
            //Serial.println("rightwall");
            rightWallTrack(WALLTRACKDIST, -turningAngle());
          }
        }

      } else if (!righterror && rightDist < CORNERDETECT) {
        //Serial.println("rightvalid");
        if (cw) {
          rightWallTrack(WALLTRACKDIST, turningAngle());
        } else {
          rightWallTrack(WALLTRACKDIST, -turningAngle());
        }


      } else if (!lefterror && leftDist < CORNERDETECT) {
        //Serial.println("leftvalid");
        if (cw) {
          leftWallTrack(WALLTRACKDIST, turningAngle());
        } else {
          leftWallTrack(WALLTRACKDIST, -turningAngle());
        }

      } else {
        //Serial.println("nonevalid");
        if (cw) {
          trackHeading(turningAngle());
        } else {
          trackHeading(-turningAngle());
        }
      }

      if (frontDist < FRONTTHRES && !fronterror && backDist > BACKTHRES && !backerror && millis() - lastReverse >= 1000 && leftDist + rightDist > 100) {
        corner++;

        digitalWrite(28, HIGH);
        digitalWrite(29, HIGH);

        unsigned long long initial = millis();

        while (millis() - initial <= 250 || leftCounter + rightCounter < 5) {
          if (!lefterror && leftDist > CORNERDETECT) {
            leftCounter++;
          }

          if (!righterror && rightDist > CORNERDETECT) {
            rightCounter++;
          }
        }

        if (leftCounter > rightCounter) {
          cw = false;

        } else {
          cw = true;
        }
        var = REVERSETURN;
      }


      break;

    case REVERSETURN:
      leftCounter = 0;
      rightCounter = 0;
      Serial.println("REVERSETURN");

      digitalWrite(28, HIGH);
      digitalWrite(29, LOW);

      if (cw) {
        runPosition(leftAngle);

        if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {

          if (corner == 12) {
            var = LASTWALL;

          } else {
            digitalWrite(28, HIGH);
            digitalWrite(29, HIGH);
            delay(250);
            var = WALLTRACK;
          }
        }
      } else {
        runPosition(rightAngle);

        if (fabsf(getError(-turningAngle())) < TURNTOLERANCE) {
          if (corner == 12) {
            var = LASTWALL;

          } else {
            digitalWrite(28, HIGH);
            digitalWrite(29, HIGH);
            delay(250);

            var = WALLTRACK;
          }
        }
      }
      break;


    case LASTWALL:
      Serial.println("LASTWALL");

      if (cw) {
        leftWallTrack(leftStartDist, 0);
      } else {
        rightWallTrack(rightStartDist, 0);
      }

      if (frontDist <= frontStartDist + DISTTOLERANCE && backDist >= backStartDist - DISTTOLERANCE) {  //CHANGE TOLERANCE

        var = STOP;
      }

      break;

    case LEFTOBSTACLE:

      switch (leftobstaclevar) {

        case 0:  //turnleft
          Serial.println("left");
          Serial.print(stuffYaw);
          Serial.print("\t");
          Serial.println(-(miniLeftAngle()));

          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(leftAngle);

          if (cw) {
            if (abs(stuffYaw - miniLeftAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 1;
            }
          } else {
            if (abs(stuffYaw + miniLeftAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 1;
            }
          }

          break;

        case 1:  //turnright to front
          Serial.println("right");
          Serial.print(stuffYaw);
          Serial.print("\t");
          Serial.println(-(miniRightAngle()));

          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(rightAngle);

          if (cw) {
            if (abs(stuffYaw - turningAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 2;
              wallCounter = 0;
              initialTime = millis();
            }
          } else {
            if (abs(stuffYaw + turningAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 2;
              wallCounter = 0;
              initialTime = millis();
            }
          }

          break;

        case 2:  //find obstacle

          Serial.print(rightDist);
          Serial.print("\t");
          Serial.println(oriWallDist);
          Serial.println(wallCounter);

          trackHeading(turningAngle());

          if (rightDist < oriWallDist && !righterror) {
            wallCounter++;

            if (wallCounter >= 5) {
              obstacleDist = rightDist;
              wallCounter = 0;
              leftobstaclevar = 3;
            }
          }

          if (millis() - initialTime >= 250) {
            wallCounter = 0;
            leftobstaclevar = 4;
          }


          break;

        case 3:
          Serial.println(wallCounter);

          trackHeading(turningAngle());

          if (rightDist >= obstacleDist && !righterror) {
            wallCounter++;

            if (wallCounter >= 10) {
              wallCounter = 0;
              leftobstaclevar = 4;
            }
          }

          break;

        case 4:  //turn right
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(rightAngle);

          if (cw) {
            if (abs(stuffYaw - miniRightAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 5;
              wallCounter = 0;
            }
          } else {
            if (abs(stuffYaw + miniRightAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 5;
              wallCounter = 0;
            }
          }

          break;


        case 5:  // turnleft to front
          Serial.println("front");
          Serial.print(stuffYaw);
          Serial.print("\t");
          Serial.println(-(turningAngle()));

          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(leftAngle);

          if (cw) {
            if (abs(stuffYaw - turningAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 0;
              leftGreen = false;
              rightGreen = false;
              leftRed = false;
              rightRed = false;
              var = prevvar;
            }
          } else {
            if (abs(stuffYaw + turningAngle()) < TURNTOLERANCE) {
              leftobstaclevar = 0;
              leftGreen = false;
              rightGreen = false;
              leftRed = false;
              rightRed = false;
              var = prevvar;
            }
          }

          break;
      }

      break;

    case RIGHTOBSTACLE:

      switch (rightobstaclevar) {

        case 0:  //turnright
          Serial.println("right");
          /*
          Serial.print(stuffYaw);
          Serial.print("\t");
          Serial.println(miniRightAngle());
          */
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(rightAngle);

          if (cw) {
            if (abs(stuffYaw - miniRightAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 1;
            }
          } else {
            if (abs(stuffYaw + miniRightAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 1;
            }
          }

          break;


        case 1:  //turnleft
          Serial.println("left");

          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(leftAngle);

          if (cw) {
            if (abs(stuffYaw - turningAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 2;
              wallCounter = 0;
              initialTime = millis();
            }
          } else {
            if (abs(stuffYaw + turningAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 2;
              wallCounter = 0;
              initialTime = millis();
            }
          }

          break;

        case 2:
          Serial.print(leftDist);
          Serial.print("\t");
          Serial.println(oriWallDist);
          Serial.println(wallCounter);

          trackHeading(turningAngle());

          if (leftDist < oriWallDist && !lefterror) {
            wallCounter++;

            if (wallCounter >= 5) {
              obstacleDist = leftDist;
              wallCounter = 0;
              rightobstaclevar = 3;
            }
          }

          if (millis() - initialTime >= 500) {
            wallCounter = 0;
            rightobstaclevar = 4;
          }


          break;

        case 3:
          Serial.println(wallCounter);

          trackHeading(turningAngle());

          if (leftDist >= obstacleDist && !lefterror) {
            wallCounter++;

            if (wallCounter >= 10) {
              wallCounter = 0;
              rightobstaclevar = 4;
            }
          }

          break;

        case 4:  //turn left
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(leftAngle);

          if (cw) {
            if (abs(stuffYaw - miniLeftAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 5;
              wallCounter = 0;
            }
          } else {
            if (abs(stuffYaw + miniLeftAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 5;
              wallCounter = 0;
            }
          }

          break;

        case 5:  //turnright to front
          Serial.println("front");
          Serial.print(stuffYaw);
          Serial.print("\t");
          Serial.println(-(turningAngle()));

          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);

          runPosition(rightAngle);

          if (cw) {
            if (abs(stuffYaw - turningAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 0;
              leftGreen = false;
              rightGreen = false;
              leftRed = false;
              rightRed = false;
              var = prevvar;
            }
          } else {
            if (abs(stuffYaw + turningAngle()) < TURNTOLERANCE) {
              rightobstaclevar = 0;
              leftGreen = false;
              rightGreen = false;
              leftRed = false;
              rightRed = false;
              var = prevvar;
            }
          }

          break;
      }

      break;

    case STOP:
      digitalWrite(28, HIGH);
      digitalWrite(29, HIGH);
      runPosition(zeroPosition);
  }
}

void loopy() {
  /*
  Serial.println(leftGreen);
  Serial.println(rightGreen);
  Serial.println(leftRed);
  Serial.println(rightRed);
  */
  switch (var) {
    case PRINT:

      break;

    case START:
      while (!stuffValid || !rangeDataReady) {
      }
      Serial.println("START");

      if (leftDist < rightDist && !lefterror && !righterror) {

        startLeft = true;
        startDist = leftDist;


      } else if (!righterror) {
        startLeft = false;
        startDist = rightDist;
      }

      corner = 0;
      frontStartDist = frontDist;
      backStartDist = backDist;
      leftStartDist = leftDist * 10;
      rightStartDist = rightDist * 10;

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

      if (leftGreen) {
        prevvar = FIRSTWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("leftgreen");

      } else if (rightGreen) {
        prevvar = FIRSTWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("rightgreen");

      } else if (leftRed) {
        prevvar = FIRSTWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("leftred");

      } else if (rightRed) {
        prevvar = FIRSTWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("rightred");
      }

      if (frontDist < FRONTTHRES && !fronterror) {
        if (leftDist > CORNERDETECT && !lefterror) {
          Serial.println("left");

          cw = false;
          initialpulseCountback = pulseCountback;
          var = BASICOVERSHOOT;

          /*
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(1000);
          */

        } else if (rightDist > CORNERDETECT && !righterror) {
          Serial.println("right");
          cw = true;
          initialpulseCountback = pulseCountback;
          var = BASICOVERSHOOT;

          /*
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
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

          var = SMALLLEFTTURN;
          obstacleTurnAngle = 60;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= BASICOVERSHOOTDIST) {
          corner++;
          /*
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          delay(1000);
          */
          var = SMALLRIGHTTURN;
          obstacleTurnAngle = 60;
        }
      }

      break;

    case SMALLLEFTTURN:

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle);


      if (fabsf(getError(-miniLeftAngle())) < TURNTOLERANCE) {

        var = TURNRIGHT;

        /*
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        delay(1000);
        */
      }

      break;

    case SMALLRIGHTTURN:
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);
      Serial.println("SMALLRIGHTTURN");


      if (fabsf(getError(miniRightAngle())) < TURNTOLERANCE) {


        var = TURNLEFT;

        /*
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        delay(1000);
        */
      }

      break;

    case TURNLEFT:
      frontDistCounter = 0;
      Serial.println("TURNLEFT");
      //Serial.println(stuffYaw);
      Serial.println(-(turningAngle()));

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle);


      if (fabsf(getError(-turningAngle())) < TURNTOLERANCE) {

        initialTime = millis();

        var = FINDWALL;


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
      //Serial.println(stuffYaw);
      Serial.println(turningAngle());

      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);


      if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {


        initialTime = millis();
        var = FINDWALL;

        /*
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        delay(1000);
        */
      }


      break;

    case REVERSEBACK:

      if (cw) {
        reverseHeading(turningAngle());

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) <= 500) {
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(500);
          var = FINDWALL;
        }
      } else {
        reverseHeading(-(turningAngle()));

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) <= 500) {
          digitalWrite(28, LOW);
          digitalWrite(29, LOW);
          delay(500);
          var = FINDWALL;
        }
      }


      break;


    case FINDWALL:
      Serial.println("FINDWALL");

      if (cw) {
        trackHeading(turningAngle());

        if (!righterror && rightDist < WALLFOUND && millis() - initialTime >= 500) {
          if (lastturn) {
            var = LASTWALL;

          } else {
            initialTime = millis();
            var = WALLTRACK;
          }
        }
      } else {
        trackHeading(-(turningAngle()));

        if (!lefterror && leftDist < WALLFOUND && millis() - initialTime >= 500) {

          if (lastturn) {
            var = LASTWALL;

          } else {

            initialTime = millis();

            var = WALLTRACK;
          }
        }
      }

      if (leftGreen) {
        prevvar = FINDWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("leftgreen");

      } else if (rightGreen) {
        prevvar = FINDWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("rightgreen");

      } else if (leftRed) {
        prevvar = FINDWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("leftred");

      } else if (rightRed) {
        prevvar = FINDWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("rightred");
      }

      break;

    case LEFTOBSTACLE:

      switch (leftobstaclevar) {

        case 0:  // turn left
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(leftAngle);

          if (fabsf(getError(miniLeftAngle())) < TURNTOLERANCE) {
            leftobstaclevar = 1;
          }
          break;

        case 1:  // turn right to front
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(rightAngle);

          if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {
            leftobstaclevar = 2;
            wallCounter = 0;
            initialTime = millis();
          }
          break;

        case 2:  // find obstacle
          trackHeading(turningAngle());

          if (rightDist < oriWallDist && !righterror) {
            wallCounter++;
            if (wallCounter >= 5) {
              obstacleDist = rightDist;
              wallCounter = 0;
              leftobstaclevar = 3;
            }
          }

          if (millis() - initialTime >= 250) {
            wallCounter = 0;
            leftobstaclevar = 4;
          }
          break;

        case 3:  // wait until obstacle passed
          trackHeading(turningAngle());

          if (rightDist >= obstacleDist && !righterror) {
            wallCounter++;
            if (wallCounter >= 10) {
              wallCounter = 0;
              leftobstaclevar = 4;
            }
          }
          break;

        case 4:  // turn right
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(rightAngle);

          if (fabsf(getError(miniRightAngle())) < TURNTOLERANCE) {
            leftobstaclevar = 5;
            wallCounter = 0;
          }
          break;

        case 5:  // turn left back to front
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(leftAngle);

          if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {
            leftobstaclevar = 0;
            leftGreen = rightGreen = leftRed = rightRed = false;
            var = prevvar;
          }
          break;
      }

      break;


    case RIGHTOBSTACLE:

      switch (rightobstaclevar) {

        case 0:  // turn right
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(rightAngle);

          if (fabsf(getError(miniRightAngle())) < TURNTOLERANCE) {
            rightobstaclevar = 1;
          }
          break;

        case 1:  // turn left to front
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(leftAngle);

          if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {
            rightobstaclevar = 2;
            wallCounter = 0;
            initialTime = millis();
          }
          break;

        case 2:  // find obstacle
          trackHeading(turningAngle());

          if (leftDist < oriWallDist && !lefterror) {
            wallCounter++;
            if (wallCounter >= 5) {
              obstacleDist = leftDist;
              wallCounter = 0;
              rightobstaclevar = 3;
            }
          }

          if (millis() - initialTime >= 400) {
            wallCounter = 0;
            rightobstaclevar = 4;
          }
          break;

        case 3:  // wait until obstacle passed
          trackHeading(turningAngle());

          if (leftDist >= obstacleDist && !lefterror) {
            wallCounter++;
            if (wallCounter >= 10) {
              wallCounter = 0;
              rightobstaclevar = 4;
            }
          }
          break;

        case 4:  // turn left
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(leftAngle);

          if (fabsf(getError(miniLeftAngle())) < TURNTOLERANCE) {
            rightobstaclevar = 5;
            wallCounter = 0;
          }
          break;

        case 5:  // turn right back to front
          digitalWrite(28, LOW);
          digitalWrite(29, HIGH);
          runPosition(rightAngle);

          if (fabsf(getError(turningAngle())) < TURNTOLERANCE) {
            rightobstaclevar = 0;
            leftGreen = rightGreen = leftRed = rightRed = false;
            var = prevvar;
          }
          break;
      }

      break;



    case WALLTRACK:
      Serial.println("WALLTRACK");

      if (leftGreen) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("leftgreen");

      } else if (rightGreen) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("rightgreen");

      } else if (leftRed) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("leftred");

      } else if (rightRed) {
        prevvar = WALLTRACK;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("rightred");
      }

      if (cw) {
        //trackHeading(turningAngle());
        rightWallTrack(WALLTRACKDIST, turningAngle());
        if (!fronterror && frontDist < FRONTTHRES) {
          frontDistCounter++;
        } else {
          frontDistCounter = 0;
        }


        if (!righterror && frontDistCounter > 50 && rightDist > CORNERDETECT) {

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
        } else {
          frontDistCounter = 0;
        }

        if (!lefterror && frontDistCounter > 50 && leftDist > CORNERDETECT) {

          if (corner == 11) {  //11
            Serial.println("hello");
            initialpulseCountback = pulseCountback;

            var = OVERSHOOT;
          } else {
            initialpulseCountback = pulseCountback;
            digitalWrite(28, LOW);
            digitalWrite(29, LOW);
            delay(1000);
            Serial.println(leftDist);
            Serial.println("NO");
            var = STOP;
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
      Serial.println(rightStartDist - 10);


      if (cw) {
        trackHeading(turningAngle());

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= (rightStartDist - OVERSHOOTDIST)) {

          lastturn = true;
          corner++;
          /*
          runPosition(zeroPosition);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          */
          var = SMALLLEFTTURN;
        }
      } else {
        trackHeading(-(turningAngle()));

        if (abs(((pulseCountback * gearRatio) * ((PI * 40) / 360)) - ((initialpulseCountback * gearRatio) * ((PI * 40) / 360))) >= (leftStartDist - OVERSHOOTDIST)) {

          lastturn = true;
          corner++;
          /*
          runPosition(zeroPosition);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          */
          var = SMALLRIGHTTURN;
        }
      }

      break;

    case LASTWALL:
      Serial.println("LASTWALL");
      Serial.println(snappedHeading);

      if (leftGreen) {
        prevvar = LASTWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("leftgreen");

      } else if (rightGreen) {
        prevvar = LASTWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = rightDist;
        var = LEFTOBSTACLE;
        Serial.println("rightgreen");

      } else if (leftRed) {
        prevvar = LASTWALL;
        obstacleTurnAngle = SMALLTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("leftred");

      } else if (rightRed) {
        prevvar = LASTWALL;
        obstacleTurnAngle = BIGTURNANGLE;
        oriWallDist = leftDist;
        var = RIGHTOBSTACLE;
        Serial.println("rightred");
      }

      if (startLeft) {
        leftWallTrack(startDist, 0);
      } else {
        rightWallTrack(startDist, 0);
      }

      if (!fronterror && !backerror && frontDist <= frontStartDist + DISTTOLERANCE && backDist >= backStartDist - DISTTOLERANCE) {  //CHANGE TOLERANCE

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

      digitalWrite(28, HIGH);
      digitalWrite(29, HIGH);
      runPosition(zeroPosition);
      break;
  }
}
