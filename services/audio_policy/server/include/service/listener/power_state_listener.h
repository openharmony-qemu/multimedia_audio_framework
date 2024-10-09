/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef ST_POWER_STATE_LISTENER_H
#define ST_POWER_STATE_LISTENER_H

#include <iremote_stub.h>
#include <audio_interrupt_info.h>

#include "suspend/isync_sleep_callback.h"
#include "suspend/sleep_priority.h"
#include "power_mgr_client.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::PowerMgr;
class AudioPolicyServer;
class PowerListerMethods {
public:
    PowerListerMethods() = default;
    virtual ~PowerListerMethods() = default;
    static void InitAudioInterruptInfo(AudioInterrupt& audioInterrupt);
};

class PowerStateListenerStub : public IRemoteStub<ISyncSleepCallback> {
public:
    PowerStateListenerStub() = default;
    virtual ~PowerStateListenerStub() = default;
    DISALLOW_COPY_AND_MOVE(PowerStateListenerStub);

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnSyncSleepCallbackStub(MessageParcel &data);
    int32_t OnSyncWakeupCallbackStub(MessageParcel &data);
};

class PowerStateListener : public PowerStateListenerStub {
public:
    explicit PowerStateListener(const sptr<AudioPolicyServer> audioPolicyServer);
    virtual ~PowerStateListener() {}
    void OnSyncSleep(bool OnForceSleep) override;
    void OnSyncWakeup(bool OnForceSleep) override;

private:
    void ControlAudioFocus(bool applyFocus);
    sptr<AudioPolicyServer> audioPolicyServer_;
};
 
class SyncHibernateListenerStub : public IRemoteStub<ISyncHibernateCallback> {
public:
    SyncHibernateListenerStub() = default;
    virtual ~SyncHibernateListenerStub() = default;
    DISALLOW_COPY_AND_MOVE(SyncHibernateListenerStub);
 
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;
 
private:
    int32_t OnSyncHibernateCallbackStub();
    int32_t OnSyncWakeupCallbackStub();
};
 
class SyncHibernateListener : public SyncHibernateListenerStub {
public:
    explicit SyncHibernateListener(const sptr<AudioPolicyServer> audioPolicyServer);
    virtual ~SyncHibernateListener() {}
    void OnSyncHibernate() override;
    void OnSyncWakeup() override;
 
private:
    void ControlAudioFocus(bool isHibernate);
    sptr<AudioPolicyServer> audioPolicyServer_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_POWER_STATE_LISTENER_H
