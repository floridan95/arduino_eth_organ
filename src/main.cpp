#define DEBUG

#include <Arduino.h>
#include "main.h"
#include "output.h"
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
IPMIDI_CREATE_DEFAULT_INSTANCE();

int t1, t2;
HystFilter analog1(1024, 16, 5);
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

    Serial.begin(115200);
    while (!Serial)
    {
        ; // Waiting for serial connection
    }

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

    t1 = millis();
    pinMode(A0, INPUT);

    Loggingln("Starting Loop");
}

void loop()
{
    MIDI.read();
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

void sendCrescLevel(uint8_t level, boolean direction)
{
    uint8_t idx = level % 2;
    if (direction)
    {
        MIDI.sendNoteOn(cresc_mem[idx], 0x7F, 0x0A);
        MIDI.sendNoteOn(cresc_mem[idx + 1], 0x7F, 0x0A);
    }
    else
    {
        MIDI.sendNoteOff(cresc_mem[idx], 0x7F, 0x0A);
        MIDI.sendNoteOff(cresc_mem[idx + 1], 0x7F, 0x0A);
    }
}

void OnMidiNoteOn(byte channel, byte note, byte velocity)
{
    Logging("NoteOn:");
    Logging(channel);
    Logging(note);
    Loggingln(velocity);
    switch (channel)
    {
    case 14:
        // Registers
        o.setState(note, true);
        break;
    case 1:
        o.setState(note-44, true);
        break;
    default:
        // Turn on notes on channel
        break;
    }
    //Echo
    //MIDI.sendNoteOn(note, velocity, channel);
}

void OnMidiNoteOff(byte channel, byte note, byte velocity)
{
    Logging("NoteOff:");
    Logging(channel);
    Logging(note);
    Loggingln(velocity);
    switch (channel)
    {
    case 14:
        // Registers
        o.setState(note, false);
        break;
    case 1:
        o.setState(note-44, false);
        break;
    default:
        // Turn on notes on channel
        break;
    }
    // Echo
    //MIDI.sendNoteOff(note, velocity, channel);
}