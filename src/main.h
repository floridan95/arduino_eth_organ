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

void sendCrescLevel(uint8_t level, boolean direction);

void midiProcess(byte channel,byte note, boolean state);