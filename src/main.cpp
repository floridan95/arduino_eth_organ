#define DEBUG

#include <Arduino.h>
#include "main.h"
#include "output.h"
#include "matrix_read.h"
#include "input_shift_reg.h"

#include <Ethernet.h>
#include <ipMIDI.h>
#include <HystFilter.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip(192, 168, 88, 12);
static uint8_t reply[] = {192, 168, 88, 255};
Output o = Output();
MatrixKBD m = MatrixKBD(1);
InputShiftReg iReg = InputShiftReg();
IPMIDI_CREATE_DEFAULT_INSTANCE();

long test = 0;
int t1, t2;
HystFilter analog1(1024, 16, 5);
HystFilter swellShades(128, 16, 5);

boolean first_second_octave_coupler; // set these with register switches - currently not connected
boolean first_octave_coupler;

uint16_t cresc;

uint8_t cresc_mem[32] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0,
                         11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0};

uint16_t setzer_mem[16] = {
    0b1011001111000110,
    0b0100000010110100,
    0b1100101011101111,
    0b0101011110111001,
    0b1010001100010000,
    0b0110110010010100,
    0b1101001010010010,
    0b0001110110010100,
    0b1111111110001000,
    0b1100000001000001,
    0b1000110111010101,
    0b0110010101001010,
    0b0111111001010011,
    0b0001000100101010,
    0b0011100110101100,
    0b1001111111101100};

void setup()
{
    // You can use Ethernet.init(pin) to configure the CS pin
    Ethernet.init(53); // Most Arduino shields

#ifdef DEBUG
    Serial.begin(115200);
    while (!Serial)
    {
        ; // Waiting for serial connection
    }
#endif
    // start the Ethernet
    Ethernet.begin(mac, ip);
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Loggingln("Ethernet shield was not found.");
        while (true)
        {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    Loggingln("Waiting for link");
    while (Ethernet.linkStatus() != LinkON)
    {
        delay(100); // do nothing, no point running without Ethernet hardware
    }
    Loggingln("IP address");
    Loggingln(Ethernet.localIP());

    Loggingln("Starting Connection");
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.getTransport()->setReply(reply);
    MIDI.setHandleNoteOn(OnMidiNoteOn);
    MIDI.setHandleNoteOff(OnMidiNoteOff);
    MIDI.setHandleControlChange(OnMidiControlChange);
    m.setMIDICallback(sendMessage);
    iReg.setMIDICallback(sendMessage);
    iReg.setOutputMessageCallback(midiProcess);

    t1 = millis();
    pinMode(A0, INPUT);

    Loggingln("Starting Loop");
}

void loop()
{
    MIDI.read();
    //m.shift_matrix();
    iReg.readShiftRegisters();
    t2 = millis();
    if (t2 - t1 > 100)
    {
        auto cresc_tmp = analog1.getOutputLevel(analogRead(CRESC));
        if (cresc_tmp > cresc)
        {
            //sendCrescLevel(cresc_tmp, true);
        }
        else if (cresc_tmp < cresc)
        {
            //sendCrescLevel(cresc_tmp, false);
        }
        cresc = cresc_tmp;
    }
}

void sendMessage(uint8_t midiNoteNumber, boolean on)
{
    if (on)
    {
        MIDI.sendNoteOn(midiNoteNumber, 0x7F, m._channel);
    }
    else
    {
        MIDI.sendNoteOff(midiNoteNumber, 0x00, m._channel);
    }
}
void sendCrescLevel(uint8_t level, boolean direction)
{
    uint8_t idx = level * 2; // analog is from 0-15 -> but we have 32 slots
    if (direction)
    {
        MIDI.sendNoteOn(cresc_mem[idx], 0x7F, 0x0A);
        for (uint8_t i = 0; i < 16; i++)
        {
            o.state |= (bitRead(cresc_mem[idx], i) << i);
        }
    }
    else
    {
        MIDI.sendNoteOff(cresc_mem[idx], 0x7F, 0x0A);
        for (uint8_t i = 0; i < 16; i++)
        {
            o.state |= (bitRead(cresc_mem[idx], i) << i);
        }
    }
}

void OnMidiNoteOn(byte channel, byte note, byte velocity)
{
    Logging("NoteOn:");
    Logging(channel);
    Logging(note);
    Loggingln(velocity);
    test++;
    Logging(" DropTest ");
    Logging(test);
    midiProcess(channel, note, true);
    //Echo
    //MIDI.sendNoteOn(note, velocity, channel);
}

void OnMidiNoteOff(byte channel, byte note, byte velocity)
{
    Logging("NoteOff:");
    Logging(channel);
    Logging(note);
    Loggingln(velocity);
    test++;
    midiProcess(channel, note, false);
    // Echo
    //MIDI.sendNoteOff(note, velocity, channel);
}

void OnMidiControlChange(byte control, byte value, byte channel)
{
    value = swellShades.getOutputLevel(value);
    switch (channel)
    {
    case 1:
        // control first manual swell shades
        break;
    case 2:
        // control second manual swell shades
        break;
    case 123:
        // AllNotesOff
        o.clear();
        break;
    default:
        break;
    }
}

void addCouplers(uint8_t channel, uint8_t note, boolean state)
{
    if (channel == 1 && first_octave_coupler)
    {
        o.setState(note + 12, state);
    }
    else if(channel == 1 && first_second_octave_coupler){
        // second.setState(note+ 12,state);
    }
        
}

void midiProcess(byte channel, byte note, boolean state)
{
    switch (channel)
    {
    case 1:
        // First manual
        o.setState(note, state);
        addCouplers(channel, note, state);
        break;
    case 2:
        // Second Manual
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        // 6-9 regiszterek (channel,note,state)
        break;

    case 10:
        // Pedal
        o.setState(note, state);
        break;
    case 11:
        // Setzer
    default:
        // Turn on notes on channel
        break;
    }
}