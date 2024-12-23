/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
#define ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
#ifndef LOG_TAG
#define LOG_TAG "PulseAudioServiceAdapterImpl"
#endif

#include "pulse_audio_service_adapter_impl.h"

#include <sstream>
#include <unistd.h>
#include <thread>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "hisysevent.h"
#include <set>
#include <unordered_map>

#include "media_monitor_manager.h"
#include "event_bean.h"
#include "pa_adapter_tools.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static unique_ptr<AudioServiceAdapterCallback> g_audioServiceAdapterCallback;
SafeMap<uint32_t, uint32_t> PulseAudioServiceAdapterImpl::sourceIndexSessionIDMap;

static const int32_t PA_SERVICE_IMPL_TIMEOUT = 10; // 10s is better
static const unordered_map<std::string, AudioStreamType> STREAM_TYPE_STRING_ENUM_MAP = {
    {"voice_call", STREAM_VOICE_CALL},
    {"voice_call_assistant", STREAM_VOICE_CALL_ASSISTANT},
    {"music", STREAM_MUSIC},
    {"ring", STREAM_RING},
    {"media", STREAM_MEDIA},
    {"voice_assistant", STREAM_VOICE_ASSISTANT},
    {"system", STREAM_SYSTEM},
    {"alarm", STREAM_ALARM},
    {"notification", STREAM_NOTIFICATION},
    {"bluetooth_sco", STREAM_BLUETOOTH_SCO},
    {"enforced_audible", STREAM_ENFORCED_AUDIBLE},
    {"dtmf", STREAM_DTMF},
    {"tts", STREAM_TTS},
    {"accessibility", STREAM_ACCESSIBILITY},
    {"recording", STREAM_RECORDING},
    {"movie", STREAM_MOVIE},
    {"game", STREAM_GAME},
    {"speech", STREAM_SPEECH},
    {"system_enforced", STREAM_SYSTEM_ENFORCED},
    {"ultrasonic", STREAM_ULTRASONIC},
    {"wakeup", STREAM_WAKEUP},
    {"voice_message", STREAM_VOICE_MESSAGE},
    {"navigation", STREAM_NAVIGATION},
    {"camcorder", STREAM_CAMCORDER}
};

AudioServiceAdapter::~AudioServiceAdapter() = default;
PulseAudioServiceAdapterImpl::~PulseAudioServiceAdapterImpl() = default;

unique_ptr<AudioServiceAdapter> AudioServiceAdapter::CreateAudioAdapter(unique_ptr<AudioServiceAdapterCallback> cb)
{
    CHECK_AND_RETURN_RET_LOG(cb != nullptr, nullptr, "CreateAudioAdapter cb is nullptr!");
    return make_unique<PulseAudioServiceAdapterImpl>(cb);
}

PulseAudioServiceAdapterImpl::PulseAudioServiceAdapterImpl(unique_ptr<AudioServiceAdapterCallback> &cb)
{
    g_audioServiceAdapterCallback = move(cb);
}

bool PulseAudioServiceAdapterImpl::Connect()
{
    mMainLoop = pa_threaded_mainloop_new();
    CHECK_AND_RETURN_RET_LOG(mMainLoop, false, "MainLoop creation failed");
    pa_threaded_mainloop_set_name(mMainLoop, "OS_AudioML");
    if (pa_threaded_mainloop_start(mMainLoop) < 0) {
        AUDIO_ERR_LOG("Failed to start mainloop");
        pa_threaded_mainloop_free(mMainLoop);
        return false;
    }
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server

    PaLockGuard palock(mMainLoop);
    Trace trace("PulseAudioServiceAdapterImpl::Connect");

    while (true) {
        pa_context_state_t state;

        if (mContext != nullptr) {
            state = pa_context_get_state(mContext);
            if (state == PA_CONTEXT_READY) {
                break;
            }
            // if pulseaudio is ready, retry connect to pulseaudio. before retry wait for sometime. reduce sleep later
            usleep(PA_CONNECT_RETRY_SLEEP_IN_MICRO_SECONDS);
        }

        bool result = ConnectToPulseAudio();
        if (!result || !PA_CONTEXT_IS_GOOD(pa_context_get_state(mContext))) {
            continue;
        }

        AUDIO_DEBUG_LOG("pa context not ready... wait");

        // Wait for the context to be ready
        AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::Connect", PA_SERVICE_IMPL_TIMEOUT,
            [](void *) {
                AUDIO_ERR_LOG("Connect timeout");
            }, nullptr, XcollieFlag);
        pa_threaded_mainloop_wait(mMainLoop);
    }

    return true;
}

bool PulseAudioServiceAdapterImpl::ConnectToPulseAudio()
{
    if (mContext != nullptr) {
        AUDIO_DEBUG_LOG("context is not null, disconnect first!");
        pa_context_disconnect(mContext);
        pa_context_set_state_callback(mContext, nullptr, nullptr);
        pa_context_set_subscribe_callback(mContext, nullptr, nullptr);
        pa_context_unref(mContext);
        mContext = nullptr;
    }
    pa_proplist *proplist = pa_proplist_new();
    if (proplist == nullptr) {
        AUDIO_ERR_LOG("Connect to pulseAudio and new proplist return nullptr!");
        return false;
    }
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "PulseAudio Service");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.ohos.pulseaudio.service");
    mContext = pa_context_new_with_proplist(pa_threaded_mainloop_get_api(mMainLoop), nullptr, proplist);
    pa_proplist_free(proplist);

    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, false, "creating pa context failed");

    pa_context_set_state_callback(mContext,  PulseAudioServiceAdapterImpl::PaContextStateCb, this);
    if (pa_context_connect(mContext, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(mContext) == PA_ERR_INVALID) {
            AUDIO_ERR_LOG("pa context connect failed: %{public}s",
                pa_strerror(pa_context_errno(mContext)));
            goto Fail;
        }
    }

    return true;

Fail:
    /* Make sure we don't get any further callbacks */
    pa_context_set_state_callback(mContext, nullptr, nullptr);
    pa_context_set_subscribe_callback(mContext, nullptr, nullptr);
    pa_context_unref(mContext);
    mContext = nullptr;
    return false;
}

uint32_t PulseAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, string moduleArgs)
{
    AUDIO_PRERELEASE_LOGI("OpenAudioPort enter.");
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::OpenAudioPort", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("OpenAudioPort timeout");
        }, nullptr, XcollieFlag);
    lock_guard<mutex> lock(lock_);

    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    PaLockGuard palock(mMainLoop);
    Trace trace("PulseAudioServiceAdapterImpl::OpenAudioPort");
    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }

    pa_operation *operation = pa_context_load_module(mContext, audioPortName.c_str(), moduleArgs.c_str(),
        PaModuleLoadCb, reinterpret_cast<void*>(userData.get()));
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_load_module returned nullptr");
        return PA_INVALID_INDEX;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);

    CHECK_AND_RETURN_RET_LOG(userData->idx != PA_INVALID_INDEX, PA_INVALID_INDEX,
        "OpenAudioPort returned invalid index");

    return userData->idx;
}

int32_t PulseAudioServiceAdapterImpl::CloseAudioPort(int32_t audioHandleIndex)
{
    lock_guard<mutex> lock(lock_);

    PaLockGuard palock(mMainLoop);
    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }

    pa_operation *operation = pa_context_unload_module(mContext, audioHandleIndex, nullptr, nullptr);
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_unload_module returned nullptr!");
        return ERROR;
    }

    pa_operation_unref(operation);
    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SuspendAudioDevice(string &audioPortName, bool isSuspend)
{
    AUDIO_INFO_LOG("[%{public}s] : [%{public}d]", audioPortName.c_str(), isSuspend);
    PaLockGuard palock(mMainLoop);
    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }

    auto suspendFlag = isSuspend ? 1 : 0;
    pa_operation *operation = pa_context_suspend_sink_by_name(mContext, audioPortName.c_str(), suspendFlag,
        nullptr, nullptr);
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_suspend_sink_by_name failed!");
        return ERR_OPERATION_FAILED;
    }

    pa_operation_unref(operation);

    return SUCCESS;
}

bool PulseAudioServiceAdapterImpl::SetSinkMute(const std::string &sinkName, bool isMute, bool isSync)
{
    AUDIO_DEBUG_LOG("MuteAudioDevice: [%{public}s] : [%{public}d]", sinkName.c_str(), isMute);
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::SetSinkMute", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("SetSinkMute timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    PaLockGuard palock(mMainLoop);
    Trace trace("PulseAudioServiceAdapterImpl::SetSinkMute");

    int muteFlag = isMute ? 1 : 0;

    pa_operation *operation = nullptr;
    if (isSync) {
        operation = pa_context_set_sink_mute_by_name(mContext, sinkName.c_str(), muteFlag,
            PulseAudioServiceAdapterImpl::PaSinkMuteCb, reinterpret_cast<void *>(userData.get()));
    } else {
        operation = pa_context_set_sink_mute_by_name(mContext, sinkName.c_str(), muteFlag,
            nullptr, nullptr);
    }

    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_suspend_sink_by_name failed!");
        return false;
    }

    if (isSync) {
        while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
            pa_threaded_mainloop_wait(mMainLoop);
        }
    }

    pa_operation_unref(operation);

    return true;
}

int32_t PulseAudioServiceAdapterImpl::SetDefaultSink(string name)
{
    PaLockGuard palock(mMainLoop);
    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }

    pa_operation *operation = pa_context_set_default_sink(mContext, name.c_str(), nullptr, nullptr);
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_set_default_sink failed!");
        return ERR_OPERATION_FAILED;
    }
    isSetDefaultSink_ = true;
    pa_operation_unref(operation);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetDefaultSource(string name)
{
    PaLockGuard palock(mMainLoop);
    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }

    pa_operation *operation = pa_context_set_default_source(mContext, name.c_str(), nullptr, nullptr);
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_set_default_source failed!");
        return ERR_OPERATION_FAILED;
    }
    isSetDefaultSource_ = true;
    pa_operation_unref(operation);

    return SUCCESS;
}

void PulseAudioServiceAdapterImpl::PaGetSinksCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData *>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        AUDIO_ERR_LOG("Failed to get sink information: %{public}s", pa_strerror(pa_context_errno(c)));
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    CHECK_AND_RETURN_LOG(i->proplist != nullptr, "Invalid Proplist for sink (%{public}d).", i->index);

    const char *adapterCStr = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_STRING);
    if (adapterCStr == nullptr) {
        adapterCStr = "";
    }
    AUDIO_DEBUG_LOG("sink[%{public}d] device[%{public}s] name[%{public}s]", i->index, adapterCStr,
        i->name);
    std::string sinkDeviceName(adapterCStr);
    std::string sinkName(i->name);
    SinkInfo sinkInfo = {};
    sinkInfo.sinkId = i->index;
    sinkInfo.sinkName = sinkName;
    sinkInfo.adapterName = sinkDeviceName;
    userData->sinkInfos.push_back(sinkInfo);
}

std::vector<SinkInfo> PulseAudioServiceAdapterImpl::GetAllSinks()
{
    AUDIO_PRERELEASE_LOGI("GetAllSinks enter.");
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::GetAllSinks", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("GetAllSinks timeout");
        }, nullptr, XcollieFlag);
    lock_guard<mutex> lock(lock_);
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->sinkInfos = {};

    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, userData->sinkInfos, "mContext is nullptr");

    PaLockGuard palock(mMainLoop);
    Trace trace("PulseAudioServiceAdapterImpl::GetAllSinks");

    pa_operation *operation = pa_context_get_sink_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetSinksCb, reinterpret_cast<void*>(userData.get()));
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_get_sink_info_list returned nullptr");
        return userData->sinkInfos;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);

    AUDIO_DEBUG_LOG("end, get [%{public}zu] sinks.", userData->sinkInfos.size());
    return userData->sinkInfos;
}

std::vector<uint32_t> PulseAudioServiceAdapterImpl::GetTargetSinks(std::string adapterName)
{
    std::vector<SinkInfo> sinkInfos = GetAllSinks();
    std::vector<uint32_t> targetSinkIds = {};
    for (size_t i = 0; i < sinkInfos.size(); i++) {
        if (sinkInfos[i].adapterName == adapterName) {
            targetSinkIds.push_back(sinkInfos[i].sinkId);
        }
    }
    return targetSinkIds;
}

int32_t PulseAudioServiceAdapterImpl::SetLocalDefaultSink(std::string name)
{
    std::vector<SinkInput> allSinkInputs = GetAllSinkInputs();

    std::string remoteDevice = "remote";
    std::vector<uint32_t> remoteSinks = GetTargetSinks(remoteDevice);

    // filter sink-inputs which are not connected with remote sinks.
    for (auto sinkInput : allSinkInputs) {
        uint32_t sink = sinkInput.deviceSinkId;
        // the sink inputs connected to remote device remain the same
        CHECK_AND_CONTINUE_LOG(std::find(remoteSinks.begin(), remoteSinks.end(), sink) == remoteSinks.end(),
            "sink-input[%{public}d] connects with remote device[%{public}d]",
            sinkInput.paStreamId, sinkInput.deviceSinkId);
        // move the remaining sink inputs to the default sink
        uint32_t invalidSinkId = PA_INVALID_INDEX;
        MoveSinkInputByIndexOrName(sinkInput.paStreamId, invalidSinkId, name);
    }

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex,
    std::string sinkName)
{
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::MoveSinkInputByIndexOrName", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("MoveSinkInputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("PulseAudioServiceAdapterImpl::MoveSinkInputByIndexOrName:id:" + std::to_string(sinkInputId) +
        +":index:" + std::to_string(sinkIndex) + ":name:" + sinkName);

    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, ERROR, "mContext is nullptr");
    PaLockGuard palock(mMainLoop);
    pa_operation *operation = nullptr;
    if (sinkName.empty()) {
        operation = pa_context_move_sink_input_by_index(mContext, sinkInputId, sinkIndex,
            PulseAudioServiceAdapterImpl::PaMoveSinkInputCb, reinterpret_cast<void *>(userData.get()));
    } else {
        operation = pa_context_move_sink_input_by_name(mContext, sinkInputId, sinkName.c_str(),
            PulseAudioServiceAdapterImpl::PaMoveSinkInputCb, reinterpret_cast<void *>(userData.get()));
    }

    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_get_sink_input_info_list nullptr");
        return ERROR;
    }
    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }
    pa_operation_unref(operation);

    int result = userData->moveResult;
    AUDIO_DEBUG_LOG("move result:[%{public}d]", result);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName(uint32_t sourceOutputId, uint32_t sourceIndex,
    std::string sourceName)
{
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName",
        PA_SERVICE_IMPL_TIMEOUT, [](void *) {
            AUDIO_ERR_LOG("MoveSourceOutputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("PulseAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName:id:" + std::to_string(sourceOutputId) +
        +":index:" + std::to_string(sourceIndex) + ":name:" + sourceName);

    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    if (mContext == nullptr) {
        AUDIO_ERR_LOG("mContext is nullptr");
        return ERROR;
    }
    PaLockGuard palock(mMainLoop);
    pa_operation *operation = nullptr;
    if (sourceName.empty()) {
        operation = pa_context_move_source_output_by_index(mContext, sourceOutputId, sourceIndex,
            PulseAudioServiceAdapterImpl::PaMoveSourceOutputCb, reinterpret_cast<void *>(userData.get()));
    } else {
        operation = pa_context_move_source_output_by_name(mContext, sourceOutputId, sourceName.c_str(),
            PulseAudioServiceAdapterImpl::PaMoveSourceOutputCb, reinterpret_cast<void *>(userData.get()));
    }

    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_get_sink_input_info_list nullptr");
        return ERROR;
    }
    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }
    pa_operation_unref(operation);

    int result = userData->moveResult;
    AUDIO_DEBUG_LOG("move result:[%{public}d]", result);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetSourceOutputMute(int32_t uid, bool setMute)
{
    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, ERROR, "mContext is nullptr");
    vector<SourceOutput> sourOutputs = GetAllSourceOutputs();
    lock_guard<mutex> lock(lock_);
    int32_t streamSet = 0;
    for (uint32_t i = 0; i < sourOutputs.size(); i ++) {
        if (sourOutputs[i].uid == uid) {
            PaLockGuard palock(mMainLoop);
            pa_operation *operation = pa_context_set_source_output_mute(mContext, sourOutputs[i].paStreamId,
                (setMute ? 1 : 0), nullptr, nullptr);
            if (operation == nullptr) {
                AUDIO_ERR_LOG("pa_context_set_source_output_mute nullptr");
                return ERROR;
            }
            pa_operation_unref(operation);
            AUDIO_DEBUG_LOG("set source output Mute : %{public}s for stream :uid %{public}d",
                (setMute ? "true" : "false"), sourOutputs[i].uid);
            streamSet++;
        }
    }
    AUDIO_INFO_LOG("set %{public}d %{public}s", streamSet, (setMute ? "mute" : "unmuted"));
    return streamSet;
}

vector<SinkInput> PulseAudioServiceAdapterImpl::GetAllSinkInputs()
{
    AUDIO_PRERELEASE_LOGI("GetAllSinkInputs enter");
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->sinkInfos = GetAllSinks();

    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::GetAllSinkInputs", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("GetAllSinkInputs timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, userData->sinkInputList, "mContext is nullptr");

    PaLockGuard palock(mMainLoop);
    Trace trace("PulseAudioServiceAdapterImpl::GetAllSinkInputs");

    pa_operation *operation = pa_context_get_sink_input_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetAllSinkInputsCb, reinterpret_cast<void*>(userData.get()));
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_get_sink_input_info_list returned nullptr");
        return userData->sinkInputList;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);

    AUDIO_DEBUG_LOG("get:[%{public}zu]", userData->sinkInputList.size());
    return userData->sinkInputList;
}

vector<SourceOutput> PulseAudioServiceAdapterImpl::GetAllSourceOutputs()
{
    AudioXCollie audioXCollie("PulseAudioServiceAdapterImpl::GetAllSourceOutputs", PA_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("GetAllSourceOutputs timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("PulseAudioServiceAdapterImpl::GetAllSourceOutputs");

    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    CHECK_AND_RETURN_RET_LOG(mContext != nullptr, userData->sourceOutputList, "mContext is nullptr");

    CHECK_AND_RETURN_RET_LOG(isSetDefaultSource_, userData->sourceOutputList, "default source has not been set.");

    PaLockGuard palock(mMainLoop);

    pa_operation *operation = pa_context_get_source_output_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetAllSourceOutputsCb, reinterpret_cast<void*>(userData.get()));
    if (operation == nullptr) {
        AUDIO_ERR_LOG("pa_context_get_source_output_info_list returned nullptr");
        return userData->sourceOutputList;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);

    return userData->sourceOutputList;
}

void PulseAudioServiceAdapterImpl::Disconnect()
{
    if (mContext != nullptr) {
        AUDIO_WARNING_LOG("disconnect context! should not happen");
        pa_context_disconnect(mContext);
        /* Make sure we don't get any further callbacks */
        pa_context_set_state_callback(mContext, nullptr, nullptr);
        pa_context_set_subscribe_callback(mContext, nullptr, nullptr);
        pa_context_unref(mContext);
        mContext = nullptr;
    }

    if (mMainLoop != nullptr) {
        AUDIO_WARNING_LOG("disconnect mainloop! should not happen");
        pa_threaded_mainloop_stop(mMainLoop);
        pa_threaded_mainloop_free(mMainLoop);
        mMainLoop = nullptr;
    }
}

AudioStreamType PulseAudioServiceAdapterImpl::GetIdByStreamType(string streamType)
{
    AudioStreamType stream = STREAM_MUSIC;
    if (STREAM_TYPE_STRING_ENUM_MAP.find(streamType) != STREAM_TYPE_STRING_ENUM_MAP.end()) {
        stream = STREAM_TYPE_STRING_ENUM_MAP.at(streamType);
    } else {
        AUDIO_WARNING_LOG("Invalid stream type [%{public}s]. Use default type", streamType.c_str());
    }
    return stream;
}

void PulseAudioServiceAdapterImpl::PaMoveSinkInputCb(pa_context *c, int success, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData *>(userdata);

    AUDIO_DEBUG_LOG("result[%{public}d]", success);
    userData->moveResult = success;

    pa_threaded_mainloop_signal(userData->thiz->mMainLoop, 0);

    return;
}

void PulseAudioServiceAdapterImpl::PaMoveSourceOutputCb(pa_context *c, int success, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData *>(userdata);

    AUDIO_INFO_LOG("result[%{public}d]", success);
    userData->moveResult = success;

    pa_threaded_mainloop_signal(userData->thiz->mMainLoop, 0);

    return;
}

void PulseAudioServiceAdapterImpl::PaSinkMuteCb(pa_context *c, int success, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData *>(userdata);
    AUDIO_DEBUG_LOG("result[%{public}d]", success);
    pa_threaded_mainloop_signal(userData->thiz->mMainLoop, 0);
}

void PulseAudioServiceAdapterImpl::PaContextStateCb(pa_context *c, void *userdata)
{
    PulseAudioServiceAdapterImpl *thiz = reinterpret_cast<PulseAudioServiceAdapterImpl*>(userdata);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_context_set_subscribe_callback(c, PulseAudioServiceAdapterImpl::PaSubscribeCb, thiz);

            pa_operation *operation = pa_context_subscribe(c, (pa_subscription_mask_t)
                (PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE |
                PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT |
                PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr);
            if (operation == nullptr) {
                pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
                return;
            }
            pa_operation_unref(operation);
            pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
            break;
        }

        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            AUDIO_ERR_LOG("state is PA_CONTEXT_FAILED or PA_CONTEXT_TERMINATED");
            pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
            return;

        default:
            return;
    }
}

void PulseAudioServiceAdapterImpl::PaModuleLoadCb(pa_context *c, uint32_t idx, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    if (idx == PA_INVALID_INDEX) {
        AUDIO_ERR_LOG("Failure: %{public}s", pa_strerror(pa_context_errno(c)));
        userData->idx = PA_INVALID_INDEX;
    } else {
        userData->idx = idx;
    }
    pa_threaded_mainloop_signal(userData->thiz->mMainLoop, 0);

    return;
}

template <typename T>
inline void CastValue(T &a, const char *raw)
{
    if (raw == nullptr) {
        return;
    }
    std::stringstream valueStr;
    valueStr << raw;
    valueStr >> a;
}

void PulseAudioServiceAdapterImpl::PaGetSourceOutputNoSignalCb(pa_context *c, const pa_source_output_info *i,
    int eol, void *userdata)
{
    AUDIO_INFO_LOG("in eol[%{public}d]", eol);
    UserData *userData = reinterpret_cast<UserData*>(userdata);

    if (eol < 0) {
        delete userData;
        AUDIO_ERR_LOG("Failed to get source output information: %{public}s",
            pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol) {
        delete userData;
        return;
    }

    CHECK_AND_RETURN_LOG(i->proplist != nullptr, "Invalid proplist for source output (%{public}d).", i->index);

    const char *streamSession = pa_proplist_gets(i->proplist, "stream.sessionID");
    CHECK_AND_RETURN_LOG(streamSession != nullptr, "Invalid stream parameter:sessionID.");

    std::stringstream sessionStr;
    uint32_t sessionID;
    sessionStr << streamSession;
    sessionStr >> sessionID;
    AUDIO_INFO_LOG("sessionID %{public}u", sessionID);
    sourceIndexSessionIDMap.Insert(i->index, sessionID);
}

void PulseAudioServiceAdapterImpl::PaGetAllSinkInputsCb(pa_context *c, const pa_sink_input_info *i, int eol,
    void *userdata)
{
    AUDIO_DEBUG_LOG("in eol[%{public}d]", eol);
    UserData *userData = reinterpret_cast<UserData *>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        AUDIO_ERR_LOG("Failed to get sink input information: %{public}s", pa_strerror(pa_context_errno(c)));
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    CHECK_AND_RETURN_LOG(i->proplist != nullptr,
        "Invalid Proplist for sink input (%{public}d).", i->index);

    const char *streamMode = pa_proplist_gets(i->proplist, "stream.mode");
    if (streamMode != nullptr && streamMode == DUP_STREAM) { return; }

    AudioStreamType audioStreamType = STREAM_DEFAULT;
    const char *streamType = pa_proplist_gets(i->proplist, "stream.type");
    if (streamType != nullptr) {
        audioStreamType = thiz->GetIdByStreamType(streamType);
    }

    SinkInput sinkInput = {};
    sinkInput.streamType = audioStreamType;

    sinkInput.deviceSinkId = i->sink;
    for (auto sinkInfo : userData->sinkInfos) {
        if (sinkInput.deviceSinkId == sinkInfo.sinkId) {
            sinkInput.sinkName = sinkInfo.sinkName;
            break;
        }
    }
    sinkInput.paStreamId = i->index;
    CastValue<int32_t>(sinkInput.streamId, pa_proplist_gets(i->proplist, "stream.sessionID"));
    CastValue<int32_t>(sinkInput.uid, pa_proplist_gets(i->proplist, "stream.client.uid"));
    CastValue<int32_t>(sinkInput.pid, pa_proplist_gets(i->proplist, "stream.client.pid"));
    CastValue<uint64_t>(sinkInput.startTime, pa_proplist_gets(i->proplist, "stream.startTime"));

    userData->sinkInputList.push_back(sinkInput);
}

void PulseAudioServiceAdapterImpl::PaGetAllSourceOutputsCb(pa_context *c, const pa_source_output_info *i, int eol,
    void *userdata)
{
    AUDIO_INFO_LOG("in eol[%{public}d]", eol);
    UserData *userData = reinterpret_cast<UserData *>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        AUDIO_ERR_LOG("Failed to get source output information: %{public}s", pa_strerror(pa_context_errno(c)));
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    CHECK_AND_RETURN_LOG(i->proplist != nullptr,
        "Invalid Proplist for source output (%{public}d).", i->index);

    int32_t sessionID = 0;
    const char *sessionCStr = pa_proplist_gets(i->proplist, "stream.sessionID");
    if (sessionCStr != nullptr) {
        std::stringstream sessionStr;
        sessionStr << sessionCStr;
        sessionStr >> sessionID;
    }

    AudioStreamType audioStreamType = STREAM_DEFAULT;
    const char *streamType = pa_proplist_gets(i->proplist, "stream.type");
    if (streamType != nullptr) {
        audioStreamType = thiz->GetIdByStreamType(streamType);
    }

    SourceOutput sourceOutput = {};
    sourceOutput.streamId = sessionID;
    sourceOutput.streamType = audioStreamType;

    sourceOutput.paStreamId = i->index;
    sourceOutput.deviceSourceId = i->source;
    CastValue<int32_t>(sourceOutput.uid, pa_proplist_gets(i->proplist, "stream.client.uid"));
    CastValue<int32_t>(sourceOutput.pid, pa_proplist_gets(i->proplist, "stream.client.pid"));
    CastValue<uint64_t>(sourceOutput.startTime, pa_proplist_gets(i->proplist, "stream.startTime"));
    userData->sourceOutputList.push_back(sourceOutput);
}

void PulseAudioServiceAdapterImpl::ProcessSourceOutputEvent(pa_context *c, pa_subscription_event_type_t t, uint32_t idx,
    void *userdata)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    PulseAudioServiceAdapterImpl *thiz = reinterpret_cast<PulseAudioServiceAdapterImpl*>(userdata);
    userData->thiz = thiz;
    if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
        pa_operation *operation = pa_context_get_source_output_info(c, idx,
            PulseAudioServiceAdapterImpl::PaGetSourceOutputNoSignalCb, reinterpret_cast<void*>(userData.get()));
        if (operation == nullptr) {
            AUDIO_ERR_LOG("pa_context_get_source_output_info nullptr");
            return;
        }
        userData.release();
        pa_operation_unref(operation);
    } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
        uint32_t sessionID = 0;
        if (sourceIndexSessionIDMap.Find(idx, sessionID) == true) {
            AUDIO_ERR_LOG("sessionID: %{public}d removed", sessionID);
            g_audioServiceAdapterCallback->OnAudioStreamRemoved(sessionID);
            sourceIndexSessionIDMap.Erase(idx);
        } else {
            AUDIO_ERR_LOG("cannot find sessionID in sourceIndexSessionIDMap");
        }
    }
}

void PulseAudioServiceAdapterImpl::PaSubscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx,
    void *userdata)
{
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
                AUDIO_INFO_LOG("PA_SUBSCRIPTION_EVENT_NEW");
                g_audioServiceAdapterCallback->OnSetVolumeDbCb();
            } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                AUDIO_INFO_LOG("PA_SUBSCRIPTION_EVENT_REMOVE");
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            ProcessSourceOutputEvent(c, t, idx, userdata);
            break;

        default:
            break;
    }
}
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
