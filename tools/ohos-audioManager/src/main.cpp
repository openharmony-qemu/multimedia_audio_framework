/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <memory>
#include <vector>

#include "audio_system_manager.h"
#include "audio_device_descriptor.h"
#include "common.h"

using namespace OHOS::AudioStandard;

typedef std::function<int(int, char**)> CommandHandler;

struct Command {
    const char* name;
    const char* description;
    const char* usage;
    const char* parameters;
    const char* examples;
    CommandHandler handler;
};

static std::unordered_map<std::string, Command> gCommands;
static const char* gProgramName = "";
static bool gHasSubCommand = false;
static const char* gToolDescription = "Audio management tool for controlling volume, devices, and audio settings. "
    "Used for audio device management and volume control scenarios. "
    "Does not support advanced audio processing or routing configuration.";

#define REGISTER_CMD(name, desc, usage, params, examples, handler) \
    gCommands[name] = {name, desc, usage, params, examples, handler}

AudioVolumeType ParseBasicVolumeType(const std::string& typeStr) {
    if (typeStr == "STREAM_MUSIC" || typeStr == "music") {
        return AudioVolumeType::STREAM_MUSIC;
    }
    if (typeStr == "STREAM_RING" || typeStr == "ring") {
        return AudioVolumeType::STREAM_RING;
    }
    if (typeStr == "STREAM_VOICE_CALL" || typeStr == "voice_call") {
        return AudioVolumeType::STREAM_VOICE_CALL;
    }
    if (typeStr == "STREAM_VOICE_COMMUNICATION" || typeStr == "voice_communication") {
        return AudioVolumeType::STREAM_VOICE_COMMUNICATION;
    }
    if (typeStr == "STREAM_VOICE_ASSISTANT" || typeStr == "voice_assistant") {
        return AudioVolumeType::STREAM_VOICE_ASSISTANT;
    }
    if (typeStr == "STREAM_ALARM" || typeStr == "alarm") {
        return AudioVolumeType::STREAM_ALARM;
    }
    if (typeStr == "STREAM_SYSTEM" || typeStr == "system") {
        return AudioVolumeType::STREAM_SYSTEM;
    }
    if (typeStr == "STREAM_ACCESSIBILITY" || typeStr == "accessibility") {
        return AudioVolumeType::STREAM_ACCESSIBILITY;
    }
    return AudioVolumeType::STREAM_MUSIC;
}

AudioVolumeType ParseExtendedVolumeType(const std::string& typeStr) {
    if (typeStr == "STREAM_VOICE_RING" || typeStr == "voice_ring") {
        return AudioVolumeType::STREAM_VOICE_RING;
    }
    if (typeStr == "STREAM_NAVIGATION" || typeStr == "navigation") {
        return AudioVolumeType::STREAM_NAVIGATION;
    }
    if (typeStr == "STREAM_NOTIFICATION" || typeStr == "notification") {
        return AudioVolumeType::STREAM_NOTIFICATION;
    }
    if (typeStr == "STREAM_ULTRASONIC" || typeStr == "ultrasonic") {
        return AudioVolumeType::STREAM_ULTRASONIC;
    }
    if (typeStr == "STREAM_ANNOUNCEMENT" || typeStr == "announcement") {
        return AudioVolumeType::STREAM_ANNOUNCEMENT;
    }
    if (typeStr == "STREAM_EMERGENCY" || typeStr == "emergency") {
        return AudioVolumeType::STREAM_EMERGENCY;
    }
    if (typeStr == "STREAM_ALL" || typeStr == "all") {
        return AudioVolumeType::STREAM_ALL;
    }
    return AudioVolumeType::STREAM_MUSIC;
}

AudioVolumeType ParseVolumeType(const std::string& typeStr) {
    AudioVolumeType basicType = ParseBasicVolumeType(typeStr);
    if (basicType != AudioVolumeType::STREAM_MUSIC || typeStr == "STREAM_MUSIC" || typeStr == "music") {
        return basicType;
    }
    return ParseExtendedVolumeType(typeStr);
}

DeviceType ParseBasicDeviceType(const std::string& typeStr) {
    if (typeStr == "DEVICE_TYPE_INVALID" || typeStr == "invalid") {
        return DeviceType::DEVICE_TYPE_INVALID;
    }
    if (typeStr == "DEVICE_TYPE_EARPIECE" || typeStr == "earpiece") {
        return DeviceType::DEVICE_TYPE_EARPIECE;
    }
    if (typeStr == "DEVICE_TYPE_SPEAKER" || typeStr == "speaker") {
        return DeviceType::DEVICE_TYPE_SPEAKER;
    }
    if (typeStr == "DEVICE_TYPE_WIRED_HEADSET" || typeStr == "wired_headset") {
        return DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    }
    if (typeStr == "DEVICE_TYPE_WIRED_HEADPHONES" || typeStr == "wired_headphones") {
        return DeviceType::DEVICE_TYPE_WIRED_HEADPHONES;
    }
    if (typeStr == "DEVICE_TYPE_BLUETOOTH_SCO" || typeStr == "bluetooth_sco") {
        return DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    }
    if (typeStr == "DEVICE_TYPE_BLUETOOTH_A2DP" || typeStr == "bluetooth_a2dp") {
        return DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    }
    if (typeStr == "DEVICE_TYPE_MIC" || typeStr == "mic") {
        return DeviceType::DEVICE_TYPE_MIC;
    }
    if (typeStr == "DEVICE_TYPE_USB_HEADSET" || typeStr == "usb_headset") {
        return DeviceType::DEVICE_TYPE_USB_HEADSET;
    }
    if (typeStr == "DEVICE_TYPE_DP" || typeStr == "dp") {
        return DeviceType::DEVICE_TYPE_DP;
    }
    if (typeStr == "DEVICE_TYPE_REMOTE_CAST" || typeStr == "remote_cast") {
        return DeviceType::DEVICE_TYPE_REMOTE_CAST;
    }
    if (typeStr == "DEVICE_TYPE_USB_DEVICE" || typeStr == "usb_device") {
        return DeviceType::DEVICE_TYPE_USB_DEVICE;
    }
    return DeviceType::DEVICE_TYPE_NONE;
}

DeviceType ParseExtendedDeviceType(const std::string& typeStr) {
    if (typeStr == "DEVICE_TYPE_HDMI" || typeStr == "hdmi") {
        return DeviceType::DEVICE_TYPE_HDMI;
    }
    if (typeStr == "DEVICE_TYPE_LINE_DIGITAL" || typeStr == "line_digital") {
        return DeviceType::DEVICE_TYPE_LINE_DIGITAL;
    }
    if (typeStr == "DEVICE_TYPE_REMOTE_DAUDIO" || typeStr == "remote_daudio") {
        return DeviceType::DEVICE_TYPE_REMOTE_DAUDIO;
    }
    if (typeStr == "DEVICE_TYPE_HEARING_AID" || typeStr == "hearing_aid") {
        return DeviceType::DEVICE_TYPE_HEARING_AID;
    }
    if (typeStr == "DEVICE_TYPE_NEARLINK" || typeStr == "nearlink") {
        return DeviceType::DEVICE_TYPE_NEARLINK;
    }
    if (typeStr == "DEVICE_TYPE_SYSTEM_PRIVATE" || typeStr == "system_private") {
        return DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE;
    }
    if (typeStr == "DEVICE_TYPE_DEFAULT" || typeStr == "default") {
        return DeviceType::DEVICE_TYPE_DEFAULT;
    }
    if (typeStr == "DEVICE_TYPE_NONE" || typeStr == "none") {
        return DeviceType::DEVICE_TYPE_NONE;
    }
    return DeviceType::DEVICE_TYPE_NONE;
}

DeviceType ParseDeviceType(const std::string& typeStr) {
    DeviceType basicType = ParseBasicDeviceType(typeStr);
    if (basicType != DeviceType::DEVICE_TYPE_NONE) {
        return basicType;
    }
    return ParseExtendedDeviceType(typeStr);
}

DeviceFlag ParseDeviceFlag(const std::string& flagStr) {
    if (flagStr == "OUTPUT_DEVICES_FLAG" || flagStr == "output") {
        return DeviceFlag::OUTPUT_DEVICES_FLAG;
    }
    if (flagStr == "INPUT_DEVICES_FLAG" || flagStr == "input") {
        return DeviceFlag::INPUT_DEVICES_FLAG;
    }
    if (flagStr == "ALL_DEVICES_FLAG" || flagStr == "all") {
        return DeviceFlag::ALL_DEVICES_FLAG;
    }
    return DeviceFlag::OUTPUT_DEVICES_FLAG;
}

std::string BasicVolumeTypeToString(AudioVolumeType type) {
    switch (type) {
        case AudioVolumeType::STREAM_MUSIC:
            return "STREAM_MUSIC";
        case AudioVolumeType::STREAM_RING:
            return "STREAM_RING";
        case AudioVolumeType::STREAM_VOICE_CALL:
            return "STREAM_VOICE_CALL";
        case AudioVolumeType::STREAM_VOICE_COMMUNICATION:
            return "STREAM_VOICE_COMMUNICATION";
        case AudioVolumeType::STREAM_VOICE_ASSISTANT:
            return "STREAM_VOICE_ASSISTANT";
        case AudioVolumeType::STREAM_ALARM:
            return "STREAM_ALARM";
        case AudioVolumeType::STREAM_SYSTEM:
            return "STREAM_SYSTEM";
        case AudioVolumeType::STREAM_ACCESSIBILITY:
            return "STREAM_ACCESSIBILITY";
        default:
            return "STREAM_MUSIC";
    }
}

std::string ExtendedVolumeTypeToString(AudioVolumeType type) {
    switch (type) {
        case AudioVolumeType::STREAM_VOICE_RING:
            return "STREAM_VOICE_RING";
        case AudioVolumeType::STREAM_NAVIGATION:
            return "STREAM_NAVIGATION";
        case AudioVolumeType::STREAM_NOTIFICATION:
            return "STREAM_NOTIFICATION";
        case AudioVolumeType::STREAM_ULTRASONIC:
            return "STREAM_ULTRASONIC";
        case AudioVolumeType::STREAM_ANNOUNCEMENT:
            return "STREAM_ANNOUNCEMENT";
        case AudioVolumeType::STREAM_EMERGENCY:
            return "STREAM_EMERGENCY";
        case AudioVolumeType::STREAM_ALL:
            return "STREAM_ALL";
        default:
            return "STREAM_MUSIC";
    }
}

std::string VolumeTypeToString(AudioVolumeType type) {
    std::string basicResult = BasicVolumeTypeToString(type);
    if (basicResult != "STREAM_MUSIC" || type == AudioVolumeType::STREAM_MUSIC) {
        return basicResult;
    }
    return ExtendedVolumeTypeToString(type);
}

std::string BasicDeviceTypeToString(DeviceType type) {
    switch (type) {
        case DeviceType::DEVICE_TYPE_INVALID:
            return "DEVICE_TYPE_INVALID";
        case DeviceType::DEVICE_TYPE_EARPIECE:
            return "DEVICE_TYPE_EARPIECE";
        case DeviceType::DEVICE_TYPE_SPEAKER:
            return "DEVICE_TYPE_SPEAKER";
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
            return "DEVICE_TYPE_WIRED_HEADSET";
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
            return "DEVICE_TYPE_WIRED_HEADPHONES";
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            return "DEVICE_TYPE_BLUETOOTH_SCO";
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            return "DEVICE_TYPE_BLUETOOTH_A2DP";
        case DeviceType::DEVICE_TYPE_MIC:
            return "DEVICE_TYPE_MIC";
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
            return "DEVICE_TYPE_USB_HEADSET";
        case DeviceType::DEVICE_TYPE_DP:
            return "DEVICE_TYPE_DP";
        case DeviceType::DEVICE_TYPE_REMOTE_CAST:
            return "DEVICE_TYPE_REMOTE_CAST";
        case DeviceType::DEVICE_TYPE_USB_DEVICE:
            return "DEVICE_TYPE_USB_DEVICE";
        default:
            return "DEVICE_TYPE_NONE";
    }
}

std::string ExtendedDeviceTypeToString(DeviceType type) {
    switch (type) {
        case DeviceType::DEVICE_TYPE_HDMI:
            return "DEVICE_TYPE_HDMI";
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
            return "DEVICE_TYPE_LINE_DIGITAL";
        case DeviceType::DEVICE_TYPE_REMOTE_DAUDIO:
            return "DEVICE_TYPE_REMOTE_DAUDIO";
        case DeviceType::DEVICE_TYPE_HEARING_AID:
            return "DEVICE_TYPE_HEARING_AID";
        case DeviceType::DEVICE_TYPE_NEARLINK:
            return "DEVICE_TYPE_NEARLINK";
        case DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE:
            return "DEVICE_TYPE_SYSTEM_PRIVATE";
        case DeviceType::DEVICE_TYPE_DEFAULT:
            return "DEVICE_TYPE_DEFAULT";
        case DeviceType::DEVICE_TYPE_NONE:
            return "DEVICE_TYPE_NONE";
        default:
            return "DEVICE_TYPE_NONE";
    }
}

std::string DeviceTypeToString(DeviceType type) {
    std::string basicResult = BasicDeviceTypeToString(type);
    if (basicResult != "DEVICE_TYPE_NONE" || type == DeviceType::DEVICE_TYPE_NONE) {
        return basicResult;
    }
    return ExtendedDeviceTypeToString(type);
}

int CmdGetVolume(int argc, char** argv) {
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            volumeType = ParseVolumeType(argv[i + 1]);
        }
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t volume = manager->GetVolume(volumeType);
    if (volume < 0) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get volume",
            "Check if audio service is running and volume type is valid");
    }

    json result;
    result["volumeType"] = VolumeTypeToString(volumeType);
    result["volume"] = volume;
    return OutputSuccess(result);
}

int CmdSetVolume(int argc, char** argv) {
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    int32_t volume = -1;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            volumeType = ParseVolumeType(argv[i + 1]);
        } else if (strcmp(argv[i], "--volume") == 0 && i + 1 < argc) {
            volume = atoi(argv[i + 1]);
        }
    }

    if (volume < 0) {
        return OutputError("ERR_ARG_MISSING", "Missing required parameter: --volume",
            "Please provide volume value. Example: --volume 50");
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t result = manager->SetVolume(volumeType, volume);
    if (result != 0) {
        return OutputError("ERR_PERMISSION_DENIED",
            "Failed to set volume. System permission may be required",
            "Check if the process has system permission or volume value is valid");
    }

    json resultJson;
    resultJson["volumeType"] = VolumeTypeToString(volumeType);
    resultJson["volume"] = volume;
    return OutputSuccess(resultJson);
}

int CmdGetMaxVolume(int argc, char** argv) {
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            volumeType = ParseVolumeType(argv[i + 1]);
        }
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t maxVolume = manager->GetMaxVolume(volumeType);
    if (maxVolume < 0) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get max volume",
            "Check if audio service is running and volume type is valid");
    }

    json result;
    result["volumeType"] = VolumeTypeToString(volumeType);
    result["maxVolume"] = maxVolume;
    return OutputSuccess(result);
}

int CmdIsMute(int argc, char** argv) {
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            volumeType = ParseVolumeType(argv[i + 1]);
        }
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    bool isMute = manager->IsStreamMute(volumeType);

    json result;
    result["volumeType"] = VolumeTypeToString(volumeType);
    result["isMute"] = isMute;
    return OutputSuccess(result);
}

int CmdSetMute(int argc, char** argv) {
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    bool mute = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            volumeType = ParseVolumeType(argv[i + 1]);
        } else if (strcmp(argv[i], "--mute") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                mute = (strcmp(argv[i + 1], "true") == 0 || strcmp(argv[i + 1], "1") == 0);
            } else {
                mute = true;
            }
        }
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t result = manager->SetMute(volumeType, mute);
    if (result != 0) {
        return OutputError("ERR_PERMISSION_DENIED",
            "Failed to set mute. Permission may be required for STREAM_RING or STREAM_VOICE_RING type",
            "Check if the process has required permissions. STREAM_RING and STREAM_VOICE_RING types "
            "require ACCESS_NOTIFICATION_POLICY_PERMISSION");
    }

    json resultJson;
    resultJson["volumeType"] = VolumeTypeToString(volumeType);
    resultJson["mute"] = mute;
    return OutputSuccess(resultJson);
}

int CmdGetDevices(int argc, char** argv) {
    DeviceFlag deviceFlag = DeviceFlag::OUTPUT_DEVICES_FLAG;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--flag") == 0 && i + 1 < argc) {
            deviceFlag = ParseDeviceFlag(argv[i + 1]);
        }
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = manager->GetDevices(deviceFlag);

    json result;
    result["deviceFlag"] = static_cast<int>(deviceFlag);
    result["devices"] = json::array();

    for (const auto& device : devices) {
        json deviceJson;
        deviceJson["deviceType"] = static_cast<int>(device->deviceType_);
        result["devices"].push_back(deviceJson);
    }

    return OutputSuccess(result);
}

int CmdSelectOutputDevice(int argc, char** argv) {
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            DeviceType deviceType = ParseDeviceType(argv[i + 1]);
            auto descriptor = std::make_shared<AudioDeviceDescriptor>();
            descriptor->deviceType_ = deviceType;
            audioDeviceDescriptors.push_back(descriptor);
        }
    }

    if (audioDeviceDescriptors.empty()) {
        return OutputError("ERR_ARG_MISSING", "Missing required parameter: --type",
            "Please provide at least one device type. Example: --type DEVICE_TYPE_SPEAKER");
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t result = manager->SelectOutputDevice(audioDeviceDescriptors);
    if (result != 0) {
        return OutputError("ERR_PERMISSION_DENIED", "Failed to select output device. "
            "System permission is required",
            "Check if the process has system permission and device type is valid");
    }

    json resultJson;
    resultJson["devices"] = json::array();
    for (const auto& device : audioDeviceDescriptors) {
        resultJson["devices"].push_back(static_cast<int>(device->deviceType_));
    }
    return OutputSuccess(resultJson);
}

void ShowGeneralHelp() {
    CLI_LOG("%s - %s", gProgramName, gToolDescription);
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s [command] [options]", gProgramName);
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("  --help                  Display this help message");
    CLI_LOG("");
    CLI_LOG("SubCommands:");
    for (const auto& pair : gCommands) {
        CLI_LOG("  %-22s %s", pair.first.c_str(), pair.second.description ? pair.second.description : "");
    }
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  %s --help", gProgramName);
    for (const auto& pair : gCommands) {
        if (strcmp(pair.first.c_str(), "help") != 0) {
            CLI_LOG("  %s %s --help", gProgramName, pair.first.c_str());
            break;
        }
    }
}

void ShowCommandHelp(const Command& cmd) {
    CLI_LOG("%s %s - %s", gProgramName, cmd.name, cmd.description ? cmd.description : "N/A");

    if (cmd.usage) {
        CLI_LOG("");
        CLI_LOG("Usage:");
        CLI_LOG("  %s", cmd.usage);
    }

    if (cmd.parameters) {
        CLI_LOG("");
        CLI_LOG("Parameters:");
        CLI_LOG("%s", cmd.parameters);
    }
    CLI_LOG("    --help                 Display this help message");

    if (cmd.examples) {
        CLI_LOG("");
        CLI_LOG("Examples:");
        CLI_LOG("%s", cmd.examples);
    }
}

int CmdHelp(int argc, char** argv) {
    std::string targetCmd;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            targetCmd = argv[i];
            break;
        }
    }

    if (targetCmd.empty()) {
        ShowGeneralHelp();
        return 0;
    }

    auto it = gCommands.find(targetCmd);
    if (it == gCommands.end()) {
        json response;
        response["status"] = "failed";
        response["errCode"] = "ERR_INVALID_COMMAND";
        response["errMsg"] = "Unknown command: " + targetCmd;
        std::cout << response.dump() << std::endl;
        return 1;
    }

    ShowCommandHelp(it->second);
    return 0;
}

void RegisterHelpCommand() {
    REGISTER_CMD("help", "Show help information",
        "ohos-audioManager help [command]",
        "    command                Optional. Show detailed help for a specific command.",
        "    ohos-audioManager help\n"
        "    ohos-audioManager help get-volume",
        CmdHelp);
}

void RegisterGetVolumeCommand() {
    REGISTER_CMD("get-volume", "Get the current volume for a specific volume type",
        "ohos-audioManager get-volume [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_ACCESSIBILITY, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager get-volume\n"
        "    ohos-audioManager get-volume --type STREAM_RING\n"
        "    ohos-audioManager get-volume --type STREAM_MUSIC",
        CmdGetVolume);
}

void RegisterSetVolumeCommand() {
    REGISTER_CMD("set-volume", "Set the volume for a specific volume type",
        "ohos-audioManager set-volume --volume <value> [--type <type>]",
        "    --volume <value>       Required. Volume value to set (typically 0-100 or 0-15)\n"
        "    --type <type>          Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_ACCESSIBILITY, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager set-volume --volume 50\n"
        "    ohos-audioManager set-volume --volume 10 --type STREAM_RING\n"
        "    ohos-audioManager set-volume --volume 80 --type STREAM_MUSIC",
        CmdSetVolume);
}

void RegisterGetMaxVolumeCommand() {
    REGISTER_CMD("get-max-volume", "Get the maximum volume for a specific volume type",
        "ohos-audioManager get-max-volume [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_ACCESSIBILITY, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager get-max-volume\n"
        "    ohos-audioManager get-max-volume --type STREAM_RING\n"
        "    ohos-audioManager get-max-volume --type STREAM_MUSIC",
        CmdGetMaxVolume);
}

void RegisterIsMuteCommand() {
    REGISTER_CMD("is-mute", "Check if a specific volume type is muted",
        "ohos-audioManager is-mute [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_ACCESSIBILITY, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager is-mute\n"
        "    ohos-audioManager is-mute --type STREAM_RING\n"
        "    ohos-audioManager is-mute --type STREAM_MUSIC",
        CmdIsMute);
}

void RegisterSetMuteCommand() {
    REGISTER_CMD("set-mute", "Set the mute state for a specific volume type",
        "ohos-audioManager set-mute --mute <true|false> [--type <type>]",
        "    --mute <true|false>    Required. Mute state: true to mute, false to unmute\n"
        "    --type <type>          Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_ACCESSIBILITY, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager set-mute --mute true\n"
        "    ohos-audioManager set-mute --mute false --type STREAM_RING\n"
        "    ohos-audioManager set-mute --mute true --type STREAM_MUSIC",
        CmdSetMute);
}

void RegisterGetDevicesCommand() {
    REGISTER_CMD("get-devices", "Get the list of audio devices",
        "ohos-audioManager get-devices [--flag <flag>]",
        "    --flag <flag>          Optional. Device flag. Values: [OUTPUT_DEVICES_FLAG, "
        "INPUT_DEVICES_FLAG, ALL_DEVICES_FLAG]. Default: OUTPUT_DEVICES_FLAG",
        "    ohos-audioManager get-devices\n"
        "    ohos-audioManager get-devices --flag OUTPUT_DEVICES_FLAG\n"
        "    ohos-audioManager get-devices --flag INPUT_DEVICES_FLAG",
        CmdGetDevices);
}

void RegisterSelectOutputDeviceCommand() {
    REGISTER_CMD("select-output-device", "Select the output audio device",
        "ohos-audioManager select-output-device --type <deviceType>",
        "    --type <deviceType>    Required. Device type. Values: [DEVICE_TYPE_INVALID, "
        "DEVICE_TYPE_EARPIECE, DEVICE_TYPE_SPEAKER, DEVICE_TYPE_WIRED_HEADSET, "
        "DEVICE_TYPE_WIRED_HEADPHONES, DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_A2DP, "
        "DEVICE_TYPE_MIC, DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_DP, DEVICE_TYPE_REMOTE_CAST, "
        "DEVICE_TYPE_USB_DEVICE, DEVICE_TYPE_HDMI, DEVICE_TYPE_LINE_DIGITAL, "
        "DEVICE_TYPE_REMOTE_DAUDIO, DEVICE_TYPE_HEARING_AID, DEVICE_TYPE_NEARLINK, "
        "DEVICE_TYPE_SYSTEM_PRIVATE, DEFAULT]. Can be specified multiple times",
        "    ohos-audioManager select-output-device --type DEVICE_TYPE_SPEAKER\n"
        "    ohos-audioManager select-output-device --type DEVICE_TYPE_BLUETOOTH_A2DP\n"
        "    ohos-audioManager select-output-device --type DEFAULT",
        CmdSelectOutputDevice);
}

void InitCommands() {
    RegisterHelpCommand();
    RegisterGetVolumeCommand();
    RegisterSetVolumeCommand();
    RegisterGetMaxVolumeCommand();
    RegisterIsMuteCommand();
    RegisterSetMuteCommand();
    RegisterGetDevicesCommand();
    RegisterSelectOutputDeviceCommand();
    gHasSubCommand = (gCommands.size() > 1);
}

void PrintUsage(const char* prog) {
    CLI_ERROR("Usage: %s <command> [options...]", prog);
    CLI_ERROR("Run '%s help' for more information", prog);
}

int main(int argc, char** argv) {
    gProgramName = argv[0];

    if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        InitCommands();
        char* helpArgv[2] = {argv[0], nullptr};
        CmdHelp(1, helpArgv);
        return 0;
    }

    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    InitCommands();

    std::string cmdName = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            char* helpArgv[2] = {argv[0], const_cast<char*>(cmdName.c_str())};
            CmdHelp(2, helpArgv);
            return 0;
        }
    }

    auto it = gCommands.find(cmdName);
    if (it == gCommands.end()) {
        OutputError("ERR_INVALID_COMMAND", "Unknown command: " + cmdName,
                    "Use --help to see available commands");
        return 1;
    }

    return it->second.handler(argc - 2, argv + 2);
}