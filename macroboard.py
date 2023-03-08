from serial import Serial
from keys import *


opcodes = {
    'NOTHING':     0x00,
    'RESERVED':   0xF0,
    'PRESS':      0xF1,
    'RELEASE':    0xF2,
    'RELEASEALL': 0xF3,
    'DELAY':      0xF4,
}
states = {
    'IDLE': 0xF8,
    'NAME': 0xF9,
    'DATA': 0xFA,
    'DONE': 0xFB,
    'OPEN': 0xFC,
}


def _load(data, tty='/dev/ttyACM0'):
    with Serial(tty, 115200, timeout=0.1) as box:
        box.write(data)
        echo = bytes()
        while not echo:
            echo = box.readall()
        print(echo.decode())

def _createFile(filename):
    data = bytearray()
    data.append(states['NAME'])
    data.extend(filename.encode(encoding='ascii'))
    data.append(states['DATA'])
    data.extend(actions)
    data.append(states['DONE'])
    return data

def _join(line, sep):
    for ch in line:
        if sep: actions.append(sep)
        actions.append(ord(ch))


def clear():
    actions.clear()

def write(line):
    _join(line, opcodes['NOTHING'])

def press(line):
    _join(line, opcodes['PRESS'])

def release(line):
    _join(line, opcodes['RELEASE'])

def releaseAll():
    actions.append(opcodes['RELEASEALL'])

def delay(time):
    while time > 0:
        if opcodes['DELAY'] == time:
            time -= 1
        actions.append(opcodes['DELAY'])
        actions.append(min(time, 255))   # fix
        time -= 255

def save(filename='default', *args, **kwargs):
    data = _createFile(filename)
    _load(data, *args, **kwargs)

def remove(filename=None, *args, **kwargs):
    if filename:
        data = _createFile(filename)
        data.pop()
    else:
        data = bytes((states['OPEN'],))
    _load(data, *args, **kwargs)



actions = bytearray()

if __name__ == '__main__':
    write('macroboard')
    releaseAll()
    save()
