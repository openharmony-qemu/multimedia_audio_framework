/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 */

#ifndef AUDIO_HMS_AINR_API_H
#define AUDIO_HMS_AINR_API_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
*
*     Noise Reduction Algorithm Library API    Part 1: Macros, Return Values, and Custom Structures
*
******************************************************************************/
// =============================================================================
//      Success Return Codes
// =============================================================================
#define AUDIO_AINR_EOK   (0)          // Function returns normally

// =============================================================================
//      Error Return Codes
// =============================================================================
#define AUDIO_AINR_ERR_BASE                                     (0)          // AHA error code base

// AudioAinrGetVersion return codes
#define AUDIO_AINR_GETVERSION_INV_VERSION                    (AUDIO_AINR_ERR_BASE -  1)  // Invalid version address
#define AUDIO_AINR_GETVERSION_INV_RELEASETIME                (AUDIO_AINR_ERR_BASE -  2)  // Invalid releaseTime address
#define AUDIO_AINR_GETVERSION_4_BYTES_NOT_ALIGN_VERSION      (AUDIO_AINR_ERR_BASE -  3)  // Address not 4-byte aligned in GetVersion (version)
#define AUDIO_AINR_GETVERSION_4_BYTES_NOT_ALIGN_RELEASETIME  (AUDIO_AINR_ERR_BASE -  4)  // Address not 4-byte aligned in GetVersion (releaseTime)

// AudioAinrGetSize return codes
#define AUDIO_AINR_GETSIZE_INV_CHANSIZE                      (AUDIO_AINR_ERR_BASE -  5)  // Null pointer for channel buffer size
#define AUDIO_AINR_GETSIZE_INV_SCRATCHSIZE                   (AUDIO_AINR_ERR_BASE -  6)  // Null pointer for scratch buffer size
#define AUDIO_AINR_GETSIZE_4_BYTES_NOT_ALIGN_CHANSIZE        (AUDIO_AINR_ERR_BASE -  7)  // Channel buffer start address not 4-byte aligned
#define AUDIO_AINR_GETSIZE_8_BYTES_NOT_ALIGN_SCRATCHSIZE     (AUDIO_AINR_ERR_BASE -  8)  // Scratch buffer start address not 8-byte aligned

// AudioAinrInit return codes
#define AUDIO_AINR_INIT_INV_HANDLE                           (AUDIO_AINR_ERR_BASE - 11)  // Null handle or pointer
#define AUDIO_AINR_INIT_INV_CHANNELBUF                       (AUDIO_AINR_ERR_BASE - 12)  // Null channel buffer space
#define AUDIO_AINR_INIT_ERR_BUFSIZE                          (AUDIO_AINR_ERR_BASE - 13)  // Channel buffer space too small
#define AUDIO_AINR_INIT_8_BYTES_NOT_ALIGN_HANDLE             (AUDIO_AINR_ERR_BASE - 14)  // Channel variable start address not 8-byte aligned
#define AUDIO_AINR_INIT_4_BYTES_NOT_ALIGN_CONFIG             (AUDIO_AINR_ERR_BASE - 15)  // Configuration parameter start address not 4-byte aligned
#define AUDIO_AINR_INIT_8_BYTES_NOT_ALIGN_SCRATCHBUF         (AUDIO_AINR_ERR_BASE - 16)  // Scratch buffer start address not 8-byte aligned
#define AUDIO_AINR_INIT_INV_CONFIG                           (AUDIO_AINR_ERR_BASE - 17)  // Null configuration parameter structure
#define AUDIO_AINR_INIT_INV_SCRATCHBUF                       (AUDIO_AINR_ERR_BASE - 18)  // Null scratch buffer space
#define AUDIO_AINR_INIT_INV_OBJMEMMANAGE                     (AUDIO_AINR_ERR_BASE - 19)  // Actual allocated space differs from GetSize

// AudioAinrApply return codes
#define AUDIO_AINR_APPLY_INV_PAHADATA                        (AUDIO_AINR_ERR_BASE - 71)  // Null input/output data structure in apply
#define AUDIO_AINR_APPLY_INV_PAHADATA_DATAIN                 (AUDIO_AINR_ERR_BASE - 72)  // Null input data pointer in apply
#define AUDIO_AINR_APPLY_INV_PAHADATA_DATAOUT                (AUDIO_AINR_ERR_BASE - 73)  // Null output data pointer in apply
#define AUDIO_AINR_APPLY_INV_PHANDLE                         (AUDIO_AINR_ERR_BASE - 74)  // Null channel handle in apply
#define AUDIO_AINR_APPLY_INV_SCRATCHBUF                      (AUDIO_AINR_ERR_BASE - 75)  // Null scratch buffer pointer in apply
#define AUDIO_AINR_APPLY_ERR_PROTECTFLAG                     (AUDIO_AINR_ERR_BASE - 76)  // Channel corruption detected
#define AUDIO_AINR_APPLY_4_BYTES_NOT_ALIGN_PAHADATA          (AUDIO_AINR_ERR_BASE - 77)  // Data structure start address not 4-byte aligned
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_HANDLE            (AUDIO_AINR_ERR_BASE - 78)  // Handle address not 8-byte aligned
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_SCRATCHBUF        (AUDIO_AINR_ERR_BASE - 79)  // Scratch buffer address not 8-byte aligned
#define AUDIO_AINR_APPLY_UNINITIED                           (AUDIO_AINR_ERR_BASE - 80)  // Call order error (uninitialized)
#define AUDIO_AINR_COMMON_ERR_PROTECTFLAG                    (AUDIO_AINR_ERR_BASE - 81)  // Common channel corruption
#define AUDIO_AINR_AINR_ERR_PROTECTFLAG                      (AUDIO_AINR_ERR_BASE - 82)  // Common channel corruption
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_DATAOUT           (AUDIO_AINR_ERR_BASE - 83)  // dataOut not 8-byte aligned
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_DATAIN            (AUDIO_AINR_ERR_BASE - 84)  // dataIn not 8-byte aligned
#define AUDIO_AINR_APPLY_ERR_FRAMELAP                        (AUDIO_AINR_ERR_BASE - 85)  // Incorrect frameLap
// ==================================================================================================================//


/******************************************************************************
*
*                    Part 2: Structure Definitions
*
******************************************************************************/
// ============================================================================
// Initialization Configuration Structure Definition
// ============================================================================

/* Device information structure passed from upper layer to algorithm */
typedef struct {
    signed short sampleRate;                       // Sample rate, supports 16k only; Range:{1}; Default:1; Error Code:-22;
    signed short channelNum;                       // Number of channels, supports mono only; Range:{1}; Default:1; Error Code:-21;
    signed short frameLenth;                       // Samples per frame, 16K->160 samples; Range:{160}; Default:160; Error Code:-23;
    signed short bitNum;                           // PCM input bit depth, supports 16bit only; Range:{16}; Default:16; Error Code:-24;
    signed short sReserved[4];                     // Reserved; Range:{0}; Default:0; Error Code:0;
} AudioAinrStruSysConfig, *AudioAinrPstSysConfig;

typedef struct {
    bool supportCurrdevice;                        // Supports current device
    bool supportRealtimeProc;                      // Supports real-time processing
    signed short sampleRate;                       // Sample rate
    signed short bitdepth;                         // Bit depth
    signed short channelNum;                       // Number of channels
    signed short frameLenth;                       // Frame length
} AudioAinrSpecStruct, *AudioAinrSpecPointer;

/* PCM Data Sample Rates */
#define AUDIO_AINR_PCM_SAMPLERATE_16K   (1)  // Supported sample rate: 16000

/* PCM Data Bit Depth Formats */
#define AUDIO_AINR_PCM_16_BIT         (16)   // PCM signed 16 bits

/* PCM Data Frame Length */
#define AUDIO_AINR_PCM_16K_FRAME_LEN  (160)  // Input frame length: 160 samples

/* PCM Data Channel Count */
#define AUDIO_AINR_PCM_CHANNEL_NUM    (1)    // Mono channel

// ============================================================================
// Function Dispatch Interface Parameter Structure
// ============================================================================

/* Main algorithm input/output data stream structure definition */
typedef struct {
    signed short *dataIn;    // Input data stream (mono)
    signed short *pad0;      // Padding
    signed short *dataOut;   // Output data stream (mono)
    signed short *pad1;      // Padding

    float *freqDomainOut;    // Frequency domain output
    float *freqDomainMic;    // Microphone frequency domain data
    float *ainrGainOut;      // AINR gain output
    float pad2;              // Padding
} AudioAinrDataTransferStruct, *AudioAinrDataTransferPointer;

/*******************************************************************************
*
*  Part 3: Main Dispatch -- Function Declarations
*
*******************************************************************************/
// =============================================================================
// Function Name : AudioAinrGetVersion
// Description   : Gets version information
// Input Params  : version     -- Algorithm version
//                 releaseTime -- Algorithm release time
// Output Params : None
// Return Value  : AUDIO_AINR_EOK on success, other codes indicate failure
// =============================================================================
signed int AudioAinrGetVersion(unsigned int *version, unsigned int *releaseTime);

// =============================================================================
// Function Name : AudioAinrGetSize
// Description   : Gets the channel buffer size
// Input Params  : chanSize variable address
// Output Params : chanSize  -- Channel buffer size (corresponds to handle and bufSize)
// Return Value  : AUDIO_AINR_EOK on success, other codes indicate failure
// =============================================================================
signed int AudioAinrGetSize(signed int *chanSize);

// =============================================================================
// Function Name : AudioAinrInit
// Description   : Initializes an algorithm instance (channel variables) and returns its handle
// Input Params  : handle   -- Object handle
//                 config   -- System configuration parameters
//                 bufSize  -- Channel buffer size
// Output Params : None
// Return Value  : AUDIO_AINR_EOK on success, other codes indicate failure
// =============================================================================
signed int AudioAinrInit(signed char *handle, AudioAinrPstSysConfig config, unsigned int bufSize);


// =============================================================================
// Function Name : AudioAinrApply
// Description   : Main algorithm processing function
// Input Params  : handle     -- Algorithm instance handle
//                 pAhaData   -- Input/output data structure
// Output Params : pAhaData's dataOut pointer holds the output buffer
// Return Value  : AUDIO_AINR_EOK on success, other codes indicate failure
// =============================================================================
signed int AudioAinrApply(signed char *handle, AudioAinrDataTransferPointer pAhaData);

/******************************************************************************
******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif