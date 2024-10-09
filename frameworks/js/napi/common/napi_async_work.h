/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef NAPI_ASYNC_WORK_H
#define NAPI_ASYNC_WORK_H

#include <functional>
#include <memory>
#include <string>
#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_common_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using NapiCbInfoParser = std::function<void(size_t argc, napi_value *argv)>;
using NapiAsyncExecute = std::function<void(void)>;
using NapiAsyncComplete = std::function<void(napi_value&)>;

struct ContextBase {
    virtual ~ContextBase();
    void GetCbInfo(napi_env env, napi_callback_info info, NapiCbInfoParser parse = NapiCbInfoParser(),
        bool sync = false);
    void SignError(int32_t code);
    void SignError(int32_t code, const std::string &errorMessage);
    napi_env env = nullptr;
    napi_value output = nullptr;
    napi_status status = napi_invalid_arg;
    std::string errMessage;
    int32_t errCode;
    napi_value self = nullptr;
    void* native = nullptr;
    std::string taskName;

private:
    napi_deferred deferred = nullptr;
    napi_async_work work = nullptr;
    napi_ref callbackRef = nullptr;
    napi_ref selfRef = nullptr;

    NapiAsyncExecute execute = nullptr;
    NapiAsyncComplete complete = nullptr;
    std::shared_ptr<ContextBase> hold; /* cross thread data */

    static constexpr size_t ARGC_MAX = 6;

    friend class NapiAsyncWork;
};

struct NapiWorkData {
    napi_env env_;
    napi_ref cb_;
};

struct AutoRef {
    AutoRef(napi_env env, napi_ref cb)
        : env_(env), cb_(cb)
    {
    }
    ~AutoRef()
    {
        uv_loop_s *loop = nullptr;
        napi_get_uv_event_loop(env_, &loop);
        if (loop == nullptr) {
            return;
        }

        NapiWorkData *workData = new (std::nothrow) NapiWorkData();
        if (workData == nullptr) {
            return;
        }
        workData->env_ = env_;
        workData->cb_ = cb_;

        uv_work_t *work = new(std::nothrow) uv_work_t;
        if (work == nullptr) {
            delete workData;
            workData = nullptr;
            return;
        }
        work->data = (void *)workData;

        int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
            // Js thread
            NapiWorkData *workData = reinterpret_cast<NapiWorkData *>(work->data);
            napi_env env = workData->env_;
            napi_ref cb = workData->cb_;
            if (env != nullptr && cb != nullptr) {
                (void)napi_delete_reference(env, cb);
            }
            delete workData;
            delete work;
        }, uv_qos_default);
        if (ret != 0) {
            delete work;
            work = nullptr;
            delete workData;
            workData = nullptr;
        }
    }
    napi_env env_;
    napi_ref cb_;
};

class NapiAsyncWork {
public:
    static napi_value Enqueue(napi_env env, std::shared_ptr<ContextBase> ctxt, const std::string &name,
        NapiAsyncExecute execute = NapiAsyncExecute(),
        NapiAsyncComplete complete = NapiAsyncComplete());

private:
    enum {
        /* AsyncCallback / Promise output result index  */
        RESULT_ERROR = 0,
        RESULT_DATA = 1,
        RESULT_ALL = 2
    };
    static void CommonCallbackRoutine(ContextBase *ctxt);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_ASYNC_WORK_H