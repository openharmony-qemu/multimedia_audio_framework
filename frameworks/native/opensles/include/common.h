/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef COMMON_H
#define COMMON_H

#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>

#include <iostream>
#include <cstdlib>
#include <stddef.h>

#include "audio_common_log.h"
#include "audioplayer_adapter.h"
#include "audiocapturer_adapter.h"

struct CEngine;

struct ClassTable;

/** itf struct **/

struct IObject {
    const struct SLObjectItf_ *mItf;
    CEngine *mEngine;
    const ClassTable *mClass;
    SLuint8 mState;
};

struct IEngine {
    const struct SLEngineItf_ *mItf;
    IObject *mThis;
};

struct IPlay {
    const struct SLPlayItf_ *mItf;
    SLuint32 mState;
    SLuint8 mId;
};

struct IRecord {
    const struct SLRecordItf_ *mItf;
    SLuint32 mState;
    SLuint8 mId;
};

struct IOHBufferQueue {
    const struct SLOHBufferQueueItf_ *mItf;
    SLuint32 mState;
    SLInterfaceID mIid;
    SLuint8 mId;
};

struct IVolume {
    const struct SLVolumeItf_ *mItf;
    SLuint8 mId;
};

/** class struct **/

struct CEngine {
    IObject mObject;
    IEngine mEngine;
};

struct CAudioPlayer {
    IObject mObject;
    IPlay mPlay;
    IVolume mVolume;
    IOHBufferQueue mBufferQueue;
    SLuint32 mId;
};

struct CAudioRecorder {
    IObject mObject;
    IRecord mRecord;
    IOHBufferQueue mBufferQueue;
    SLuint32 mId;
};

struct COutputMix {
    IObject mObject;
};

struct ClassTable {
    SLuint32 mObjectId;
    size_t mSize;
};

extern ClassTable EngineTab;

extern ClassTable AudioPlayerTab;

extern ClassTable AudioRecorderTab;

extern ClassTable OutputMixTab;

ClassTable *ObjectIdToClass(SLuint32 objectId);

IObject *Construct(const ClassTable *classTable, SLEngineItf itf);

void IOHBufferQueueInit(void *self, const SLInterfaceID iid, SLuint32 id);

void IEngineInit(void *self);

void IObjectInit(void *self);

void IPlayInit(void *self, SLuint32 id);

void IVolumeInit(void *self, SLuint32 id);

void IRecordInit(void *self, SLuint32 id);

SLresult EngineDestory(void* self);

SLresult AudioPlayerDestroy(void* self);

SLresult OutputMixDestroy(void* self);

SLresult AudioRecorderDestroy(void *self);
#endif
