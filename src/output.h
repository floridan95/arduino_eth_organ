#pragma once
#include <Arduino.h>

#define NUMBER_OF_OUTPUT_SHIFT_REGS 2
#define DATA_WIDHT_OUTPUT NUMBER_OF_OUTPUT_SHIFT_REGS * 8

#define LOAD595 8
#define CLOCK595 14
#define DATA595 15
#define CLR595 10

// Matrix one by one scanning, or magnet driver
class Output
{
public:
    Output()
    {
        pinMode(LOAD595, OUTPUT);
        pinMode(CLOCK595, OUTPUT);
        pinMode(DATA595, OUTPUT);
        pinMode(CLR595, OUTPUT);
    }

    uint32_t state;
    uint32_t old_state;

    void shift_matrix()
    {
        if (state == 0)
        {
            state = 1;
        }
        else
        {
            state = state << 1;
        }
        shift();
    }

    void setState(uint8_t index, boolean value)
    {
        bitWrite(state, index-36, value);
        shift();
    }

    void shift()
    {
        if (old_state != state)
        {
            Loggingln("State changed.");
            for(uint8_t i = 0; i< 32; i++){
                Serial.print(bitRead(state,i));
            }
            
            for (uint8_t i = 0; i < DATA_WIDHT_OUTPUT; i++)
            {

                digitalWrite(DATA595, !!bitRead(state, DATA_WIDHT_OUTPUT-1 - i));
                digitalWrite(CLOCK595, HIGH);
                digitalWrite(CLOCK595, LOW);
            }
            digitalWrite(LOAD595, LOW);
            digitalWrite(LOAD595, HIGH);
            old_state = state;
        }
    }

    void clear()
    {
        digitalWrite(CLR595, LOW);
        delayMicroseconds(1);
        digitalWrite(CLR595, HIGH);
    }
};
