#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <EEPROM.h> // RP2040 native flash-emulated non-volatile storage layer
#include <EVN.h>
EVNAlpha board;

// =================================================================================
// CONFIGURATION ROUTINE DEFINITION
// =================================================================================
// UNCOMMENT this line to calibrate the sensor and save offsets to QSPI Flash.
// COMMENT out this line to run normally using saved offsets instantly on boot.
//#define RUN_CALIBRATION 

// Base address map markers for the EEPROM storage emulator
#define EEPROM_VALID_FLAG_ADDR  0
#define EEPROM_OFFSETS_ADDR     4
#define EEPROM_MAGIC_SIGNATURE  0x6500A5A5

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

bool dmpReady = false;          
uint16_t packetSize;            
uint8_t fifoBuffer[64]; 

YPRData getYawPitchRoll();
void handleCalibrationSequence();
void loadSavedOffsets();

void setup() {
    board.begin();
    board.setPort(1);
    //Serial.begin(115200);
    while (!Serial); 

    // Initialize the emulated EEPROM sector sizing (allocation inside the QSPI chip)
    EEPROM.begin(512);

    //Wire.begin();
    //Wire.setClock(400000); 

    Serial.println(F("Initializing MPU6500..."));
    mpu.initialize();

    uint8_t chipID = mpu.getDeviceID();
    if (chipID == 0x00 || chipID == 0xFF) {
        Serial.println(F("Hardware connection failed. Check your SDA/SCL lines."));
        while (1);
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
        while(1);
    }
}

void loop() {
    YPRData stuff;

    while (true) {
    stuff = getYawPitchRoll();
    if (stuff.valid) {
        break;
    }
    }
    Serial.println(stuff.yaw);
}

/**
 * @brief Performs internal multi-point sensor averages and saves variables permanently.
 */
void handleCalibrationSequence() {
    Serial.println(F("[CALIBRATION] Starting... PLACE SENSOR FLAT AND COMPLETELY STILL!"));
    delay(3000); // 3-second safety window to allow hand tremors to decay completely

    // 6 calculation loops executed directly inside internal hardware pipelines
    Serial.println("Calibrating Accel");
    mpu.CalibrateAccel(6);
    Serial.println("Calibrating Gyro");
    mpu.CalibrateGyro(6);
    Serial.println(F("[CALIBRATION] Offsets calculated by MPU hardware engine."));

    // Read generated offset values from the MPU6500 hardware registers
    IMUOffsets computedOffsets;
    computedOffsets.xGyro  = mpu.getXGyroOffset();
    computedOffsets.yGyro  = mpu.getYGyroOffset();
    computedOffsets.zGyro  = mpu.getZGyroOffset();
    computedOffsets.zAccel = mpu.getZAccelOffset();

    // Commit values sequentially down to the QSPI Flash emulator array block
    EEPROM.put(EEPROM_VALID_FLAG_ADDR, (uint32_t)EEPROM_MAGIC_SIGNATURE);
    EEPROM.put(EEPROM_OFFSETS_ADDR, computedOffsets);
    
    // Explicitly command the RP2040 memory controllers to execute physical block writes
    bool success = EEPROM.commit(); 

    if (success) {
        Serial.println(F("[QSPI FLASH] Success! Calibration structures written to permanent memory."));
        Serial.print(F("XGyro: "));  Serial.println(computedOffsets.xGyro);
        Serial.print(F("YGyro: "));  Serial.println(computedOffsets.yGyro);
        Serial.print(F("ZGyro: "));  Serial.println(computedOffsets.zGyro);
        Serial.print(F("ZAccel: ")); Serial.println(computedOffsets.zAccel);
    } else {
        Serial.println(F("[ERROR] Flash memory write tracking execution failure."));
    }

    Serial.println(F("Calibration phase complete. Comment out #define RUN_CALIBRATION and re-upload."));
    while(1); // Halt execution to prevent parsing bad loops during active tuning
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
    YPRData data = {0.0f, 0.0f, 0.0f, false};
    if (!dmpReady) return data;

    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
        Quaternion q;           
        VectorFloat gravity;    
        float ypr[3];           

        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        data.yaw   = ypr[0] * 180.0f / M_PI;
        data.pitch = ypr[1] * 180.0f / M_PI;
        data.roll  = ypr[2] * 180.0f / M_PI;
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

