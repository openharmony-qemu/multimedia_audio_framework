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
*     降噪算法库接口    第一部分   宏定义、返回值与自定义结构体
*
******************************************************************************/
// =============================================================================
//     成功返回码
// =============================================================================
#define AUDIO_AINR_EOK   (0)          // 接口函数正常返回

// =============================================================================
//     接口返回错误码
// =============================================================================
#define AUDIO_AINR_ERR_BASE                                     (0)          // AHA错误码开始

// AudioAinrGetVersion 返回码
#define AUDIO_AINR_GETVERSION_INV_VERSION                    (AUDIO_AINR_ERR_BASE -  1)  // 无效的version地址
#define AUDIO_AINR_GETVERSION_INV_RELEASETIME                (AUDIO_AINR_ERR_BASE -  2)  // 无效的releaseTime地址
#define AUDIO_AINR_GETVERSION_4_BYTES_NOT_ALIGN_VERSION      (AUDIO_AINR_ERR_BASE -  3)  // GetVersion函数内地址非4字节对齐
#define AUDIO_AINR_GETVERSION_4_BYTES_NOT_ALIGN_RELEASETIME  (AUDIO_AINR_ERR_BASE -  4)  // GetVersion函数内地址非4字节对齐

// AudioAinrGetSize 返回码
#define AUDIO_AINR_GETSIZE_INV_CHANSIZE                      (AUDIO_AINR_ERR_BASE -  5)  // 通道变量长度指针为空
#define AUDIO_AINR_GETSIZE_INV_SCRATCHSIZE                   (AUDIO_AINR_ERR_BASE -  6)  // 缓冲区的长度指针为空
#define AUDIO_AINR_GETSIZE_4_BYTES_NOT_ALIGN_CHANSIZE        (AUDIO_AINR_ERR_BASE -  7)  // 通道变量首地址不是4字节对齐
#define AUDIO_AINR_GETSIZE_8_BYTES_NOT_ALIGN_SCRATCHSIZE     (AUDIO_AINR_ERR_BASE -  8)  // 缓冲区变量首地址不是8字节对齐

// AudioAinrInit 返回码
#define AUDIO_AINR_INIT_INV_HANDLE                           (AUDIO_AINR_ERR_BASE - 11)  // 空的句柄或指针
#define AUDIO_AINR_INIT_INV_CHANNELBUF                       (AUDIO_AINR_ERR_BASE - 12)  // 通道变量空间为空
#define AUDIO_AINR_INIT_ERR_BUFSIZE                          (AUDIO_AINR_ERR_BASE - 13)  // 通道变量空间太小
#define AUDIO_AINR_INIT_8_BYTES_NOT_ALIGN_HANDLE             (AUDIO_AINR_ERR_BASE - 14)  // 通道变量首地址不是8字节对齐
#define AUDIO_AINR_INIT_4_BYTES_NOT_ALIGN_CONFIG             (AUDIO_AINR_ERR_BASE - 15)  // 配置参数首地址不是4字节对齐
#define AUDIO_AINR_INIT_8_BYTES_NOT_ALIGN_SCRATCHBUF         (AUDIO_AINR_ERR_BASE - 16)  // 缓冲区变量首地址不是8字节对齐
#define AUDIO_AINR_INIT_INV_CONFIG                           (AUDIO_AINR_ERR_BASE - 17)  // 空的配置参数结构体
#define AUDIO_AINR_INIT_INV_SCRATCHBUF                       (AUDIO_AINR_ERR_BASE - 18)  // 缓冲区空间为空
#define AUDIO_AINR_INIT_INV_OBJMEMMANAGE                     (AUDIO_AINR_ERR_BASE - 19)  // 实际分配空间与GetSize的不一致

// AudioAinrApply 返回码
#define AUDIO_AINR_APPLY_INV_PAHADATA                        (AUDIO_AINR_ERR_BASE - 71)  // apply接口输入输出数据结构为空
#define AUDIO_AINR_APPLY_INV_PAHADATA_DATAIN                 (AUDIO_AINR_ERR_BASE - 72)  // apply接口输入输出数据结构为空
#define AUDIO_AINR_APPLY_INV_PAHADATA_DATAOUT                (AUDIO_AINR_ERR_BASE - 73)  // apply接口输入输出数据结构为空
#define AUDIO_AINR_APPLY_INV_PHANDLE                         (AUDIO_AINR_ERR_BASE - 74)  // apply接口输入通道指针为空
#define AUDIO_AINR_APPLY_INV_SCRATCHBUF                      (AUDIO_AINR_ERR_BASE - 75)  // apply接口输入缓冲区指针为空
#define AUDIO_AINR_APPLY_ERR_PROTECTFLAG                     (AUDIO_AINR_ERR_BASE - 76)  // 通道被踩
#define AUDIO_AINR_APPLY_4_BYTES_NOT_ALIGN_PAHADATA          (AUDIO_AINR_ERR_BASE - 77)  // 通道变量首地址不是4字节对齐
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_HANDLE            (AUDIO_AINR_ERR_BASE - 78)  // 通道变量首地址不是8字节对齐
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_SCRATCHBUF        (AUDIO_AINR_ERR_BASE - 79)  // 通道变量首地址不是8字节对齐
#define AUDIO_AINR_APPLY_UNINITIED                           (AUDIO_AINR_ERR_BASE - 80)  // 调用次序错误，未初始化
#define AUDIO_AINR_COMMON_ERR_PROTECTFLAG                    (AUDIO_AINR_ERR_BASE - 81)  // common通道被踩
#define AUDIO_AINR_AINR_ERR_PROTECTFLAG                      (AUDIO_AINR_ERR_BASE - 82)  // common通道被踩
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_DATAOUT           (AUDIO_AINR_ERR_BASE - 83)  // dataout不是8字节对齐
#define AUDIO_AINR_APPLY_8_BYTES_NOT_ALIGN_DATAIN            (AUDIO_AINR_ERR_BASE - 84)  // datain不是8字节对齐
#define AUDIO_AINR_APPLY_ERR_FRAMELAP                        (AUDIO_AINR_ERR_BASE - 85)  // frameLap不正确
// ==================================================================================================================//


/******************************************************************************
*
*                    第二部分 结构体定义
*
******************************************************************************/
// ============================================================================
// 初始化配置结构体定义
// ============================================================================

/* 算法层从上层获得的设备类信息结构体 */
typedef struct {
    signed short sampleRate;                       // 采样率，仅支持16k; 范围:{1}; 默认值:1; 错误码:-22;
    signed short channelNum;                       // 通道数量，仅支持单声道; 范围:{1}; 默认值:1; 错误码:-21;
    signed short frameLenth;                       // 每帧样点数，16K->160点; 范围:{160}; 默认值:160; 错误码:-23;
    signed short bitNum;                           // pcm输入位宽，仅支持16bit; 范围:{16}; 默认值:16; 错误码:-24;
    signed short sReserved[4];                     // 保留位; 范围:{0}; 默认值:0; 错误码:0;
} AudioAinrStruSysConfig, *AudioAinrPstSysConfig;

typedef struct {
    bool supportCurrdevice;
    bool supportRealtimeProc;
    signed short sampleRate;
    signed short bitdepth;
    signed short channelNum;
    signed short frameLenth;
} AudioAinrSpecStruct, *AudioAinrSpecPointer;

/* pcm数据采样率 */
#define AUDIO_AINR_PCM_SAMPLERATE_16K   (1)  // 算法支持采样率16000

/* pcm数据位宽格式 */
#define AUDIO_AINR_PCM_16_BIT         (16)   // PCM signed 16 bits

/* pcm数据帧长 */
#define AUDIO_AINR_PCM_16K_FRAME_LEN  (160)  // 输入帧长，160个采样点

/* pcm数据声道数 */
#define AUDIO_AINR_PCM_CHANNEL_NUM    (1)    // 单声道

// ============================================================================
// 函数调度接口参数结构体
// ============================================================================

/* 主调算法输入输出数据流结构体定义 */
typedef struct {
    signed short *dataIn;    // 输入数据流，1声道
    signed short *pad0;
    signed short *dataOut;   // 输出数据流，1声道
    signed short *pad1;

    float *freqDomainOut;
    float *freqDomainMic;
    float *ainrGainOut;
    float pad2;
} AudioAinrDataTransferStruct, *AudioAinrDataTransferPointer;

/*******************************************************************************
*
*  第三部分 主调度 -- 函数声明
*
*******************************************************************************/
// =============================================================================
// 函数名称  : AudioAinrGetVersion
// 功能描述  : 获取版本信息
// 输入参数  : version    --  算法版本
//            releaseTime  -- 算法发布时间
// 输出参数  : 无
// 返 回 值  : AUDIO_AINR_EOK表示成功， 其他返回码表示失败
// =============================================================================
signed int AudioAinrGetVersion(unsigned int *version, unsigned int *releaseTime);

// =============================================================================
// 函数名称  : AudioAinrGetSize
// 功能描述  : 获取通道大小
// 输入参数  : chanSize变量地址
// 输出参数  : chanSize  通道变量大小，对应handle和bufSize
// 返 回 值  : AUDIO_AINR_EOK表示成功， 其他返回码表示失败
// =============================================================================
signed int AudioAinrGetSize(signed int *chanSize);

// =============================================================================
// 函数名称  : AudioAinrInit
// 功能描述  : 初始化算法实例（通道变量），并返回其句柄
// 输入参数  : handle   -- 对象句柄
//            config   -- 系统配置参数
//            bufSize  -- 通道变量大小
// 输出参数  : 无
// 返 回 值  : AUDIO_AINR_EOK表示成功， 其他返回码表示失败
// =============================================================================
signed int AudioAinrInit(signed char *handle, AudioAinrPstSysConfig config, unsigned int bufSize);


// =============================================================================
// 函数名称  : AudioAinrApply
// 功能描述  : 算法处理主函数
// 输入参数  : handle    -- 算法实例句柄
//            pAhaData  -- 输入输出数据结构体
// 输出参数  : pAhaData的dataOut指针保存输出buff
// 返 回 值  : AUDIO_AINR_EOK表示成功， 其他返回码表示失败
// =============================================================================
signed int AudioAinrApply(signed char *handle, AudioAinrDataTransferPointer pAhaData);

/******************************************************************************
******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif