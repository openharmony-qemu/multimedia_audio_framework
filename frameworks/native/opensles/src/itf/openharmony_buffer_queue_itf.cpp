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

SLresult Enqueue(SLOHBufferQueueItf self, const void *buffer, SLuint32 size)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    if (thiz->mIid == SL_IID_PLAY) {
        AudioPlayerAdapter::GetInstance()->EnqueueAdapter(thiz->mId, buffer, size);
    } else if (thiz->mIid == SL_IID_RECORD) {
        AudioCapturerAdapter::GetInstance()->EnqueueAdapter(thiz->mId, buffer, size);
    }

    return SL_RESULT_SUCCESS;
}

SLresult Clear(SLOHBufferQueueItf self)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    if (thiz->mIid == SL_IID_PLAY) {
        AudioPlayerAdapter::GetInstance()->ClearAdapter(thiz->mId);
    } else if (thiz->mIid == SL_IID_RECORD) {
        AudioCapturerAdapter::GetInstance()->ClearAdapter(thiz->mId);
    }
    
    return SL_RESULT_SUCCESS;
}

SLresult GetState(SLOHBufferQueueItf self, SLOHBufferQueueState *state)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    if (thiz->mIid == SL_IID_PLAY) {
        AudioPlayerAdapter::GetInstance()->GetStateAdapter(thiz->mId, state);
    } else if (thiz->mIid == SL_IID_RECORD) {
        AudioCapturerAdapter::GetInstance()->GetStateAdapter(thiz->mId, state);
    }

    return SL_RESULT_SUCCESS;
}

static SLresult GetBuffer(SLOHBufferQueueItf self, SLuint8 **buffer, SLuint32 *size)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    if (thiz->mIid == SL_IID_PLAY) {
        AudioPlayerAdapter::GetInstance()->GetBufferAdapter(thiz->mId, buffer, size);
    } else if (thiz->mIid == SL_IID_RECORD) {
        AudioCapturerAdapter::GetInstance()->GetBufferAdapter(thiz->mId, buffer, size);
    }
    return SL_RESULT_SUCCESS;
}

SLresult RegisterCallback(SLOHBufferQueueItf self, SlOHBufferQueueCallback callback, void *pContext)
{
    if (self == nullptr || callback == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    if (thiz->mIid == SL_IID_PLAY) {
        AudioPlayerAdapter::GetInstance()->RegisterCallbackAdapter(self, callback, pContext);
    } else if (thiz->mIid == SL_IID_RECORD) {
        AudioCapturerAdapter::GetInstance()->RegisterCallbackAdapter(self, callback, pContext);
    }
    return SL_RESULT_SUCCESS;
}

static const struct SLOHBufferQueueItf_ IOHBufferQueueItf = {
    Enqueue,
    Clear,
    GetState,
    GetBuffer,
    RegisterCallback
};

void IOHBufferQueueInit(void *self, const SLInterfaceID iid, SLuint32 id)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *) self;
    thiz->mItf = &IOHBufferQueueItf;
    thiz->mIid = iid;
    thiz->mId = id;
    if (thiz->mIid == SL_IID_PLAY) {
        thiz->mState = SL_PLAYSTATE_STOPPED;
    } else if (thiz->mIid == SL_IID_RECORD) {
        thiz->mState = SL_RECORDSTATE_STOPPED;
    }
}
