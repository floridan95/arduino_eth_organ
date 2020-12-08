#pragma once
#include <Arduino.h>

#define NUMBER_OF_INPUT_SHIFT_REGS 4
#define DATA_WIDTH NUMBER_OF_INPUT_SHIFT_REGS * 8

#define ploadPin 3
#define clockEnablePin 4
#define dataPin 5
#define clockPin 6

#define BYTES_VAL_T uint32_t

class InputShiftReg
{
public:
    BYTES_VAL_T readShiftRegisters()
    {
        uint32_t bitval;
        BYTES_VAL_T bytesVal = 0;

        // Latch in the current state of the pins.
        digitalWrite(clockEnablePin, HIGH);
        digitalWrite(ploadPin, LOW);
        digitalWrite(ploadPin, HIGH);
        digitalWrite(clockEnablePin, LOW);

        for (uint8_t i = 0; i < DATA_WIDTH; i++)
        {
            bitval = digitalRead(dataPin);
            bytesVal = bytesVal | (bitval << ((DATA_WIDTH - 1) - i));

            digitalWrite(clockPin, HIGH);
            digitalWrite(clockPin, LOW);
        }
        return bytesVal;
    }
};