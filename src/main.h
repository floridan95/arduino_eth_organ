#ifdef DEBUG
#define Loggingln(msg) (Serial.println(msg));
#define Logging(msg) (Serial.print(msg));
#else
#define Logging(msg)
#define Loggingln(msg)
#endif

#define SET D42
#define CLEAR D43
#define PREV D44
#define NEXT D45
#define CRESC A0

void OnMidiNoteOn(byte channel, byte note, byte velocity);

void OnMidiNoteOff(byte channel, byte note, byte velocity);

void OnMidiControlChange(byte control, byte value, byte channel);

void sendCrescLevel(uint8_t level, boolean direction);

void sendMessage(uint8_t midiNoteNumber, boolean on);

void addCouplers(Output out, uint8_t note, boolean state);

void midiProcess(byte channel,byte note, boolean state);