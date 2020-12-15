#pragma once
#include <Arduino.h>

#define NUMBER_OF_OUTPUT_MATRIX_SHIFT_REGS 1
#define DATA_WIDHT_MATRIX_OUTPUT NUMBER_OF_OUTPUT_MATRIX_SHIFT_REGS * 8

#define LOAD595 8
#define CLOCK595 14
#define DATA595 15
#define CLR595 10


uint8_t cols[8] = {32,33,34,35,36,37,38,39}; // if not using shift registers
uint8_t rows[8] = {40,41,42,43,44,45,46,47};

const uint8_t midiMap[8][8] = {
    /*1,	2,	3,	4,	5,	6,	7,	8,
    9,	10,	11,	12,	13,	14,	15,	16,
    17, 18, 19,	20,	21,	22,	23,	24,
    25,	26,	27,	28,	29,	30,	31,	32,
    33,	34,	35,	36,	37,	38,	39,	40,
    41,	42,	43,	44,	45,	46,	47,	48,
    49,	50,	51,	52,	53,	54,	55,	56,
    57,	58,	59,	60,	61,	62,	63,	64*/
    60,	60,	52,	52,	44,	44,	36,	36,
    61,	61,	53,	53,	45,	45,	37,	37,
    62, 62, 54,	54,	46,	46,	38,	38,
    63,	63,	55,	55,	47,	47,	39,	39,
    64,	64,	56,	56,	48,	48,	40,	40,
    65,	65,	57,	57,	49,	49,	41,	41,
    66,	66,	58,	58,	50,	50,	42,	42,
    67,	67,	59,	59,	51,	51,	43,	43/*
    92, 92, 84, 84, 76, 76, 68, 68,
    93, 93, 85, 85, 77, 77, 69, 69,
    94, 94, 86, 86, 78, 78, 70, 70,
    95, 95, 87, 87, 79, 79, 71, 71,
    96, 96, 88, 88, 80, 80, 72, 72,
    97, 97, 89, 89, 81, 81, 73, 73,
    98, 98, 90, 90, 82, 82, 74, 74,
    99, 99, 91, 91, 83, 83, 75, 75 */
};

uint8_t colIdx = 0;

uint8_t bitStateMap[8]; // 8x8 bits 

// Matrix one by one scanning, or magnet driver
class MatrixKBD
{
public:
    MatrixKBD(uint8_t channel)
    {
        pinMode(LOAD595, OUTPUT);
        pinMode(CLOCK595, OUTPUT);
        pinMode(DATA595, OUTPUT);
        pinMode(CLR595, OUTPUT);
        for(int ele:rows){
            pinMode(ele,INPUT_PULLUP);
        }
        _channel = channel;
    }
    uint8_t _channel;
    uint8_t state;
    uint8_t old_state;
    typedef void (*midiCallback)(uint8_t note,boolean on);
    midiCallback _midiCallback;

    void shift_matrix()
    {
        if (state == 0)
        {
            state = 1;
            colIdx = 0;
        }
        else
        {
            state = state << 1;
            colIdx++;
        }
        shift();
    }

    void shift()
    {
        if (old_state != state)
        {   
            Loggingln("State changed.");
            for(uint8_t i = 0; i< 8; i++){
                Logging(bitRead(state,i));
            }
            
            for (uint8_t i = 0; i < DATA_WIDHT_MATRIX_OUTPUT; i++)
            {

                digitalWrite(DATA595, !bitRead(state, DATA_WIDHT_MATRIX_OUTPUT-1 - i));
                digitalWrite(CLOCK595, HIGH);
                digitalWrite(CLOCK595, LOW);
            }
            digitalWrite(LOAD595, LOW);
            digitalWrite(LOAD595, HIGH);
            
            for(uint8_t i = 0;i<sizeof(rows);i++)
            {
                boolean noteState = !digitalRead(rows[i]);
                if(noteState != bitRead(bitStateMap[colIdx],i)) {
                    _midiCallback(midiMap[colIdx][i],noteState);
                }
                bitWrite(bitStateMap[colIdx],i,state);
            }
            old_state = state;
        }
    }

    void setMIDICallback(midiCallback callback){
        _midiCallback = callback;
    }
    void clear()
    {
        digitalWrite(CLR595, LOW);
        delayMicroseconds(1);
        digitalWrite(CLR595, HIGH);
    }
};
