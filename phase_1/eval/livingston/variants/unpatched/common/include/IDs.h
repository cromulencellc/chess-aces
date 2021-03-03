#ifndef IDS_H
#define IDS_H
#include <stdint.h>
typedef uint8_t typeID;
typedef uint8_t waveID;
typedef uint8_t effectID;
#define EFFECT_C ((typeID)0x00)
#define EFFECT_I (0x00)
#define TRACK_C ((typeID)0x01)
#define TRACK_I (0x01)
#define SIN_C ((waveID)0x00)
#define SIN_I (0x00)
#define SQU_C ((waveID)0x01)
#define SQU_I (0x01)
#define TRI_C ((waveID)0x02)
#define TRI_I (0x02)
#define SAW_C ((waveID)0x03)
#define SAW_I (0x03)
#define WHI_C ((waveID)0x04)
#define WHI_I (0x04)
#define CHORUS_C ((effectID)0x00)
#define CHORUS_I (0x00)
#define COMPRESS_C ((effectID)0x01)
#define COMPRESS_I (0x01)
#define DELAY_C ((effectID)0x02)
#define DELAY_I (0x02)
#define ENVELOPE_C ((effectID)0x03)
#define ENVELOPE_I (0x03)
#define EQ_C ((effectID)0x04)
#define EQ_I (0x04)
#define TREMOLO_C ((effectID)0x05)
#define TREMOLO_I (0x05)
#endif