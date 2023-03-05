#include <Keyboard.h>


uint8_t PROG[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x84, 0xff, 0x84, 0xff, 0x84, 0xff, 0x84, 0xeb, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x84, 0xff, 0x84, 0xff, 0x84, 0xff, 0x84, 0xeb, 0x20, 0x6d, 0x61, 0x63, 0x72, 0x6f, 0x62, 0x6f, 0x78, 0x21};

void setup() {
    Keyboard.begin();
    delay(1000);
    exec(PROG, sizeof(PROG));
}

void loop() {

}

void exec(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (data[i] != 0x84) delay(50);
        else delay(data[++i]), ++i;     // UB

        switch (data[i]) {
            case 0x81:
                Keyboard.press(data[++i]);
                break;
            case 0x82:
                Keyboard.release(data[++i]);
                break;
            case 0x83:
                Keyboard.releaseAll();
                break;
            default:
                Keyboard.write(data[i]);
                break;
        }
    }
}
