#include <Keyboard.h>
#include <SPI.h>
#include <SD.h>


enum opcodes {
    NOTHING,
    POSTDELAY,
    RESERVED = 0x80,
    PRESS,
    RELEASE,
    RELEASEALL,
    DELAY,
};

enum states {
    IDLE = 0x90,
    NAME,
    DATA,
    ECHO,   // not implemented yet
    DONE,
};


volatile bool execute = true;

File file;


void setup() {
    Serial.begin(115200);

    if (!SD.begin(10)) error();

    Keyboard.begin();
    delay(1000);

    file = SD.open("serial");
    execFromFile();
    file.close();
}


void loop() {
    if (Serial.available()) {
        writeToFile();
    }
}


void writeToFile() {

    uint8_t name[13], pos = 0;
    uint8_t ch, state = IDLE;

    while (Serial.available()) {
        ch = Serial.read();

        switch (state) {
            case IDLE:
                if (ch == NAME)
                    state = ch;
                break;

            case NAME:
                if (ch == DATA) {
                    state = ch;
                    name[pos] = 0;
                    if (SD.exists(name)) SD.remove(name);
                    file = SD.open(name, FILE_WRITE);
                    break;
                }
                if (i == 13) error();
                name[pos++] = ch;
                break;

            case DATA:
                if (ch == DONE) {
                    state = ch;
                    file.close();
                    break;
                }
                file.write(ch);
                break;
        }
    }

    if (file) {
        file.close();
        SD.remove(name);
        error();
    }
}


void execFromFile() {

    if (!file) error();

    uint8_t ch, modifier = NOTHING;
    execute = true;

    while (file.available() && execute) {
        ch = file.read();

        if (modifier == NOTHING && ch != DELAY) delay(50);

        switch (modifier) {
            case PRESS:
                Keyboard.press(ch);
                modifier = NOTHING;
                break;

            case RELEASE:
                Keyboard.release(ch);
                modifier = NOTHING;
                break;

            case DELAY:
                delay(ch);
                modifier = POSTDELAY;
                break;

            default:
                modifier = NOTHING;
                if (ch < 128)
                    Keyboard.write(ch);
                else if (ch == RELEASEALL)
                    Keyboard.releaseAll();
                else
                    modifier = ch;
                break;
        }
    }
}


void execFromString(char *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (data[i] != DELAY) delay(50);
        else delay(data[++i]), ++i;     // UB

        switch (data[i]) {
            case PRESS:
                Keyboard.press(data[++i]);
                break;
            case RELEASE:
                Keyboard.release(data[++i]);
                break;
            case RELEASEALL:
                Keyboard.releaseAll();
                break;
            default:
                Keyboard.write(data[i]);
                break;
        }
    }
}

void error() {
    DDRB |= 1;
    while (1) {
        PORTB ^= 1;
        delay(500);
    }
}
