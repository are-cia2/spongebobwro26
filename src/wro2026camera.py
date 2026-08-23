import csi
import time
from machine import UART
from machine import LED
uart = UART(1, 115200, timeout_char=200)

threshold_green = [(17, 46, -26, -5, -8, 16)]
threshold_red = [(38, 53, 22, 40, 16, 36)]

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=500)

csi0.auto_gain(False, gain_db=15)
csi0.snapshot(time=500)

csi0.auto_whitebal(False, rgb_gain_db=((0.686702, 0.0, 7.43538)))
csi0.snapshot(time=500)


csi0.auto_exposure(
    False, exposure_us=int(16648)
)
csi0.snapshot(time=500)

csi0.vflip(True)
csi0.hmirror(True)

clock = time.clock()
state = 0

prevstate = 0
greenCounter = 0
redCounter = 0
# led = LED("LED_BLUE")

while True:
    clock.tick()

    img = csi0.snapshot()
    state = 0
    LED("LED_GREEN").off()
    LED("LED_RED").off()
    LED("LED_BLUE").on()
    for blob in img.find_blobs(
        threshold_red,
        pixels_threshold=2500,
        area_threshold=2500,
        merge=True,
    ):
        if abs(blob.rotation_deg()-90) < 20:
            # print(blob.area())
            # print(blob.pixels())
            rect = blob.rect()
            img.draw_rectangle(rect)
            LED("LED_GREEN").off()
            LED("LED_BLUE").off()
            LED("LED_RED").on()
            print("red")
            print(redCounter)
            redCounter = redCounter + 1
            if redCounter > 50:
                if 200 - (rect[1] + rect[3]) < 90:
                    img.draw_rectangle(rect)
                    if blob.cx() < 140:
                        # leftred
                        print("leftred")
                        uart.write("c")
                        redCounter = 0
                    else:
                        # rightred
                        print("rightred")
                        uart.write("d")
                        redCounter = 0


    for blob in img.find_blobs(
        threshold_green,
        pixels_threshold=2500,
        area_threshold=2500,
        merge=True,
    ):
        if abs(blob.rotation_deg()-90) < 20:
            # print(blob.area())
            rect = blob.rect()
            LED("LED_GREEN").on()
            LED("LED_BLUE").off()
            LED("LED_RED").off()
            img.draw_rectangle(rect)
            greenCounter = greenCounter + 1
            if greenCounter > 50:
                if 200 - (rect[1] + rect[3]) < 90:
                    if blob.cx() < 140:
                        # leftgreen
                        print("leftgreen")
                        uart.write("a")
                        greenCounter = 0
                    else:
                        # rightgreen
                        print("rightgreen")
                        uart.write("b")
                        greenCounter = 0
