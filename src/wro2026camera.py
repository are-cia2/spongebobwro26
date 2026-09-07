import csi
import time
import math
from machine import UART
from machine import LED

led_blue = LED("LED_BLUE")
led_blue.on()


# 1. Initialize UART Communications
uart = UART(1, 115200, timeout_char=200)

# 2. Color Threshold Arrays (LAB Color Space)
threshold_green = [(9, 27, -128, -9, -128, 127)]
threshold_red = [(21, 38, 13, 127, -128, 127)]

# 3. OpenMV v5.0.0+ CSI Camera Initialization Engine
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=500)

# 4. Strict Manual Sensor Control Overrides
csi0.auto_gain(False, gain_db=24.082400)
csi0.snapshot(time=500)

csi0.auto_whitebal(False, rgb_gain_db=((1.604295, 0.0, 7.31645)))
csi0.snapshot(time=500)

csi0.auto_exposure(False, exposure_us=int(16648))
csi0.snapshot(time=500)

csi0.vflip(True)
csi0.hmirror(True)

# 5. Runtime Tracking Variables
clock = time.clock()
greenCounter = 0
redCounter = 0
redFound = False
greenFound = False

# 6. Global LED Definitions (Avoids inner loop memory allocation)
led_red = LED("LED_RED")
led_green = LED("LED_GREEN")
led_blue.off()

# 7. Front-filtering Settings (Targeting vertical objects at 90° with a 20° buffer)
TARGET_ROTATION = math.radians(90)
ROTATION_TOLERANCE = math.radians(20)

# Target Horizon Track Line set to 107
HORIZON_Y = 107

# 8. Main Execution Loop
while True:
    clock.tick()

    img = csi0.snapshot()

    # 🛠️ FIXED: Coordinates bundled into a tuple mapping (x0, y0, x1, y1)
    # img.draw_line((0, HORIZON_Y, img.width(), HORIZON_Y), color=(255, 255, 255), thickness=1)

    # Detect raw raw blobs via v5.0.0 API
    rawRedBlobs = img.find_blobs(threshold_red, pixels_threshold=250, area_threshold=250, merge=True)
    rawGreenBlobs = img.find_blobs(threshold_green, pixels_threshold=500, area_threshold=500, merge=True)

    # FRONT-FILTER: Ensure target blobs are vertical AND crossing the Y=107 horizon boundary
    # (b.y is the top boundary, and b.y + b.h is the bottom boundary)
    redBlobs = [
        b for b in rawRedBlobs
        if abs(b.rotation - TARGET_ROTATION) < ROTATION_TOLERANCE
        and b.y <= HORIZON_Y <= (b.y + b.h)
    ]

    greenBlobs = [
        b for b in rawGreenBlobs
        if abs(b.rotation - TARGET_ROTATION) < ROTATION_TOLERANCE
        and b.y <= HORIZON_Y <= (b.y + b.h)
    ]

    # Evaluate the largest qualified element based on the direct property .pixels
    largestRed = max(redBlobs, key=lambda b: b.pixels) if redBlobs else None
    largestGreen = max(greenBlobs, key=lambda b: b.pixels) if greenBlobs else None

    seenBlob = None
    blobColour = None

    if largestRed and largestGreen:
        if largestRed.pixels >= largestGreen.pixels:
            seenBlob = largestRed
            blobColour = "red"
        else:
            seenBlob = largestGreen
            blobColour = "green"
    elif largestRed:
        seenBlob = largestRed
        blobColour = "red"
    elif largestGreen:
        seenBlob = largestGreen
        blobColour = "green"
    else:
        seenBlob = False
        redFound = False
        greenFound = False

    # 9. Control Tracking & Interlock Logic Setup
    if seenBlob:
        rect = seenBlob.rect
        img.draw_rectangle(rect)

        if blobColour == "red":
            greenFound = False
            redFound = True
            greenCounter = 0
            print("red", redCounter)
            redCounter += 1
            if redCounter > (clock.fps() * 0.5):
                led_green.off()
                led_blue.on()
                led_red.off()

                # Check bounding box height element explicitly (Index 3 of rect tuple)
                if rect[3] >= 85:
                    led_green.off()
                    led_blue.off()
                    led_red.on()
                    if seenBlob.cx < 140:
                        print("leftred")
                        uart.write("c")
                    else:
                        print("rightred")
                        uart.write("d")
        elif blobColour == "green":
            redFound = False
            greenFound = True
            redCounter = 0

            print("green", greenCounter)
            greenCounter += 1

            if greenCounter >= (clock.fps() * 0.5):
                led_green.off()
                led_blue.on()
                led_red.off()
                # Check bounding box height element explicitly (Index 3 of rect tuple)
                if rect[3] >= 85:
                    led_red.off()
                    led_blue.off()
                    led_green.on()
                    if seenBlob.cx < 140:
                        print("leftgreen")
                        uart.write("a")
                    else:
                        print("rightgreen")
                        uart.write("b")
    else:
        # Failsafe default tracking condition state
        led_green.off()
        led_blue.off()
        led_red.off()

    if not redFound:
        redCounter = 0
    if not greenFound:
        greenCounter = 0
