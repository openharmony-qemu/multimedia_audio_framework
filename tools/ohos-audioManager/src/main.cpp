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
static const char* G_PROGRAM_NAME= "";
static bool g_hasSubCommand= false;
static const char* G_TOOL_DESCRIPTION = "Audio management tool for controlling volume, devices, and audio settings. "
    "Used for audio device management and volume control scenarios. "
    "Does not support advanced audio processing or routing configuration.";

static constexpr int32_t SUCCESS = 0;
static constexpr int32_t FAILURE = 1;
static constexpr int32_t MAX_VOLUME = 15;
static constexpr int32_t MIN_VOLUME = 0;
static constexpr int32_t INT_NUM1 = 1;
static constexpr int32_t INT_NUM2 = 2;

void RegisterCommand(const char* name, const char* desc, const char* usage,
    const char* params, const char* examples, CommandHandler handler) {
    gCommands[name] = {name, desc, usage, params, examples, handler};
}

static const std::unordered_map<std::string, AudioVolumeType> gVolumeTypeMap = {
    {"STREAM_MUSIC", AudioVolumeType::STREAM_MUSIC},
    {"STREAM_RING", AudioVolumeType::STREAM_RING},
    {"STREAM_VOICE_CALL", AudioVolumeType::STREAM_VOICE_CALL},
    {"STREAM_VOICE_COMMUNICATION", AudioVolumeType::STREAM_VOICE_COMMUNICATION},
    {"STREAM_VOICE_ASSISTANT", AudioVolumeType::STREAM_VOICE_ASSISTANT},
    {"STREAM_ALARM", AudioVolumeType::STREAM_ALARM},
    {"STREAM_SYSTEM", AudioVolumeType::STREAM_SYSTEM},
    {"STREAM_VOICE_RING", AudioVolumeType::STREAM_VOICE_RING},
    {"STREAM_NAVIGATION", AudioVolumeType::STREAM_NAVIGATION},
    {"STREAM_NOTIFICATION", AudioVolumeType::STREAM_NOTIFICATION},
    {"STREAM_ULTRASONIC", AudioVolumeType::STREAM_ULTRASONIC},
    {"STREAM_ANNOUNCEMENT", AudioVolumeType::STREAM_ANNOUNCEMENT},
    {"STREAM_EMERGENCY", AudioVolumeType::STREAM_EMERGENCY},
    {"STREAM_ALL", AudioVolumeType::STREAM_ALL}
};

static const std::unordered_map<AudioVolumeType, std::string> gVolumeTypeToStringMap = {
    {AudioVolumeType::STREAM_MUSIC, "STREAM_MUSIC"},
    {AudioVolumeType::STREAM_RING, "STREAM_RING"},
    {AudioVolumeType::STREAM_VOICE_CALL, "STREAM_VOICE_CALL"},
    {AudioVolumeType::STREAM_VOICE_COMMUNICATION, "STREAM_VOICE_COMMUNICATION"},
    {AudioVolumeType::STREAM_VOICE_ASSISTANT, "STREAM_VOICE_ASSISTANT"},
    {AudioVolumeType::STREAM_ALARM, "STREAM_ALARM"},
    {AudioVolumeType::STREAM_SYSTEM, "STREAM_SYSTEM"},
    {AudioVolumeType::STREAM_VOICE_RING, "STREAM_VOICE_RING"},
    {AudioVolumeType::STREAM_NAVIGATION, "STREAM_NAVIGATION"},
    {AudioVolumeType::STREAM_NOTIFICATION, "STREAM_NOTIFICATION"},
    {AudioVolumeType::STREAM_ULTRASONIC, "STREAM_ULTRASONIC"},
    {AudioVolumeType::STREAM_ANNOUNCEMENT, "STREAM_ANNOUNCEMENT"},
    {AudioVolumeType::STREAM_EMERGENCY, "STREAM_EMERGENCY"},
    {AudioVolumeType::STREAM_ALL, "STREAM_ALL"}
};

static const std::unordered_map<std::string, DeviceType> gDeviceTypeMap = {
    {"DEVICE_TYPE_INVALID", DeviceType::DEVICE_TYPE_INVALID},
    {"DEVICE_TYPE_EARPIECE", DeviceType::DEVICE_TYPE_EARPIECE},
    {"DEVICE_TYPE_SPEAKER", DeviceType::DEVICE_TYPE_SPEAKER},
    {"DEVICE_TYPE_WIRED_HEADSET", DeviceType::DEVICE_TYPE_WIRED_HEADSET},
    {"DEVICE_TYPE_WIRED_HEADPHONES", DeviceType::DEVICE_TYPE_WIRED_HEADPHONES},
    {"DEVICE_TYPE_BLUETOOTH_SCO", DeviceType::DEVICE_TYPE_BLUETOOTH_SCO},
    {"DEVICE_TYPE_BLUETOOTH_A2DP", DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP},
    {"DEVICE_TYPE_MIC", DeviceType::DEVICE_TYPE_MIC},
    {"DEVICE_TYPE_USB_HEADSET", DeviceType::DEVICE_TYPE_USB_HEADSET},
    {"DEVICE_TYPE_DP", DeviceType::DEVICE_TYPE_DP},
    {"DEVICE_TYPE_REMOTE_CAST", DeviceType::DEVICE_TYPE_REMOTE_CAST},
    {"DEVICE_TYPE_USB_DEVICE", DeviceType::DEVICE_TYPE_USB_DEVICE},
    {"DEVICE_TYPE_HDMI", DeviceType::DEVICE_TYPE_HDMI},
    {"DEVICE_TYPE_LINE_DIGITAL", DeviceType::DEVICE_TYPE_LINE_DIGITAL},
    {"DEVICE_TYPE_REMOTE_DAUDIO", DeviceType::DEVICE_TYPE_REMOTE_DAUDIO},
    {"DEVICE_TYPE_HEARING_AID", DeviceType::DEVICE_TYPE_HEARING_AID},
    {"DEVICE_TYPE_NEARLINK", DeviceType::DEVICE_TYPE_NEARLINK},
    {"DEVICE_TYPE_SYSTEM_PRIVATE", DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE},
    {"DEVICE_TYPE_DEFAULT", DeviceType::DEVICE_TYPE_DEFAULT},
    {"DEVICE_TYPE_NONE", DeviceType::DEVICE_TYPE_NONE}
};

static const std::unordered_map<DeviceType, std::string> gDeviceTypeToStringMap = {
    {DeviceType::DEVICE_TYPE_INVALID, "DEVICE_TYPE_INVALID"},
    {DeviceType::DEVICE_TYPE_EARPIECE, "DEVICE_TYPE_EARPIECE"},
    {DeviceType::DEVICE_TYPE_SPEAKER, "DEVICE_TYPE_SPEAKER"},
    {DeviceType::DEVICE_TYPE_WIRED_HEADSET, "DEVICE_TYPE_WIRED_HEADSET"},
    {DeviceType::DEVICE_TYPE_WIRED_HEADPHONES, "DEVICE_TYPE_WIRED_HEADPHONES"},
    {DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, "DEVICE_TYPE_BLUETOOTH_SCO"},
    {DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, "DEVICE_TYPE_BLUETOOTH_A2DP"},
    {DeviceType::DEVICE_TYPE_MIC, "DEVICE_TYPE_MIC"},
    {DeviceType::DEVICE_TYPE_USB_HEADSET, "DEVICE_TYPE_USB_HEADSET"},
    {DeviceType::DEVICE_TYPE_DP, "DEVICE_TYPE_DP"},
    {DeviceType::DEVICE_TYPE_REMOTE_CAST, "DEVICE_TYPE_REMOTE_CAST"},
    {DeviceType::DEVICE_TYPE_USB_DEVICE, "DEVICE_TYPE_USB_DEVICE"},
    {DeviceType::DEVICE_TYPE_HDMI, "DEVICE_TYPE_HDMI"},
    {DeviceType::DEVICE_TYPE_LINE_DIGITAL, "DEVICE_TYPE_LINE_DIGITAL"},
    {DeviceType::DEVICE_TYPE_REMOTE_DAUDIO, "DEVICE_TYPE_REMOTE_DAUDIO"},
    {DeviceType::DEVICE_TYPE_HEARING_AID, "DEVICE_TYPE_HEARING_AID"},
    {DeviceType::DEVICE_TYPE_NEARLINK, "DEVICE_TYPE_NEARLINK"},
    {DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE, "DEVICE_TYPE_SYSTEM_PRIVATE"},
    {DeviceType::DEVICE_TYPE_DEFAULT, "DEVICE_TYPE_DEFAULT"},
    {DeviceType::DEVICE_TYPE_NONE, "DEVICE_TYPE_NONE"}
};

static const std::unordered_map<std::string, DeviceFlag> gDeviceFlagMap = {
    {"OUTPUT_DEVICES_FLAG", DeviceFlag::OUTPUT_DEVICES_FLAG},
    {"INPUT_DEVICES_FLAG", DeviceFlag::INPUT_DEVICES_FLAG},
    {"ALL_DEVICES_FLAG", DeviceFlag::ALL_DEVICES_FLAG}
};

AudioVolumeType ParseVolumeType(const std::string& typeStr)
{
    auto it = gVolumeTypeMap.find(typeStr);
    if (it != gVolumeTypeMap.end()) {
        return it->second;
    }
    return AudioVolumeType::STREAM_MUSIC;
}

DeviceType ParseDeviceType(const std::string& typeStr)
{
    auto it = gDeviceTypeMap.find(typeStr);
    if (it != gDeviceTypeMap.end()) {
        return it->second;
    }
    return DeviceType::DEVICE_TYPE_NONE;
}

DeviceFlag ParseDeviceFlag(const std::string& flagStr)
{
    auto it = gDeviceFlagMap.find(flagStr);
    if (it != gDeviceFlagMap.end()) {
        return it->second;
    }
    return DeviceFlag::OUTPUT_DEVICES_FLAG;
}

std::string VolumeTypeToString(AudioVolumeType type)
{
    auto it = gVolumeTypeToStringMap.find(type);
    if (it != gVolumeTypeToStringMap.end()) {
        return it->second;
    }
    return "STREAM_MUSIC";
}

std::string DeviceTypeToString(DeviceType type)
{
    auto it = gDeviceTypeToStringMap.find(type);
    if (it != gDeviceTypeToStringMap.end()) {
        return it->second;
    }
    return "DEVICE_TYPE_NONE";
}

int CmdGetVolume(int argc, char** argv)
{
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    std::string typeStr = "STREAM_MUSIC";
    bool hasTypeParam = false;
    const char* validTypes = "Check if volume type is valid";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            typeStr = argv[i + 1];
            hasTypeParam = true;
        }
    }

    if (hasTypeParam) {
        auto it = gVolumeTypeMap.find(typeStr);
        if (it == gVolumeTypeMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid volume type: " + typeStr, validTypes);
        }
        volumeType = it->second;
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t volume = manager->GetVolume(volumeType);
    if (volume < MIN_VOLUME) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get volume",
            "Check if audio service is running and volume type is valid");
    }

    json result;
    result["volumeType"] = VolumeTypeToString(volumeType);
    result["volume"] = volume;
    return OutputSuccess(result);
}

bool IsAlarmOrCallVolumeType(AudioVolumeType volumeType)
{
    return volumeType == AudioVolumeType::STREAM_ALARM ||
        volumeType == AudioVolumeType::STREAM_VOICE_CALL ||
        volumeType == AudioVolumeType::STREAM_VOICE_COMMUNICATION ||
        volumeType == AudioVolumeType::STREAM_ALL;
}

int ParseVolumeArgs(int argc, char** argv, AudioVolumeType& volumeType, int32_t& volume)
{
    std::string typeStr = "STREAM_MUSIC";
    bool hasTypeParam = false;
    bool hasVolumeParam = false;
    const char* validTypes = "Check if volume type is valid";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            typeStr = argv[i + 1];
            hasTypeParam = true;
        } else if (strcmp(argv[i], "--volume") == 0 && i + 1 < argc) {
            volume = atoi(argv[i + 1]);
            hasVolumeParam = true;
        }
    }

    if (!hasVolumeParam) {
        return OutputError("ERR_ARG_MISSING", "Missing required parameter: --volume",
            "Please provide volume value. Example: --volume 50");
    }

    if (volume < MIN_VOLUME || volume > MAX_VOLUME) {
        return OutputError("ERR_INVALID_ARG", "Volume must be between 0 and 15",
            "Please provide a volume value in the range 0-15");
    }

    if (hasTypeParam) {
        auto it = gVolumeTypeMap.find(typeStr);
        if (it == gVolumeTypeMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid volume type: " + typeStr, validTypes);
        }
        volumeType = it->second;
    }

    if (volume == MIN_VOLUME && IsAlarmOrCallVolumeType(volumeType)) {
        return OutputError("ERR_INVALID_ARG", "Cannot set alarm or call volume to 0",
            "Alarm and call volumes must be at least 1");
    }

    return SUCCESS;
}

int CmdSetVolume(int argc, char** argv)
{
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    int32_t volume = -1;

    int parseResult = ParseVolumeArgs(argc, argv, volumeType, volume);
    if (parseResult != SUCCESS) {
        return parseResult;
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t result = manager->SetVolume(volumeType, volume);
    if (result != SUCCESS) {
        return OutputError("ERR_PERMISSION_DENIED",
            "Failed to set volume. System permission may be required",
            "Check if the process has system permission or volume value is valid");
    }

    json resultJson;
    resultJson["volumeType"] = VolumeTypeToString(volumeType);
    resultJson["volume"] = volume;
    return OutputSuccess(resultJson);
}

int CmdGetMaxVolume(int argc, char** argv)
{
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    std::string typeStr = "STREAM_MUSIC";
    bool hasTypeParam = false;
    const char* validTypes = "Check if volume type is valid";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            typeStr = argv[i + 1];
            hasTypeParam = true;
        }
    }

    if (hasTypeParam) {
        auto it = gVolumeTypeMap.find(typeStr);
        if (it == gVolumeTypeMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid volume type: " + typeStr, validTypes);
        }
        volumeType = it->second;
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t maxVolume = manager->GetMaxVolume(volumeType);
    if (maxVolume < MIN_VOLUME) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get max volume",
            "Check if audio service is running and volume type is valid");
    }

    json result;
    result["volumeType"] = VolumeTypeToString(volumeType);
    result["maxVolume"] = maxVolume;
    return OutputSuccess(result);
}

int CmdIsMute(int argc, char** argv)
{
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    std::string typeStr = "STREAM_MUSIC";
    bool hasTypeParam = false;
    const char* validTypes = "Check if volume type is valid";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            typeStr = argv[i + 1];
            hasTypeParam = true;
        }
    }

    if (hasTypeParam) {
        auto it = gVolumeTypeMap.find(typeStr);
        if (it == gVolumeTypeMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid volume type: " + typeStr, validTypes);
        }
        volumeType = it->second;
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

int ParseMuteArgs(int argc, char** argv, AudioVolumeType& volumeType, bool& mute)
{
    std::string typeStr = "STREAM_MUSIC";
    bool hasTypeParam = false;
    const char* validTypes = "Check if volume type is valid";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            typeStr = argv[i + 1];
            hasTypeParam = true;
        } else if (strcmp(argv[i], "--mute") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                mute = (strcmp(argv[i + 1], "true") == 0 || strcmp(argv[i + 1], "1") == 0);
            } else {
                mute = true;
            }
        }
    }

    if (hasTypeParam) {
        auto it = gVolumeTypeMap.find(typeStr);
        if (it == gVolumeTypeMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid volume type: " + typeStr, validTypes);
        }
        volumeType = it->second;
    }

    if (mute && IsAlarmOrCallVolumeType(volumeType)) {
        return OutputError("ERR_INVALID_ARG", "Cannot mute alarm or call volume",
            "Alarm and call volumes cannot be muted");
    }

    return SUCCESS;
}

int CmdSetMute(int argc, char** argv)
{
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    bool mute = false;

    int parseResult = ParseMuteArgs(argc, argv, volumeType, mute);
    if (parseResult != SUCCESS) {
        return parseResult;
    }

    AudioSystemManager* manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to get AudioSystemManager instance",
            "Check if audio service is running");
    }

    int32_t result = manager->SetMute(volumeType, mute);
    if (result != SUCCESS) {
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

int CmdGetDevices(int argc, char** argv)
{
    DeviceFlag deviceFlag = DeviceFlag::OUTPUT_DEVICES_FLAG;
    std::string flagStr = "OUTPUT_DEVICES_FLAG";
    bool hasFlagParam = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--flag") == 0 && i + 1 < argc) {
            flagStr = argv[i + 1];
            hasFlagParam = true;
        }
    }

    if (hasFlagParam) {
        auto it = gDeviceFlagMap.find(flagStr);
        if (it == gDeviceFlagMap.end()) {
            return OutputError("ERR_INVALID_ARG", "Invalid device flag: " + flagStr,
                "Valid: OUTPUT_DEVICES_FLAG, INPUT_DEVICES_FLAG, ALL_DEVICES_FLAG");
        }
        deviceFlag = it->second;
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

int CmdSelectOutputDevice(int argc, char** argv)
{
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
    if (result != SUCCESS) {
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

void ShowGeneralHelp()
{
    CLI_LOG("%s - %s", G_PROGRAM_NAME, G_TOOL_DESCRIPTION);
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s [command] [options]", G_PROGRAM_NAME);
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
    CLI_LOG("  %s --help", G_PROGRAM_NAME);
    for (const auto& pair : gCommands) {
        if (strcmp(pair.first.c_str(), "help") != 0) {
            CLI_LOG("  %s %s --help", G_PROGRAM_NAME, pair.first.c_str());
            break;
        }
    }
}

void ShowCommandHelp(const Command& cmd)
{
    CLI_LOG("%s %s - %s", G_PROGRAM_NAME, cmd.name, cmd.description ? cmd.description : "N/A");

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

int CmdHelp(int argc, char** argv)
{
    std::string targetCmd;
    for (int i = INT_NUM1; i < argc; i++) {
        if (argv[i][0] != '-') {
            targetCmd = argv[i];
            break;
        }
    }

    if (targetCmd.empty()) {
        ShowGeneralHelp();
        return SUCCESS;
    }

    auto it = gCommands.find(targetCmd);
    if (it == gCommands.end()) {
        json response;
        response["status"] = "failed";
        response["errCode"] = "ERR_INVALID_COMMAND";
        response["errMsg"] = "Unknown command: " + targetCmd;
        std::cout << response.dump() << std::endl;
        return FAILURE;
    }

    ShowCommandHelp(it->second);
    return SUCCESS;
}

void RegisterHelpCommand()
{
    RegisterCommand("help", "Show help information",
        "ohos-audioManager help [command]",
        "    command                Optional. Show detailed help for a specific command.",
        "    ohos-audioManager help\n"
        "    ohos-audioManager help get-volume",
        CmdHelp);
}

void RegisterGetVolumeCommand()
{
    RegisterCommand("get-volume", "Get the current volume for a specific volume type",
        "ohos-audioManager get-volume [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager get-volume\n"
        "    ohos-audioManager get-volume --type STREAM_RING\n"
        "    ohos-audioManager get-volume --type STREAM_MUSIC",
        CmdGetVolume);
}

void RegisterSetVolumeCommand()
{
    RegisterCommand("set-volume", "Set the volume for a specific volume type",
        "ohos-audioManager set-volume --volume <value> [--type <type>]",
        "    --volume <value>       Required. Volume value to set (typically 0-100 or 0-15)\n"
        "    --type <type>          Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager set-volume --volume 50\n"
        "    ohos-audioManager set-volume --volume 10 --type STREAM_RING\n"
        "    ohos-audioManager set-volume --volume 80 --type STREAM_MUSIC",
        CmdSetVolume);
}

void RegisterGetMaxVolumeCommand()
{
    RegisterCommand("get-max-volume", "Get the maximum volume for a specific volume type",
        "ohos-audioManager get-max-volume [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager get-max-volume\n"
        "    ohos-audioManager get-max-volume --type STREAM_RING\n"
        "    ohos-audioManager get-max-volume --type STREAM_MUSIC",
        CmdGetMaxVolume);
}

void RegisterIsMuteCommand()
{
    RegisterCommand("is-mute", "Check if a specific volume type is muted",
        "ohos-audioManager is-mute [--type <type>]",
        "    --type <type>           Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager is-mute\n"
        "    ohos-audioManager is-mute --type STREAM_RING\n"
        "    ohos-audioManager is-mute --type STREAM_MUSIC",
        CmdIsMute);
}

void RegisterSetMuteCommand()
{
    RegisterCommand("set-mute", "Set the mute state for a specific volume type",
        "ohos-audioManager set-mute --mute <true|false> [--type <type>]",
        "    --mute <true|false>    Required. Mute state: true to mute, false to unmute\n"
        "    --type <type>          Optional. Volume type. Values: [STREAM_MUSIC, STREAM_RING, "
        "STREAM_VOICE_CALL, STREAM_VOICE_COMMUNICATION, STREAM_VOICE_ASSISTANT, STREAM_ALARM, "
        "STREAM_SYSTEM, STREAM_VOICE_RING, STREAM_NAVIGATION, "
        "STREAM_NOTIFICATION, STREAM_ULTRASONIC, STREAM_ANNOUNCEMENT, STREAM_EMERGENCY, "
        "STREAM_ALL]. Default: STREAM_MUSIC",
        "    ohos-audioManager set-mute --mute true\n"
        "    ohos-audioManager set-mute --mute false --type STREAM_RING\n"
        "    ohos-audioManager set-mute --mute true --type STREAM_MUSIC",
        CmdSetMute);
}

void RegisterGetDevicesCommand()
{
    RegisterCommand("get-devices", "Get the list of audio devices",
        "ohos-audioManager get-devices [--flag <flag>]",
        "    --flag <flag>          Optional. Device flag. Values: [OUTPUT_DEVICES_FLAG, "
        "INPUT_DEVICES_FLAG, ALL_DEVICES_FLAG]. Default: OUTPUT_DEVICES_FLAG",
        "    ohos-audioManager get-devices\n"
        "    ohos-audioManager get-devices --flag OUTPUT_DEVICES_FLAG\n"
        "    ohos-audioManager get-devices --flag INPUT_DEVICES_FLAG",
        CmdGetDevices);
}

void RegisterSelectOutputDeviceCommand()
{
    RegisterCommand("select-output-device", "Select the output audio device",
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

void InitCommands()
{
    RegisterHelpCommand();
    RegisterGetVolumeCommand();
    RegisterSetVolumeCommand();
    RegisterGetMaxVolumeCommand();
    RegisterIsMuteCommand();
    RegisterSetMuteCommand();
    RegisterGetDevicesCommand();
    RegisterSelectOutputDeviceCommand();
    g_hasSubCommand= (gCommands.size() > INT_NUM1);
}

void PrintUsage(const char* prog)
{
    CLI_ERROR("Usage: %s <command> [options...]", prog);
    CLI_ERROR("Run '%s help' for more information", prog);
}

int main(int argc, char** argv)
{
    G_PROGRAM_NAME= argv[0];

    if (argc >= INT_NUM2 && strcmp(argv[1], "--help") == 0) {
        InitCommands();
        char* helpArgv[2] = {argv[0], nullptr};
        CmdHelp(INT_NUM1, helpArgv);
        return SUCCESS;
    }

    if (argc < INT_NUM2) {
        PrintUsage(argv[0]);
        return FAILURE;
    }

    InitCommands();

    std::string cmdName = argv[1];

    for (int i = INT_NUM2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            char* helpArgv[2] = {argv[0], const_cast<char*>(cmdName.c_str())};
            CmdHelp(INT_NUM2, helpArgv);
            return SUCCESS;
        }
    }

    auto it = gCommands.find(cmdName);
    if (it == gCommands.end()) {
        OutputError("ERR_INVALID_COMMAND", "Unknown command: " + cmdName,
                    "Use --help to see available commands");
        return FAILURE;
    }

    return it->second.handler(argc - INT_NUM2, argv + INT_NUM2);
}