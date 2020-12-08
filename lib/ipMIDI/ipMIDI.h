#pragma once

#include <MIDI.h>
using namespace MIDI_NAMESPACE;

#include "ipMIDI_Namespace.h"

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

BEGIN_IPMIDI_NAMESPACE

template <class UdpClass>
class ipMidiTransport
{
    friend class MIDI_NAMESPACE::MidiInterface<ipMidiTransport<UdpClass>>;

public:
    ipMidiTransport(uint16_t port = 21928, boolean multicast = false)
        : port_(port), multicast_(multicast)
    {
        uint8_t reply[4] = {225, 0, 0, 37};
        setReply(reply);
    };

    void begin()
    {
        // if we were called very soon after the board was booted, we need to give the
        // EthernetShield (WIZnet) some time to come up. Hence, we delay until millis() is at
        // least 3000. This is necessary, so that if we need to add a service record directly
        // after begin, the announce packet does not get lost in the bowels of the WIZnet chip.
        while (millis() < 3000)
            delay(100);
        if (multicast_)
        {
            dataPort_.beginMulticast(reply_, port_);
        }
        else
        {
            dataPort_.begin(port_);
        }
    }

    void setReply(uint8_t value[4])
    {
        reply_[0] = value[0];
        reply_[1] = value[1];
        reply_[2] = value[2];
        reply_[3] = value[3];
    }

protected:
    static const bool thruActivated = false;

    bool beginTransmission(MidiType)
    {
        return dataPort_.beginPacket(reply_, port_);
    };

    void write(byte byte)
    {
        dataPort_.write(byte);
    };

    void endTransmission()
    {
        dataPort_.endPacket();
    };

    byte read()
    {
        return packetBuffer_[packetAmountRead_ - packetBufferIndex_];
    };

    unsigned available()
    {
        if (packetBufferIndex_ > 0) // empty buffer first, before reading new data
            packetBufferIndex_--;
        else
        {
            auto packetSize = dataPort_.parsePacket();
            if (0 == packetSize)
                return 0; // if nothing is available, leave here
            // data is ready to be read, do not read more than what we have memory for
            packetBufferIndex_ = packetAmountRead_ = dataPort_.read(packetBuffer_, packetSize);
        }
        return packetBufferIndex_;
    };

private:
    byte packetBuffer_[UDP_TX_PACKET_MAX_SIZE];
    uint16_t packetBufferIndex_;
    uint16_t packetAmountRead_;

    UdpClass dataPort_;

    uint16_t port_;
    boolean multicast_;
    uint8_t reply_[4];
};

#define IPMIDI_CREATE_INSTANCE(Type, Name, Port, Multicast)            \
    IPMIDI_NAMESPACE::ipMidiTransport<Type> ip##Name(Port, Multicast); \
    MIDI_NAMESPACE::MidiInterface<IPMIDI_NAMESPACE::ipMidiTransport<Type>> Name((IPMIDI_NAMESPACE::ipMidiTransport<Type> &)ip##Name);

#define IPMIDI_CREATE_DEFAULT_INSTANCE() \
    IPMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, 21928, false);

#define IPMIDI_CREATE_MULTICAST_INSTANCE() \
    IPMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, 21928, true);

END_IPMIDI_NAMESPACE
