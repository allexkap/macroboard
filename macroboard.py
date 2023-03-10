from serial import Serial
import os
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


def _load(data, args):
    with Serial(args.port, 115200, timeout=0.1) as box:
        box.write(data)
        echo = bytes()
        while not echo:
            echo = box.readall()
        print(echo.decode()) # todo

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


def _save(args):
    data = _createFile(args.name)
    _load(data, args)

def _remove(args):
    if args.name:
        data = _createFile(args.name)
        data[-1] = states['IDLE']
    else:
        data = bytes((states['OPEN'], states['IDLE']))
    _load(data, args)


def parse_args():
    import argparse
    parser = argparse.ArgumentParser(description='Macros loader into board')
    parser.add_argument('path', nargs='?', help='path to file with macros')
    parser.add_argument('-r', '--remove', help='remove macros by name', action='store_true')
    parser.add_argument('-n', '--name', help='macros name', default='default')
    parser.add_argument('-t', '--port', help='board port', default='/dev/ttyACM0')
    return parser.parse_args()



actions = bytearray()

if __name__ == '__main__':
    args = parse_args()

    if args.remove:
        if args.path:
            print('Ignore path')
        _remove(args)
    else:
        with open(os.path.abspath(args.path)) as file:
            exec(file.read())
        _save(args)
