#include <Keypad.h>
#include "keyCode.h"

const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};
byte rowPins[ROWS] = {12, 14, 27, 26}; // connect to the row pinouts of the kpd
byte colPins[COLS] = {25, 33, 32}; // connect to the column pinouts of the kpd

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

