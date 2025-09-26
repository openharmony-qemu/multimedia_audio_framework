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
 
#ifndef __IMEDIA_API_H__
#define __IMEDIA_API_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_IMEDIA_INNER_API

// 8位数据类型重定义
typedef unsigned char IMEDIA_UINT8;
typedef signed char IMEDIA_INT8;
typedef char IMEDIA_CHAR;  // 8位数据类型重定义，与uniDSP头文件兼容

// 16位
typedef unsigned short IMEDIA_UINT16;
typedef signed short IMEDIA_INT16;

// 32位
typedef unsigned int IMEDIA_UINT32;
typedef signed int IMEDIA_INT32;
typedef unsigned int IMEDIA_BOOL;  // 32位数据类型重定义，与uniDSP头文件兼容

#ifndef IMEDIA_VOID
#define IMEDIA_VOID void
#endif

// 算法处理帧长
#define IMEDIA_SWS_FRAME_LEN (480)

// 错误码定义
#define IMEDIA_SWS_EOK (0)  // 正常

#define IMEDIA_SWS_EQ_BANDS (12)  // PEQ模块最大频带数
#define IMEDIA_SWS_VERLEN (64)    // 算法库版本信息及发布时间字符串长度

#define COEFFICIENT 10
#define OFFSET 100
#define EQ_BANDS 10
#define CHANNEL 2
#define ONE_BYTE_OFFSET 8
#define LBA_OFFSET 14

// 内存尺寸结构体
typedef struct tagSTRU_IMEDIA_SWS_MEM_SIZE {
    IMEDIA_INT32 iStrSize;      // 通道大小
    IMEDIA_INT32 iScracthSize;  // ScratchBuf大小
    IMEDIA_INT32 iReserve[4];   // 保留区
} iMedia_SWS_MEM_SIZE;

// 数据结构体
typedef struct tagSTRU_IMEDIA_SWS_DATA {
    IMEDIA_INT32 *piDataIn;       // 输入数据地址
    IMEDIA_INT32 *piDataOut;      // 输出数据地址
    IMEDIA_INT32 iSize;           // 输入数据长度
    IMEDIA_INT32 iEnable_SWS;     // SWS开关标识
    IMEDIA_INT32 iData_Format16;  // 1:16bit 0:24bit 2:32bit
    IMEDIA_INT32 iMasterVolume;   // 主体按键音量增益
    IMEDIA_INT32 iData_Channel;   // 输入音源声道数
    IMEDIA_INT32 iData_Reserve;   // 8字节对齐
} iMedia_SWS_DATA;

// 版本结构体
typedef struct tagSTRU_IMEDIA_SWS_VERSION {
    IMEDIA_INT8 ucCgtVersion[IMEDIA_SWS_VERLEN];   // 编译器版本号
    IMEDIA_INT8 ucReleaseVer[IMEDIA_SWS_VERLEN];   // 算法库版本号
    IMEDIA_INT8 ucReleaseTime[IMEDIA_SWS_VERLEN];  // 发布日期
} iMedia_SWS_STRU_VERSION, *iMedia_SWS_PST_VERSION;
#endif

typedef struct tagSTRU_IMEDIA_Eq_PARA {
    IMEDIA_INT16 sFrameLen;  // 帧长，480；
    IMEDIA_INT16 sEQLRBands;
    IMEDIA_INT16 sEQLRType[IMEDIA_SWS_EQ_BANDS];
    IMEDIA_INT16 sEQLRGain[IMEDIA_SWS_EQ_BANDS];
} iMedia_Eq_PARA;

extern IMEDIA_INT32 iMedia_Eq_GetSize(iMedia_SWS_MEM_SIZE *pMemSize);

extern IMEDIA_INT32 iMedia_Eq_Init(
    IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf, IMEDIA_INT32 iScratchBufLen, const iMedia_Eq_PARA *pParams);

extern IMEDIA_INT32 iMedia_Eq_Apply(
    IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf, IMEDIA_INT32 iScratchBufLen, iMedia_SWS_DATA *pData);

extern IMEDIA_INT32 iMedia_Eq_SetParams(
    IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf, IMEDIA_INT32 iScratchBufLen, const iMedia_Eq_PARA *pParams);

extern IMEDIA_INT32 iMedia_Eq_GetParams(IMEDIA_VOID *pHandle, iMedia_Eq_PARA *pParams);

extern IMEDIA_INT32 iMedia_Eq_GetVersion(iMedia_SWS_PST_VERSION *ppVersion);

#ifdef __cplusplus
}
#endif

#endif