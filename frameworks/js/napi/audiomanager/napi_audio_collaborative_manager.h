#ifndef NAPI_AUDIO_COLLABORATIVE_MANAGER_H
#define NAPI_AUDIO_COLLABORATIVE_MANAGER_H
#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_collaborative_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_COLLABORATIVE_MANAGER_NAPI_CLASS_NAME = "AudioCollaborativeManager";
class NapiAudioCollaborativeManager {
public:
    NapiAudioCollaborativeManager();
    ~NapiAudioCollaborativeManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateCollaborativeManagerWrapper(napi_env env);

private:
    struct AudioCollaborativeManagerAsyncContext : public ContextBase {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        bool collaborativeEnable;
        int32_t intValue;
        AudioSpatialDeviceState spatialDeviceState;
    };
    static bool CheckContextStatus(std::shared_ptr<AudioCollaborativeManagerAsyncContext> context);
    static bool CheckAudioCollaborativeManagerStatus(NapiAudioCollaborativeManager *napi,
    std::shared_ptr<AudioCollaborativeManagerAsyncContext> context);
    static NapiAudioCollaborativeManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value IsCollaborativePlaybackSupported(napi_env env, napi_callback_info info);
    static napi_value IsCollaborativePlaybackEnabledForDevice(napi_env env, napi_callback_info info);
    static napi_value SetCollaborativePlaybackEnabledForDevice(napi_env env, napi_callback_info info);
    static napi_value UpdateCollaborativeEnabled(
        napi_env env, std::shared_ptr<AudioCollaborativeManagerAsyncContext> &context);

    AudioCollaborativeManager *audioCollaborativeMngr_;
    napi_env env_;
};
} // AudioStandard
} // OHOS
#endif