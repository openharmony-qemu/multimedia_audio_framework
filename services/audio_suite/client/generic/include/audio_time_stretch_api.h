/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description: Public extern "C" API of libaudio_time_stretch.z.so, the
 * self-developed time-stretch (tempo) algorithm library. One .so, one API
 * family, two usage layers sharing one handle:
 *   1. Streaming layer (sonic-like contract, used by audio_speed / streaming
 *      consumers): write arbitrary frame counts, read processed frames back;
 *      input sizes are re-blocked internally so the caller never has to align
 *      its buffers.
 *   2. One-shot layer (PV-like contract, used by the audio suite through
 *      dlopen/dlsym): feed any input amount, get produced frames back in the
 *      same call, like PVChangeSpeed but float in / float out.
 *   Query layer (sonicSamplesAvailable / sonicGetSpeed counterparts):
 *      TimeStretchSamplesAvailable reports the unread buffered output frame
 *      count; TimeStretchGetSpeed reports the currently effective speed.
 * Notes: All counts are FRAMES (one frame = one sample per channel; N-channel
 * data is interleaved and N*frames floats long), no API parameter
 * carries a channel multiplier. Return convention: 0 = success, negative =
 * error code, positive = frame count; TimeStretchFlush reports its count
 * through an out-parameter. Speed is a playback tempo multiplier like
 * sonic/PV (output duration ~= input duration / speed), range [0.125, 8.0]:
 * speed 0.5 -> factor 2.0 (slowdown, phase vocoder path); speed 2.0 ->
 * factor 0.5 (speedup, WSOLA path). Pitch is a playback pitch multiplier
 * (TimeStretchSetPitch, range [0.25, 4.0]) applied on top of tempo: the
 * TSM stretch factor runs on the product stretch * pitch = (1/speed) * pitch,
 * whose domain is the full product of both ranges, i.e.
 * [MIN_SPEED*MIN_PITCH, MAX_SPEED*MAX_PITCH] = [0.03125, 32.0], and a
 * resampler removes the pitch component so the net tempo is exactly 1/speed.
 */

#ifndef AUDIO_TIME_STRETCH_API_H
#define AUDIO_TIME_STRETCH_API_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_STRETCH_SUCCESS            0
#define TIME_STRETCH_ERR_INVALID_PARAM  (-1)
#define TIME_STRETCH_ERR_INVALID_HANDLE (-2)
#define TIME_STRETCH_ERR_INVALID_SPEED  (-3)
#define TIME_STRETCH_ERR_INVALID_BUFFER (-4)
/* Reserved for ABI stability. The current no-gate implementation never refuses
 * a write because of unread output and is built with -fno-exceptions (an
 * allocation failure is process-fatal, not a returnable code), so no function
 * in this header returns this code. */
#define TIME_STRETCH_ERR_NO_MEMORY      (-5)
#define TIME_STRETCH_ERR_BUFFER_OVERFLOW (-6)

/* Supported speed range (stretch factor 1/speed is clamped inside [0.125, 8.0]). */
#define TIME_STRETCH_MIN_SPEED 0.125f
#define TIME_STRETCH_MAX_SPEED 8.0f

/* Supported pitch range (aligned with the framework stream pitch limits). */
#define TIME_STRETCH_MIN_PITCH 0.25f
#define TIME_STRETCH_MAX_PITCH 4.0f

/* Instance capability / status report (fields mirror AudioPVSpec). */
typedef struct {
    bool isSupport;         /* Always true in this build */
    bool isRealTime;        /* True: streaming layer supports real-time feed */
    unsigned int frameLen;  /* Internal processing block, in frames (sampleRate/50, 20ms) */
    unsigned int inSampleRate;  /* Sample rate of this instance */
    unsigned int inChannels;    /* Channel count of this instance */
    unsigned int inFormat;      /* 4:float - the only PCM data path; internal processing is float */
    unsigned int outSampleRate; /* Same as inSampleRate */
    unsigned int outChannels;   /* Same as inChannels */
    unsigned int outFormat;     /* Same as inFormat */
    unsigned int algorithmDelayFrames; /* Frames the algorithm buffers before first output */
    float minSpeed;             /* TIME_STRETCH_MIN_SPEED */
    float maxSpeed;             /* TIME_STRETCH_MAX_SPEED */
} AudioTimeStretchSpec;

struct TimeStretchStruct;
typedef struct TimeStretchStruct *TimeStretchHandle;

/* ==================== Streaming layer (sonic-like) ==================== */

/*
 * Create an instance.
 * sampleRate: [8000, 384000] Hz. channels: [1, 16].
 * Returns NULL on invalid parameter or out-of-memory. Initial speed is 1.0.
 */
TimeStretchHandle TimeStretchCreate(uint32_t sampleRate, uint32_t channels);

/* Destroy an instance created by TimeStretchCreate. NULL is tolerated. */
void TimeStretchDestroy(TimeStretchHandle handle);

/*
 * Set the playback speed. Takes effect from the next written frame; buffered
 * pending output keeps the old speed (change is applied on new input only).
 * speed must be within [TIME_STRETCH_MIN_SPEED, TIME_STRETCH_MAX_SPEED];
 * out-of-range is rejected with TIME_STRETCH_ERR_INVALID_SPEED (no clamping,
 * so the output/input ratio stays exactly as the caller expects). When a pitch
 * factor is active, stretch * pitch = (1/speed) * pitch must also stay inside
 * the factor range [MIN_SPEED*MIN_PITCH, MAX_SPEED*MAX_PITCH] = [0.03125,
 * 32.0]; a speed that would push the product out of range is rejected with
 * TIME_STRETCH_ERR_INVALID_SPEED and the previous speed is kept.
 */
int32_t TimeStretchSetSpeed(TimeStretchHandle handle, float speed);

/*
 * Set the playback pitch multiplier (sonic sonicSetPitch counterpart).
 * The value is validated (NaN and [TIME_STRETCH_MIN_PITCH,
 * TIME_STRETCH_MAX_PITCH]) and applied to the kernel: the TSM stretch factor
 * runs on stretch * pitch = (1/speed) * pitch (equivalently pitch/speed), and
 * this product must stay inside the kernel factor range
 * [MIN_SPEED*MIN_PITCH, MAX_SPEED*MAX_PITCH] = [0.03125, 32.0] - otherwise
 * the call is rejected with TIME_STRETCH_ERR_INVALID_PARAM. The
 * pitch-resampler step is updated so the net tempo stays exactly 1/speed
 * while the pitch shifts by `pitch` (2.0 = +12 semitones). Callers that pair
 * SetPitch with SetSpeed (audio_speed) get real pitch shift on top of
 * unchanged tempo. Out-of-range pitch is rejected (no clamping).
 */
int32_t TimeStretchSetPitch(TimeStretchHandle handle, float pitch);

/*
 * Write interleaved float input, ANY frame count (20 ms blocks or 100 s at
 * once - both are fed straight through block-by-block; the library keeps only
 * one block-sized residue internally regardless of numFrames).
 * MEMORY CONTRACT (no-gate, sonic-like): the write is ALWAYS accepted; the
 * library imposes no bound on unread output. Pending output grows naturally
 * with what the caller writes and does not read back (roughly numFrames/speed
 * per write, independent of pitch - the resampler cancels the pitch
 * component). BOUNDING MEMORY IS THE CALLER'S JOB: either drain with
 * TimeStretchReadFloat after each write (streaming consumers), or pass a
 * sufficient outCount to a one-shot call so the output is taken in the same
 * call. A caller that keeps writing without ever draining will exhaust the
 * process heap - that is the agreed contract, not an error (the module is
 * built without exceptions: an exhausted heap is fatal, there is no
 * returnable NO_MEMORY for it).
 */
int32_t TimeStretchWriteFloat(TimeStretchHandle handle, const float *data, int32_t numFrames);

/*
 * Read up to maxFrames processed interleaved float frames into data.
 * Returns the frame count actually read; 0 is legal and means "nothing
 * available yet" (algorithmic startup delay / absorption), not an error.
 * Repeated reads drain all pending output.
 */
int32_t TimeStretchReadFloat(TimeStretchHandle handle, float *data, int32_t maxFrames);

/*
 * Report how many processed frames are currently buffered and unread (the
 * sonicSamplesAvailable counterpart). The caller can use this to size its
 * output buffer before calling TimeStretchReadFloat, or to tell "still have
 * data" from "read returned 0 because the algorithmic startup delay has not
 * been crossed yet". Returns 0 when nothing is pending (or handle is NULL).
 * During streaming the readable total converges to (written frames)/speed
 * MINUS a fixed in-flight tail (the startup delay's dual: what leaves late is
 * exactly what arrived late) - only TimeStretchFlush pushes that tail out.
 * There is no internal cap on this value (see TimeStretchWriteFloat): it is
 * exactly what the caller has written but not read back.
 */
int32_t TimeStretchSamplesAvailable(TimeStretchHandle handle);

/*
 * Report the currently effective speed (the value last accepted by
 * TimeStretchSetSpeed; starts at 1.0). Returns 0.0f for a NULL handle.
 */
float TimeStretchGetSpeed(TimeStretchHandle handle);

/*
 * Report the currently effective pitch multiplier (the value last accepted by
 * TimeStretchSetPitch; starts at 1.0). Returns 1.0f for a NULL handle.
 */
float TimeStretchGetPitch(TimeStretchHandle handle);

/*
 * Discard all buffered input/output and restore the startup state (seek / stop).
 * Note: the phase-vocoder phase state is intentionally preserved, which keeps
 * timbre stable across a seek but may produce one slightly imperfect frame
 * after the first new block. Speed and pitch settings are kept.
 */
int32_t TimeStretchReset(TimeStretchHandle handle);

/*
 * End-of-stream flush, the sonicFlushStream counterpart. Feeds internal
 * silence so the buffered tail passes the processing threshold, keeps only
 * the output the real residual input can produce (about residual/speed
 * frames), then clears all buffered state so the instance is immediately
 * reusable. Frames pending BEFORE the flush stay first in line.
 * The count of newly available frames is written to *outFrames (may be
 * NULL); read them with TimeStretchReadFloat / the one-shot layer as usual.
 * Speed setting and the PV phase state are kept.
 */
int32_t TimeStretchFlush(TimeStretchHandle handle, int32_t *outFrames);

/* ==================== One-shot layer (PV-like) ==================== */

/*
 * One-call tempo change, float in / float out (drop-in shape of PVChangeSpeed
 * with float PCM; counts are FRAMES here, and multi-channel interleaved data
 * is supported).
 * Feeds inCount frames - ANY count, internally processed block-by-block with
 * only a block-sized internal residue (a 100 s call costs no full-length
 * buffer) - and pumps whatever is ready into dataOut as it goes.
 * Returns the frame count produced (0 is legal during the algorithmic startup
 * delay), or a negative error code.
 * If fewer frames are ready than outCount allows, the remainder stays buffered
 * and is returned by the next call - nothing is dropped. With a sufficient
 * outCount (>= inCount/speed) the output streams straight through into
 * dataOut in this call (minus the fixed in-flight tail; TimeStretchFlush
 * pushes the tail out at end of stream).
 * outCount=0 is a pure write: all input is consumed, output waits in the
 * internal buffer. MEMORY CONTRACT: the library does not bound that buffer -
 * callers that do not drain must manage memory themselves (see
 * TimeStretchWriteFloat).
 */
int32_t TimeStretchChangeSpeedFloat(TimeStretchHandle handle, const float *dataIn, float *dataOut,
    int32_t inCount, int32_t outCount);

/* ==================== Capability query ==================== */

/*
 * Report the configuration and status of one instance.
 * A NULL handle yields a zeroed struct (isSupport == false).
 */
AudioTimeStretchSpec TimeStretchGetSpec(TimeStretchHandle handle);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TIME_STRETCH_API_H */
