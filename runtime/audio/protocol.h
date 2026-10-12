#pragma once
/* Little-endian aligned DWORD wire contract, never process pointers. */
#define MNM_AUDIO_PAYLOAD (16u*1024u*1024u)
#define MNM_AUDIO_SIZE (128u+MNM_AUDIO_PAYLOAD)
#define MNM_AUDIO_VERSION 3u
#define MNM_AUDIO_CAP_NATIVE_WAVE 1u
#define MNM_AUDIO_WAVE_INSTALLED 0x10000u
#define MNM_AUDIO_FACTORY_NATIVE_ONLY 0x40000u
/* Header[31]: host capabilities in bits0..15; producer installation flags in
   bits16..23; saturating unsupported-local-COM count in bits24..31. */
enum MnmAudioOperation {MNM_AUDIO_PING=0,MNM_AUDIO_CREATE=1,MNM_AUDIO_UPLOAD=2,MNM_AUDIO_DUPLICATE=3,
 MNM_AUDIO_RELEASE=4,MNM_AUDIO_PLAY=5,MNM_AUDIO_STOP=6,MNM_AUDIO_RESET=7,MNM_AUDIO_VOLUME=8,MNM_AUDIO_PAN=9,MNM_AUDIO_STATUS=10,MNM_AUDIO_PRIMARY_VOLUME=11,MNM_AUDIO_PRIMARY_GET_VOLUME=12,
 MNM_AUDIO_PRIMARY_PLAY=13,MNM_AUDIO_PRIMARY_STOP=14,MNM_AUDIO_PRIMARY_STATUS=15,MNM_AUDIO_PRIMARY_FORMAT=16,
 MNM_AUDIO_LOAD_WAVE=17};
/* words: magic[0..1], version[2], size[3], request[4] (release publication),
 operation[5], voice[6], value/offset[7], byte count[8], PCM rate/channels/bits/
 alignment/byteRate/tag[9..14], creation flags[15]; response[16] (release),
 HRESULT[17], returned voice/status[18], heartbeat[19], ready[20].
 Adapter diagnostic counters: native device selections[21], pre-selection
 fallback attempts[22], failed submitted requests[23].
 LOAD_WAVE sends a bounded ASCII leaf .wav name (without NUL), returns PCM in
 the payload and byte count in value[18]. Response metadata: bytes[24], rate[25],
 byteRate[26], tag|channels<<16[27], alignment|bits<<16[28], durationMs[29],
 extra[30]. Host capabilities[31] are fixed before ready publication. All
 metadata/payload bytes precede response publication. Errors publish no PCM.
 One outstanding request per channel. On timeout the producer permanently retires
 the channel, so a late response can never be mistaken for a new command. */
