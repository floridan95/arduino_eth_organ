#pragma once
#include <Arduino.h>

#define NUMBER_OF_INPUT_SHIFT_REGS 4
#define DATA_WIDTH NUMBER_OF_INPUT_SHIFT_REGS * 8

#define ploadPin 3
#define clockEnablePin 4
#define dataPin 5
#define clockPin 6



class InputShiftReg
{
public:
    uint32_t bitStateMap;
    typedef void (*midiCallback)(uint8_t note,boolean on);
    midiCallback _midiCallback;
    
    typedef void (*outputCallback)(uint8_t channel, uint8_t note, boolean state);
    outputCallback _outputCallback;
    
    InputShiftReg() {
        pinMode(ploadPin, OUTPUT);
        pinMode(clockEnablePin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(dataPin,INPUT);
    }

    void readShiftRegisters()
    {
        boolean bitval;

        // Latch in the current state of the pins.
        digitalWrite(clockEnablePin, HIGH);
        digitalWrite(ploadPin, LOW);
        digitalWrite(ploadPin, HIGH);
        digitalWrite(clockEnablePin, LOW);

        for (uint8_t i = 0; i < DATA_WIDTH; i++)
        {
            bitval = !digitalRead(dataPin);
            if(bitRead(bitStateMap,i) != bitval){
                bitWrite(bitStateMap,i,bitval);
                _midiCallback(i+36,bitval);
                _outputCallback(1,i+36,bitval);
            }
            /* bitStateMap = bitStateMap | (bitval << ((DATA_WIDTH - 1) - i)); */
            digitalWrite(clockPin, HIGH);
            digitalWrite(clockPin, LOW);
            
        }
    }

    void setMIDICallback(midiCallback callback){
        _midiCallback = callback;
    }

    void setOutputMessageCallback(outputCallback callback){
        _outputCallback = callback;
    }
};