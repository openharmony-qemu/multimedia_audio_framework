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
#ifndef LOG_TAG
#define LOG_TAG "AudioEffectServer"
#endif

#include <iostream>
#include <vector>
#include "functional"
#include "memory"
#include <dlfcn.h>
#include "unistd.h"
#include "audio_service_log.h"
#include "audio_effect_server.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

#if (defined(__aarch64__) || defined(__x86_64__))
    constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/sys_prod/lib64/", "/system/lib64/"};
#else
    constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/sys_prod/lib/", "/system/lib/"};
#endif

bool ResolveLibrary(const std::string &path, std::string &resovledPath)
{
    for (auto *libDir: LD_EFFECT_LIBRARY_PATH) {
        std::string candidatePath = std::string(libDir) + "/" + path;
        if (access(candidatePath.c_str(), R_OK) == 0) {
            resovledPath = std::move(candidatePath);
            return true;
        }
    }

    return false;
}

static bool LoadLibrary(const std::string &relativePath, std::shared_ptr<AudioEffectLibEntry> &libEntry) noexcept
{
    std::string absolutePath;
    // find library in adsolutePath
    if (!ResolveLibrary(relativePath, absolutePath)) {
        AUDIO_ERR_LOG("<log error> find library falied in effect directories: %{public}s",
            relativePath.c_str());
        return false;
    }

    void* handle = dlopen(absolutePath.c_str(), 1);
    if (!handle) {
        AUDIO_ERR_LOG("<log error> dlopen lib %{public}s Fail", relativePath.c_str());
        return false;
    } else {
        AUDIO_INFO_LOG("<log info> dlopen lib %{public}s successful", relativePath.c_str());
    }
    dlerror(); // clean existing errors;

    AudioEffectLibrary *audioEffectLibHandle = static_cast<AudioEffectLibrary *>(dlsym(handle,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    if (!audioEffectLibHandle) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s, %{public}p", dlerror(), audioEffectLibHandle);
        dlclose(handle);
        return false;
    }
    AUDIO_INFO_LOG("<log info> dlsym lib %{public}s successful", relativePath.c_str());

    libEntry->audioEffectLibHandle = audioEffectLibHandle;

    return true;
}

static void LoadLibraries(const std::vector<Library> &libs, std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList)
{
    for (Library library: libs) {
        AUDIO_INFO_LOG("<log info> loading %{public}s : %{public}s", library.name.c_str(), library.path.c_str());

        std::shared_ptr<AudioEffectLibEntry> libEntry = std::make_shared<AudioEffectLibEntry>();
        libEntry->libraryName = library.name;

        bool loadLibrarySuccess = LoadLibrary(library.path, libEntry);
        if (!loadLibrarySuccess) {
            AUDIO_ERR_LOG("<log error> loadLibrary fail, please check logs!");

            // hisysevent for load engine error
            std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
                Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_EFFECT_ENGINE_ERROR,
                Media::MediaMonitor::FAULT_EVENT);
            bean->Add("ENGINE_TYPE", Media::MediaMonitor::AUDIO_EFFECT_PROCESS_ENGINE);
            Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);

            continue;
        }

        // Register library load success
        libList.emplace_back(std::move(libEntry));
    }
}

std::shared_ptr<AudioEffectLibEntry> FindLibrary(const std::string &name,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : libList) {
        if (lib->libraryName == name) {
            return lib;
        }
    }

    return nullptr;
}

static bool LoadEffect(const Effect &effect, const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList)
{
    std::shared_ptr<AudioEffectLibEntry> currentLibEntry = FindLibrary(effect.libraryName, libList);
    if (currentLibEntry == nullptr) {
        AUDIO_ERR_LOG("<log error> could not find library %{public}s to load effect %{public}s",
                      effect.libraryName.c_str(), effect.name.c_str());
        return false;
    }
    // check effect
    AudioEffectDescriptor descriptor;
    descriptor.libraryName = effect.libraryName;
    descriptor.effectName = effect.name;

    bool ret = currentLibEntry->audioEffectLibHandle->checkEffect(descriptor);
    if (ret) {
        currentLibEntry->effectName.push_back(effect.name);
    } else {
        AUDIO_ERR_LOG("<log error> the effect %{public}s in lib %{public}s, open check file!",
            effect.name.c_str(), effect.libraryName.c_str());
        return false;
    }

    return true;
}

void CheckEffects(const std::vector<Effect> &effects, const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList,
    std::vector<Effect> &successEffectList)
{
    for (Effect effect: effects) {
        bool ret = LoadEffect(effect, libList);
        if (!ret) {
            AUDIO_ERR_LOG("<log error> LoadEffects have failures, please check log!");
            continue;
        }

        successEffectList.push_back(effect);
    }
}

AudioEffectServer::AudioEffectServer()
{
    AUDIO_INFO_LOG("AudioEffectServer ctor");
}

AudioEffectServer::~AudioEffectServer()
{
}

bool AudioEffectServer::LoadAudioEffects(const std::vector<Library> &libraries, const std::vector<Effect> &effects,
                                         std::vector<Effect> &successEffectList)
{
    // load library
    LoadLibraries(libraries, effectLibEntries_);

    // check effects
    CheckEffects(effects, effectLibEntries_, successEffectList);
    if (successEffectList.size() > 0) {
        return true;
    } else {
        return false;
    }
}

std::vector<std::shared_ptr<AudioEffectLibEntry>> &AudioEffectServer::GetEffectEntries()
{
    return effectLibEntries_;
}

} // namespce AudioStandard
} // namespace OHOS
