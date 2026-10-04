#pragma once
/* Little-endian aligned DWORD wire contract, never process pointers. */
#define MNM_AUDIO_PAYLOAD (16u*1024u*1024u)
#define MNM_AUDIO_SIZE (128u+MNM_AUDIO_PAYLOAD)
#define MNM_AUDIO_VERSION 1u
enum MnmAudioOperation {MNM_AUDIO_PING=0,MNM_AUDIO_CREATE=1,MNM_AUDIO_UPLOAD=2,MNM_AUDIO_DUPLICATE=3,
 MNM_AUDIO_RELEASE=4,MNM_AUDIO_PLAY=5,MNM_AUDIO_STOP=6,MNM_AUDIO_RESET=7,MNM_AUDIO_VOLUME=8,MNM_AUDIO_PAN=9,MNM_AUDIO_STATUS=10};
/* words: magic[0..1], version[2], size[3], request[4] (release publication),
 operation[5], voice[6], value/offset[7], byte count[8], PCM rate/channels/bits/
 alignment/byteRate/tag[9..14], creation flags[15]; response[16] (release),
 HRESULT[17], returned voice/status[18], heartbeat[19], ready[20].
 Adapter diagnostic counters: native device selections[21], pre-selection
 fallback attempts[22], failed submitted requests[23].
 One outstanding request per channel. On timeout the producer permanently retires
 the channel, so a late response can never be mistaken for a new command. */
