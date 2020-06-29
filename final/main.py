import pyb
import sensor, image, time, os, tf

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)
tmp = ""

def takePicture():
    RED_LED_PIN = 1
    BLUE_LED_PIN = 3

    sensor.reset() # Initialize the camera sensor.
    sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
    sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
    sensor.skip_frames(time = 2000) # Let new settings take affect.

    pyb.LED(RED_LED_PIN).on()
    sensor.skip_frames(time = 2000) # Give the user time to get ready.

    pyb.LED(RED_LED_PIN).off()
    pyb.LED(BLUE_LED_PIN).on()

    print("You're on camera!")
    sensor.snapshot().save("example.jpg") # or "example.bmp" (or others)

    pyb.LED(BLUE_LED_PIN).off()
    print("Done! Reset the camera to see the saved image.")
    uart.write("a".encode())

while(1):
   a = uart.readline()
   if a is not None:
      tmp += a.decode()
      print(a.decode())

   if tmp == "takePicture":
      tmp = ""
      takePicture()
