/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <common.h>

using namespace OHOS::AudioStandard;

static SLuint32 audioPlayerId = 0;
static SLuint32 audioRecorderId = 0;
static std::mutex playerIdMutex;

static SLresult CreateLEDDevice(
    SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateVibraDevice(
    SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
static SLresult CreateAudioPlayer(
    SLEngineItf self, SLObjectItf *pPlayer, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    if (pPlayer == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    ClassTable *audioPlayerClass = ObjectIdToClass(SL_OBJECTID_AUDIOPLAYER);
    CAudioPlayer *thiz = (CAudioPlayer *) Construct(audioPlayerClass, self);
    if (thiz == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    std::unique_lock<std::mutex> lock(playerIdMutex);
    thiz->mId = audioPlayerId++;
    lock.unlock();
    IObjectInit(&thiz->mObject);
    IPlayInit(&thiz->mPlay, audioPlayerId);
    IVolumeInit(&thiz->mVolume, audioPlayerId);
    IOHBufferQueueInit(&thiz->mBufferQueue, SL_IID_PLAY, audioPlayerId);
    *pPlayer = &thiz->mObject.mItf;
    SLresult ret = AudioPlayerAdapter::GetInstance()->
        CreateAudioPlayerAdapter(audioPlayerId, pAudioSrc, pAudioSnk, OHOS::AudioStandard::STREAM_MUSIC);
    if (ret != SL_RESULT_SUCCESS) {
        return SL_RESULT_RESOURCE_ERROR;
    }

    return SL_RESULT_SUCCESS;
}

static SLresult CreateAudioRecorder(
    SLEngineItf self, SLObjectItf *pRecorder, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    if (pRecorder == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    ClassTable *audioRecorderClass = ObjectIdToClass(SL_OBJECTID_AUDIORECORDER);
    CAudioRecorder *thiz = (CAudioRecorder *) Construct(audioRecorderClass, self);
    if (thiz == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    thiz->mId = audioRecorderId;
    IObjectInit(&thiz->mObject);
    IRecordInit(&thiz->mRecord, audioRecorderId);
    IOHBufferQueueInit(&thiz->mBufferQueue, SL_IID_RECORD, audioRecorderId);
    *pRecorder = &thiz->mObject.mItf;
    SLresult ret = AudioCapturerAdapter::GetInstance()->
        CreateAudioCapturerAdapter(audioRecorderId, pAudioSrc, pAudioSnk, OHOS::AudioStandard::STREAM_MUSIC);
    if (ret != SL_RESULT_SUCCESS) {
        return SL_RESULT_RESOURCE_ERROR;
    }
    audioRecorderId++;

    return SL_RESULT_SUCCESS;
}

static SLresult CreateMidiPlayer(
    SLEngineItf self, SLObjectItf *pPlayer, SLDataSource *pMIDISrc, SLDataSource *pBankSrc, SLDataSink *pAudioOutput,
    SLDataSink *pVibra, SLDataSink *pLEDArray, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateListener(
    SLEngineItf self, SLObjectItf *pListener, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult Create3DGroup(
    SLEngineItf self, SLObjectItf *pGroup, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateOutputMix(
    SLEngineItf self, SLObjectItf *pMix, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    if (pMix == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    ClassTable *outputMixClass = ObjectIdToClass(SL_OBJECTID_OUTPUTMIX);
    COutputMix *thiz = (COutputMix *) Construct(outputMixClass, self);
    if (thiz == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IObjectInit(&thiz->mObject);
    *pMix = &thiz->mObject.mItf;
    return SL_RESULT_SUCCESS;
}

static SLresult CreateMetadataExtractor(
    SLEngineItf self, SLObjectItf *pMetadataExtractor, SLDataSource *pDataSource, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateExtensionObject(
    SLEngineItf self, SLObjectItf *pObject, void *pParameters, SLuint32 objectID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QueryNumSupportedInterfaces(SLEngineItf self, SLuint32 objectID, SLuint32 *pNumSupportedInterfaces)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QuerySupportedInterfaces(
    SLEngineItf self, SLuint32 objectID, SLuint32 index, SLInterfaceID *pInterfaceId)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QueryNumSupportedExtensions(SLEngineItf self, SLuint32 *pNumExtensions)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QuerySupportedExtension(
    SLEngineItf self, SLuint32 index, SLchar *pExtensionName, SLint16 *pNameLength)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult IsExtensionSupported(SLEngineItf self, const SLchar *pExtensionName, SLboolean *pSupported)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLEngineItf_ EngineItf = {
    CreateLEDDevice,
    CreateVibraDevice,
    CreateAudioPlayer,
    CreateAudioRecorder,
    CreateMidiPlayer,
    CreateListener,
    Create3DGroup,
    CreateOutputMix,
    CreateMetadataExtractor,
    CreateExtensionObject,
    QueryNumSupportedInterfaces,
    QuerySupportedInterfaces,
    QueryNumSupportedExtensions,
    QuerySupportedExtension,
    IsExtensionSupported
};

void IEngineInit(void *self)
{
    IEngine *thiz = (IEngine *) self;
    thiz->mItf = &EngineItf;
}
