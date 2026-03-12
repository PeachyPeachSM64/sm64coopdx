#ifndef _ULTRA64_LIBAUDIO_H_
#define _ULTRA64_LIBAUDIO_H_

#include "abi.h"

typedef struct
{
    u8 *offset;
    s32 len;
#if 0
    s8 magic[2]; // tbl: 0x0204, otherwise: 0x0203

    // for ctl (else zeros):
    union {
        // unused, just for clarification (big endian)
        struct {
            u8 bank;
            u8 ff;
            u8 numInstruments;
            u8 numDrums;
        } as_u8;

        // used
        struct {
            s16 bankAndFf;
            s16 numInstrumentsAndDrums;
        } as_s16;
    } ctl;
#endif
} ALSeqData;

typedef struct
{
#if 1
    s16 revision;
#endif
    s16 seqCount;
#if 0
    s16 unk2;
    u8 *data;
    s32 pad[2];
#endif
    ALSeqData seqArray[1];
} ALSeqFile;

void alSeqFileNew(ALSeqFile *f, u8 *base);

#endif
