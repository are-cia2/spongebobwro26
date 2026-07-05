#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <EEPROM.h>  // RP2040 native flash-emulated non-volatile storage layer
#include <EVN.h>
EVNAlpha board;

// =================================================================================
// CONFIGURATION ROUTINE DEFINITION
// =================================================================================
// UNCOMMENT this line to calibrate the sensor and save offsets to QSPI Flash.
// COMMENT out this line to run normally using saved offsets instantly on boot.
//#define RUN_CALIBRATION

// Base address map markers for the EEPROM storage emulator
#define EEPROM_VALID_FLAG_ADDR 0
#define EEPROM_OFFSETS_ADDR 4
#define EEPROM_MAGIC_SIGNATURE 0x6500A5A5

#define START 0
#define FIRSTWALL 1
#define TURNLEFT 2
#define TURNRIGHT 3
#define WALLTRACK 4
#define STOP 5
#define PRINT 6
#define FINDWALL 7

#define LEFTDS 4
#define RIGHTDS 5
#define FRONTDS 3
#define BACKDS 2

float leftAngle;
float rightAngle;
float zeroPosition;
float startDist;

int var = START;

bool cw = true;
bool startLeft = true;
int corner = 0;



// Struct configuration layout matching local hardware requirements
struct IMUOffsets {
  int16_t xGyro;
  int16_t yGyro;
  int16_t zGyro;
  int16_t zAccel;
};

MPU6050 mpu;

struct YPRData {
  float yaw;
  float pitch;
  float roll;
  bool valid;
};

YPRData stuff;

bool dmpReady = false;
uint16_t packetSize;
uint8_t fifoBuffer[64];

YPRData getYawPitchRoll();
void handleCalibrationSequence();
void loadSavedOffsets();
//volatile bool setupComplete = false;


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
  targetPosition = constrain(targetPosition, rightAngle, leftAngle);
  int positionError = pulseCount - targetPosition;  //(headingError * 5.0) - (steer.getPosition() - STEER_CENTER);
  //Serial.println(positionError);
  int pwmRun = constrain(positionError * 22, -250, 250);
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
int steer_right_bound = 0;
int steer_left_bound = 0;

int steerOffset = 5;

void setup() {
  board.begin();
  board.setPort(1);
  Serial.begin(115200);
  //while (!Serial)
  //  ;

  // Initialize the emulated EEPROM sector sizing (allocation inside the QSPI chip)
  EEPROM.begin(512);

  //Wire.begin();
  //Wire.setClock(400000);

  Serial.println(F("Initializing MPU6500..."));
  mpu.initialize();

  uint8_t chipID = mpu.getDeviceID();
  if (chipID == 0x00 || chipID == 0xFF) {
    Serial.println(F("Hardware connection failed. Check your SDA/SCL lines."));
    while (1)
      ;
  }
  Serial.println(F("Hardware communications online."));

  // Upload DMP firmware configuration payload onto the chip
  uint8_t devStatus = mpu.dmpInitialize();

  if (devStatus == 0) {

#ifdef RUN_CALIBRATION
    // Call the automated routine to calculate errors and save them to QSPI Flash
    handleCalibrationSequence();
#else
    // Pull previously stored offset data profiles out of QSPI flash instantly
    loadSavedOffsets();
#endif

    mpu.setDMPEnabled(true);
    packetSize = mpu.dmpGetFIFOPacketSize();
    dmpReady = true;
    Serial.println(F("System operational."));
  } else {
    Serial.print(F("DMP Initialization failed. Code: "));
    Serial.println(devStatus);
    while (1)
      ;
  }
  //setupComplete = true;

  SIG_B = digitalRead(Pin_B);  // Current state of B
  SIG_A = SIG_B > 0 ? 0 : 1;   // Let them be different
  // Attach iterrupt for state change, not rising or falling edges
  attachInterrupt(digitalPinToInterrupt(Pin_A), A_CHANGE, CHANGE);

  pinMode(23, OUTPUT);
  pinMode(22, OUTPUT);
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
  steer_right_bound = pulseCount;
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
  digitalWrite(21, HIGH);

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
}

/**
 * @brief Performs internal multi-point sensor averages and saves variables permanently.
 */
void handleCalibrationSequence() {
  Serial.println(F("[CALIBRATION] Starting... PLACE SENSOR FLAT AND COMPLETELY STILL!"));
  delay(3000);  // 3-second safety window to allow hand tremors to decay completely

  // 6 calculation loops executed directly inside internal hardware pipelines
  Serial.println("Calibrating Accel");
  mpu.CalibrateAccel(6);
  Serial.println("Calibrating Gyro");
  mpu.CalibrateGyro(6);
  Serial.println(F("[CALIBRATION] Offsets calculated by MPU hardware engine."));

  // Read generated offset values from the MPU6500 hardware registers
  IMUOffsets computedOffsets;
  computedOffsets.xGyro = mpu.getXGyroOffset();
  computedOffsets.yGyro = mpu.getYGyroOffset();
  computedOffsets.zGyro = mpu.getZGyroOffset();
  computedOffsets.zAccel = mpu.getZAccelOffset();

  // Commit values sequentially down to the QSPI Flash emulator array block
  EEPROM.put(EEPROM_VALID_FLAG_ADDR, (uint32_t)EEPROM_MAGIC_SIGNATURE);
  EEPROM.put(EEPROM_OFFSETS_ADDR, computedOffsets);

  // Explicitly command the RP2040 memory controllers to execute physical block writes
  bool success = EEPROM.commit();

  if (success) {
    Serial.println(F("[QSPI FLASH] Success! Calibration structures written to permanent memory."));
    Serial.print(F("XGyro: "));
    Serial.println(computedOffsets.xGyro);
    Serial.print(F("YGyro: "));
    Serial.println(computedOffsets.yGyro);
    Serial.print(F("ZGyro: "));
    Serial.println(computedOffsets.zGyro);
    Serial.print(F("ZAccel: "));
    Serial.println(computedOffsets.zAccel);
  } else {
    Serial.println(F("[ERROR] Flash memory write tracking execution failure."));
  }

  Serial.println(F("Calibration phase complete. Comment out #define RUN_CALIBRATION and re-upload."));
  while (1)
    ;  // Halt execution to prevent parsing bad loops during active tuning
}

/**
 * @brief Retrieves stored registers directly out of QSPI flash structures.
 */
void loadSavedOffsets() {
  uint32_t validationSignature = 0;
  EEPROM.get(EEPROM_VALID_FLAG_ADDR, validationSignature);

  if (validationSignature == EEPROM_MAGIC_SIGNATURE) {
    IMUOffsets storedOffsets;
    EEPROM.get(EEPROM_OFFSETS_ADDR, storedOffsets);

    // Inject the recovered values directly into the active MPU registers
    mpu.setXGyroOffset(storedOffsets.xGyro);
    mpu.setYGyroOffset(storedOffsets.yGyro);
    mpu.setZGyroOffset(storedOffsets.zGyro);
    mpu.setZAccelOffset(storedOffsets.zAccel);
    Serial.println(F("[QSPI FLASH] Valid hardware configuration loaded."));
  } else {
    Serial.println(F("[WARNING] No calibration footprint found in QSPI. Using fallback zeros."));
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);
    mpu.setZAccelOffset(0);
  }
}

/**
 * @brief Polls the MPU6500 FIFO queue, processes orientation vectors, and extracts angles.
 */
YPRData getYawPitchRoll() {
  board.setPort(1);
  YPRData data = { 0.0f, 0.0f, 0.0f, false };
  if (!dmpReady) return data;

  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    Quaternion q;
    VectorFloat gravity;
    float ypr[3];

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    data.yaw = ypr[0] * 180.0f / M_PI;
    data.valid = true;
  }
  return data;
}


float getError(float angle) {

  //float targetAngle = initialAngle + angle;

  while (angle >= 180) {
    angle = angle - 360;
  }

  while (angle <= -180) {
    angle = 360 + angle;
  }

  float error = 999;
  YPRData stuff;

  while (true) {
    stuff = getYawPitchRoll();
    if (stuff.valid) {
      break;
    }
  }
  error = angle - stuff.yaw;
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


int tfVal(byte port = LEFTDS) {
  board.setPort(port);
  int distance = 0;

  Wire.beginTransmission(0x10);
  Wire.write(0x00);             // Command to read distance (register 0x00)
  Wire.endTransmission(false);  // Send repeated start

  Wire.requestFrom(0x10, 3);  // Request 3 bytes: distance LSB, MSB, and checksum

  if (Wire.available() == 3) {
    byte lsb = Wire.read();
    byte msb = Wire.read();
    byte checksum = Wire.read();

    distance = (msb << 8) | lsb;

    //Serial.print("Distance: ");
    //Serial.print(distance);
    //Serial.println(" cm");
  }

  return distance;  // 0 means error
}


void leftWallTrack(int distance, int baseAngle) {
  digitalWrite(28, LOW);
  digitalWrite(29, HIGH);

  float leftError = tfVal(LEFTDS) - distance;
  //Serial.println(leftError);
  //Serial.print(tfVal(4));
  float targetAngle = constrain(leftError * 5, -30, 30);
  //Serial.println(targetAngle);

  runPosition(zeroPosition + getError(targetAngle));
}

void rightWallTrack(int distance, int baseAngle) {
  digitalWrite(28, LOW);
  digitalWrite(29, HIGH);

  float rightError = tfVal(RIGHTDS) - distance;
  //Serial.println(leftError);
  //Serial.print(tfVal(4));
  float targetAngle = constrain(rightError * 5, -30, 30) + baseAngle;
  //Serial.println(targetAngle);

  runPosition(zeroPosition + getError(targetAngle));
}

void trackHeading(int angle) {
  digitalWrite(28, LOW);
  digitalWrite(29, HIGH);

  runPosition(zeroPosition + getError(angle));
}

void loop() {


  switch (var) {

    case PRINT:
      Serial.print(tfVal(LEFTDS));
      Serial.print("\t");
      Serial.print(tfVal(RIGHTDS));
      Serial.print("\t");
      Serial.print(tfVal(FRONTDS));
      Serial.print("\t");
      Serial.println(tfVal(BACKDS));

      YPRData stuff;

      while (true) {
        stuff = getYawPitchRoll();
        if (stuff.valid) {
          break;
        }
      }
      Serial.println(stuff.yaw);

          

      break;

    case START:

    
      if (tfVal(LEFTDS) < tfVal(RIGHTDS)) {
        startLeft = true;
        startDist = tfVal(LEFTDS);
        
      } else {
        startLeft = false;
        startDist = tfVal(RIGHTDS);

      }
      corner = 0;
      

      /*
      Serial.print(tfVal(4));
      Serial.print("\t");
      Serial.println(tfVal(5));
      */

      var = FIRSTWALL;

      break;

    case FIRSTWALL:
      
      if (startLeft) {
        //Serial.println("LEFT");
        leftWallTrack(startDist, 0);
      } else {
        //Serial.println("RIGHT");
        rightWallTrack(startDist, 0);
      }

      Serial.println(tfVal(LEFTDS));

      if (tfVal(LEFTDS) > 40) {
        cw = false;
        //var = STOP;
        corner++;
        var = TURNLEFT;

      }  else if (tfVal(RIGHTDS) > 40) {
        cw = true;
        //var = STOP;
        corner++;
        var = TURNRIGHT;
      }

      break;

    case TURNLEFT:
    //runPosition(leftAngle);
    
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(leftAngle); 
      //zeroPosition + getError(90));

      stuff = getYawPitchRoll();

      //Serial.println(stuff.yaw);
      //Serial.println(stuff.valid);

      if (stuff.valid && stuff.yaw >= turningAngle()) {
        //var = STOP;
        var = FINDWALL;
        
      }
      

      break;

    case TURNRIGHT:
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      runPosition(rightAngle);
      //zeroPosition + getError(-90));

      if (stuff.valid && stuff.yaw <= -(turningAngle()) {
        var = FINDWALL;
        //var = STOP;
        
      }

      break;

    case FINDWALL:
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
      
      if (cw) {
        trackHeading(-(turningAngle());

        if (tfVal(RIGHTDS) <= 70) {
          corner++;
          var = WALLTRACK;
        }
      } else {
        trackHeading(turningAngle());

        if (tfVal(LEFTDS) <= 70) {
          
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
        rightWallTrack(13, -(turningAngle()));

        if (tfVal(5) > 40) {
          cw = true;
          corner++;
          var = TURNRIGHT;
        }

      } else {
        leftWallTrack(13, turningAngle());

        if (tfVal(4) > 40) {
          cw = false;
          corner++;
          var = TURNLEFT;
        }


        break;

      case STOP:
        digitalWrite(28, LOW);
        digitalWrite(29, LOW);
        break;

      }
  }
}