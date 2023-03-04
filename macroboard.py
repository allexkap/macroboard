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
    if dt:
        delay(dt)
        release(line)

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
