#define DEFAULT_DELAY 50
#define DEBOUNCE_DELAY 200
#define ERROR_DELAY 500

#define LCD_ADDRESS 0x27
#define SD_CS_PIN 10

#define BUTTON_OK_PIN 7
#define BUTTON_UP_PIN 8
#define BUTTON_DOWN_PIN 9

#define MAX_FILENAME_LENGTH 8



#include <Keyboard.h>

#include <SPI.h>
#include <SD.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>



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
    ERROR,
    IDLE = 0x90,
    NAME,
    DATA,
    DONE,
    OPEN,
};



File root, file;
LiquidCrystal_I2C lcd(0x27, 20, 4);


volatile bool execute;
uint8_t maxFileNumber = 0, displayPage = 0, fileNumber = 0;

uint64_t lastDebounceTime;



void setup() {
    Serial.begin(115200);

    if (!SD.begin(SD_CS_PIN)) panic();
    root = SD.open("/");                // кто закроет тот здохнет
    maxFileNumber = rerollToFile(-1);

    Keyboard.begin();
    delay(1000);

    lcd.init();
    lcd.backlight();
    updateDisplay(true);

    pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    attachInterrupt(4, stopExecute, FALLING);
}


void loop() {
    if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
        if (!digitalRead(BUTTON_OK_PIN)) {
            rerollToFile(fileNumber);
            file = root.openNextFile();
            executeCurrentFile();
            file.close();
        } else if (!digitalRead(BUTTON_DOWN_PIN)) {
            if (++fileNumber == maxFileNumber) fileNumber = 0;
            updateDisplay(false);
        } else if (!digitalRead(BUTTON_UP_PIN)) {
            if (!fileNumber--) fileNumber += maxFileNumber;
            updateDisplay(false);
        } else {
            goto noclick;
        }
        lastDebounceTime = millis();
    }
    noclick:
    if (Serial.available()) {
        parseSerial();
        maxFileNumber = rerollToFile(-1);
        if (fileNumber == maxFileNumber) --fileNumber;
        updateDisplay(true);
    }
}



void updateDisplay(bool hard) {
    if (!maxFileNumber) {
        lcd.clear();
        lcd.print("Upload file first");
        fileNumber = 0;
        return;
    }
    if (fileNumber / 8 != displayPage || hard) {
        lcd.clear();
        displayPage = fileNumber / 8;
        rerollToFile(displayPage * 8);
        
        for (uint8_t i = 0; i < 8; ++i) {
            file = root.openNextFile();
            if (!file) break;
            lcd.setCursor(1+i/4*10, i%4);
            lcd.print(file.name());
            file.close();
        }
    }
    for (uint8_t i = 0; i < 8; ++i) {
        lcd.setCursor(i/4*10, i%4);
        if (i != fileNumber%8) lcd.print(' ');
        else lcd.print('*');
    }
}


uint8_t rerollToFile(uint8_t number) {
    uint8_t result = 0;
    root.rewindDirectory();
    while (result != number && (file = root.openNextFile())) {
        file.close();
        ++result;
    }
    return result;
}


void parseSerial() {

    uint8_t name[MAX_FILENAME_LENGTH], pos = 0;
    uint8_t ch, state = IDLE;

    while (Serial.available() && state != ERROR) {
        ch = Serial.read();

        switch (state) {
            case IDLE:
                if (ch == NAME) {
                    state = ch;
                    break;
                }
                if (ch == OPEN) {
                    state = DATA;
                    if (!maxFileNumber) break;
                    rerollToFile(fileNumber);
                    file = root.openNextFile();
                    while (name[pos] = file.name()[pos]) ++pos;
                    break;
                }
                break;

            case NAME:
                if (ch == DATA) {
                    name[pos] = '\0';
                    if (SD.exists(name)) SD.remove(name);
                    file = SD.open(name, FILE_WRITE);
                    if (file) state = ch;
                    else state = ERROR;
                    break;
                }
                if (ch == '/' || pos == MAX_FILENAME_LENGTH) {
                    state = ERROR;
                    break;
                }
                name[pos++] = ch;
                break;

            case DATA:
                if (ch == DONE) {
                    state = IDLE;
                    file.close();
                    Serial.println("OK");
                    break;
                }
                file.write(ch);
                break;
        }
    }
    if (file) {
        Serial.println("FAIL");
        file.close();
        SD.remove(name);
    }
}


void executeCurrentFile() {

    uint8_t ch, modifier = NOTHING;
    lastDebounceTime = millis();
    execute = true;

    while (file.available() && execute) {
        ch = file.read();

        if (modifier == NOTHING && ch != DELAY) delay(DEFAULT_DELAY);

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


void stopExecute() {
    if (millis() - lastDebounceTime > DEBOUNCE_DELAY) execute = false;
}


void panic() {
    DDRB |= 1;
    PORTB &= ~1;
    while (1) {
        PORTB ^= 1;
        delay(ERROR_DELAY);
    }
}
