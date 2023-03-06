result = bytearray()

def join(line, sep):
    for ch in line:
        if sep:
            result.append(sep)
        result.append(ord(ch))

def write(line):
    join(line, 0x00)

def press(line, dt=None):
    join(line, 0x81)
    
def release(line):
    join(line, 0x82)

def releaseAll():
    result.append(0x83)

def delay(dt):
    while dt > 0:
        result.append(0x84)
        result.append(min(dt, 255))
        dt -= 255

def load(tty=None):
    pass



import serial
from time import sleep


data = bytes(ord(i) for i in '\x91serial\x92wow!\n\x94')

with serial.Serial('/dev/ttyACM0', 115200) as box:
    box.write(data)
    while True:
        print(*(chr(ch) for ch in box.read()), end='')
