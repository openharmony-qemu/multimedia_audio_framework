/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <cstdint>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <sys/time.h>
#include <atomic>
#include <cstdio>
#include <queue>
#include <climits>
#include <condition_variable>
#include <charconv>
#include <unistd.h>
#include "securec.h"

#include "audio_info.h"
#include "audio_common_utils.h"

#define AUDIO_MS_PER_SECOND 1000
#define AUDIO_US_PER_SECOND 1000000
#define AUDIO_NS_PER_MILLISECOND 1000000
#define AUDIO_NS_PER_SECOND ((uint64_t)1000000000)

#define UINT32_INVALID_VALUE 0xFFFFFFFF

#define FLOAT_EPS 1e-9f
#define OFFSET_BIT_24 3
#define BIT_DEPTH_TWO 2
#define BIT_8 8
#define BIT_16 16
#define BIT_24 24
#define BIT_32 32
namespace OHOS {
namespace AudioStandard {
const uint32_t STRING_BUFFER_SIZE = 4096;
const size_t MILLISECOND_PER_SECOND = 1000;
const uint32_t AUDIO_ID = 1041;

// Ringer or alarmer dual tone
const size_t AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT = 2;

constexpr int32_t AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT = 40;
constexpr int32_t AUDIO_EFFECT_CHAIN_COUNT_UPPER_LIMIT = 32;

/* Define AudioXcollie timeout flag, consistent with xcollie_define.h in hiviewdfx  */
static constexpr unsigned int AUDIO_XCOLLIE_FLAG_DEFAULT = (~0); // do all callback function
static constexpr unsigned int AUDIO_XCOLLIE_FLAG_NOOP = (0); // do nothing but the caller defined function
static constexpr unsigned int AUDIO_XCOLLIE_FLAG_LOG = (1 << 0); // generate log file
static constexpr unsigned int AUDIO_XCOLLIE_FLAG_RECOVERY = (1 << 1); // die when timeout

class Util {
public:
    static bool IsScoSupportSource(const SourceType sourceType);

    static bool IsDualToneStreamType(const AudioStreamType streamType);

    static bool IsRingerOrAlarmerStreamUsage(const StreamUsage &usage);

    static bool IsRingerAudioScene(const AudioScene &audioScene);

    static uint32_t GetSamplePerFrame(const AudioSampleFormat &format);

    static bool IsBackgroundSourceType(const SourceType sourceType);

    static size_t CalculatePcmSizeFromDurationCeiling(std::chrono::nanoseconds duration,
        uint32_t sampleRate, uint32_t bytesPerSample);
};

class Trace {
public:
    static void Count(const std::string &value, int64_t count);
    // Show if data is silent.
    static void CountVolume(const std::string &value, uint8_t data);
    Trace(const std::string &value);
    void End();
    ~Trace();
private:
    bool isFinished_;
};

class AudioXCollie {
public:
    AudioXCollie(const std::string &tag, uint32_t timeoutSeconds,
        std::function<void(void *)> func = nullptr, void *arg = nullptr, uint32_t flag = 0);
    ~AudioXCollie();
    void CancelXCollieTimer();
private:
    int32_t id_;
    std::string tag_;
    bool isCanceled_;
};

class CheckoutSystemAppUtil {
public:
    static bool CheckoutSystemApp(int32_t uid);
};

class ClockTime {
public:
    static int64_t GetCurNano();
    static int64_t GetRealNano();
    static int64_t GetBootNano();
    static int32_t AbsoluteSleep(int64_t nanoTime);
    static int32_t RelativeSleep(int64_t nanoTime);
    static std::string NanoTimeToString(int64_t nanoTime);

    static void GetAllTimeStamp(std::vector<uint64_t> &timestamps);
};

/**
 * Example 1: Use specific timeout call Check().
 *     WatchTimeout guard("DoSomeWorkFunction", 50 * AUDIO_US_PER_SECOND); // if func cost more than 50 ms, print log
 *     DoSomeWorkFunction();
 *     guard.CheckCurrTimeout();
 * Example 2: Use default timeout(40ms) and auto-check in release.
 *     WatchTimeout guard("DoSomeWorkFunction")
 *     DoSomeWorkFunction();
 */
class WatchTimeout {
public:
    static constexpr int64_t DEFAULT_TIMEOUT_NS = 40 * 1000 * 1000;
    WatchTimeout(const std::string &funcName, int64_t timeoutNs = DEFAULT_TIMEOUT_NS);
    ~WatchTimeout();
    void CheckCurrTimeout();
private:
    const std::string funcName_;
    int64_t timeoutNs_ = 0;
    int64_t startTimeNs_ = 0;
    bool isChecked_ = false;
};

class PermissionUtil {
public:
    static bool IsFoldAble(const char ch);
    static void UpdateBGSet();
    static bool VerifyIsAudio();
    static bool VerifyIsShell();
    static bool VerifyIsSystemApp();
    static bool VerifySelfPermission();
    static bool VerifySystemPermission();
    static bool VerifyPermission(const std::string &permissionName, uint32_t tokenId);
    static bool NeedVerifyBackgroundCapture(int32_t callingUid, SourceType sourceType);
    static bool VerifyBackgroundCapture(uint32_t tokenId, uint64_t fullTokenId);
    static bool NotifyPrivacyStart(uint32_t targetTokenId, uint32_t sessionId);
    static bool NotifyPrivacyStop(uint32_t targetTokenId, uint32_t sessionId);
    static int32_t StartUsingPermission(uint32_t targetTokenId, const char* permission);
    static int32_t StopUsingPermission(uint32_t targetTokenId, const char* permission);
    static bool CheckCallingUidPermission(const std::vector<uid_t> &allowedUids);
};

class SwitchStreamUtil {
public:
    static bool UpdateSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState);
    static bool IsSwitchStreamSwitching(SwitchStreamInfo &info, SwitchState targetState);
private:
    static bool InsertSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState);
    static bool RemoveSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState);
    static bool HandleCreatedSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState);
    static bool HandleStartedSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState);
    static bool HandleSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState);
    static void TimeoutThreadHandleTimeoutRecord(SwitchStreamInfo info, SwitchState targetState);
    static bool RemoveAllRecordBySessionId(uint32_t sessionId);
};

void AdjustStereoToMonoForPCM8Bit(int8_t *data, uint64_t len);
void AdjustStereoToMonoForPCM16Bit(int16_t *data, uint64_t len);
void AdjustStereoToMonoForPCM24Bit(uint8_t *data, uint64_t len);
void AdjustStereoToMonoForPCM32Bit(int32_t *data, uint64_t len);
void AdjustAudioBalanceForPCM8Bit(int8_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM16Bit(int16_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM24Bit(uint8_t *data, uint64_t len, float left, float right);
void AdjustAudioBalanceForPCM32Bit(int32_t *data, uint64_t len, float left, float right);

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b);
void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b);
void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b);
void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b);

std::string GetEncryptStr(const std::string &str);
std::string ConvertNetworkId(const std::string &networkId);
std::string Hide(const std::string &str);

enum ConvertHdiFormat {
    SAMPLE_U8_C = 0,
    SAMPLE_S16_C = 1,
    SAMPLE_S24_C = 2,
    SAMPLE_S32_C = 3,
    SAMPLE_F32_C = 4,
    INVALID_WIDTH_C = -1
}; // same with HdiAdapterFormat

float UpdateMaxAmplitude(ConvertHdiFormat adapterFormat, char *frame, uint64_t replyBytes);
float CalculateMaxAmplitudeForPCM8Bit(int8_t *frame, uint64_t nSamples);
float CalculateMaxAmplitudeForPCM16Bit(int16_t *frame, uint64_t nSamples);
float CalculateMaxAmplitudeForPCM24Bit(char *frame, uint64_t nSamples);
float CalculateMaxAmplitudeForPCM32Bit(int32_t *frame, uint64_t nSamples);

template <typename T>
bool isEqual(T a, T b, double precision = 0.01)
{
    return std::abs(a - b) < precision;
}

// return true if value is not in the array.
template <typename V>
inline bool NotContain(const std::vector<V> &array, const V &value)
{
    return std::find(array.begin(), array.end(), value) == array.end();
}

template <typename T>
bool StringConverter(const std::string &str, T &result);
bool StringConverterFloat(const std::string &str, float &result);

bool SetSysPara(const std::string& key, int32_t value);
template <typename T>
bool GetSysPara(const char *key, T &value);

int32_t GetEngineFlag();

enum AudioDumpFileType {
    AUDIO_APP = 0,
    OTHER_NATIVE_SERVICE = 1,
    AUDIO_PULSE = 2,
};

class DumpFileUtil {
public:
    static constexpr char DUMP_SERVER_PARA[] = "sys.audio.dump.writeserver.enable";
    static constexpr char DUMP_CLIENT_PARA[] = "sys.audio.dump.writeclient.enable";
    static constexpr uint32_t PARAM_VALUE_LENTH = 150;
    static constexpr char BETA_VERSION[] = "beta";
    static void WriteDumpFile(FILE *dumpFile, void *buffer, size_t bufferSize);
    static void CloseDumpFile(FILE **dumpFile);
    static std::map<std::string, std::string> g_lastPara;
    static void OpenDumpFile(std::string para, std::string fileName, FILE **file);
private:
    static FILE *OpenDumpFileInner(std::string para, std::string fileName, AudioDumpFileType fileType);
    static void ChangeDumpFileState(std::string para, FILE **dumpFile, std::string fileName);
};

template <typename...Args>
void AppendFormat(std::string& out, const char* fmt, Args&& ... args)
{
    char buf[STRING_BUFFER_SIZE] = {0};
    int len = ::sprintf_s(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
    if (len <= 0) {
        return;
    }
    out += buf;
}

class AudioInfoDumpUtils {
public:
    static const std::string GetStreamName(AudioStreamType streamType);
    static const std::string GetDeviceTypeName(DeviceType deviceType);
    static const std::string GetConnectTypeName(ConnectType connectType);
    static const std::string GetSourceName(SourceType sourceType);
    static const std::string GetDeviceVolumeTypeName(DeviceVolumeType deviceType);
};

template<typename T>
class ObjectRefMap {
public:
    static std::mutex allObjLock;
    static std::map<T*, uint32_t> refMap;
    static void Insert(T *obj);
    static void Erase(T *obj);
    static T *IncreaseRef(T *obj);
    static void DecreaseRef(T *obj);

    ObjectRefMap(T *obj);
    ~ObjectRefMap();
    T *GetPtr();

private:
    T *obj_ = nullptr;
};

template <typename T>
std::mutex ObjectRefMap<T>::allObjLock;

template <typename T>
std::map<T *, uint32_t> ObjectRefMap<T>::refMap;

template <typename T>
void ObjectRefMap<T>::Insert(T *obj)
{
    std::lock_guard<std::mutex> lock(allObjLock);
    refMap[obj] = 1;
}

template <typename T>
void ObjectRefMap<T>::Erase(T *obj)
{
    std::lock_guard<std::mutex> lock(allObjLock);
    auto it = refMap.find(obj);
    if (it != refMap.end()) {
        refMap.erase(it);
    }
}

template <typename T>
T *ObjectRefMap<T>::IncreaseRef(T *obj)
{
    std::lock_guard<std::mutex> lock(allObjLock);
    if (refMap.count(obj)) {
        refMap[obj]++;
        return obj;
    } else {
        return nullptr;
    }
}

template <typename T>
void ObjectRefMap<T>::DecreaseRef(T *obj)
{
    std::unique_lock<std::mutex> lock(allObjLock);
    if (refMap.count(obj) && --refMap[obj] == 0) {
        refMap.erase(obj);
        lock.unlock();
        delete obj;
        obj = nullptr;
    }
}

template <typename T>
ObjectRefMap<T>::ObjectRefMap(T *obj)
{
    if (obj != nullptr) {
        obj_ = ObjectRefMap::IncreaseRef(obj);
    }
}

template <typename T>
ObjectRefMap<T>::~ObjectRefMap()
{
    if (obj_ != nullptr) {
        ObjectRefMap::DecreaseRef(obj_);
    }
}

template <typename T>
T *ObjectRefMap<T>::GetPtr()
{
    return obj_;
}

template <typename Key, typename T>
auto SafeGetMap(const std::unordered_map<Key, std::shared_ptr<T>>& map, Key key)
    -> std::shared_ptr<T>
{
    auto it = map.find(key);
    return (it != map.end() && it->second) ? it->second : nullptr;
}

std::string GetTime();
std::string GetField(const std::string &src, const char* field, const char sep);
int32_t GetFormatByteSize(int32_t format);

struct SignalDetectAgent {
    bool CheckAudioData(uint8_t *buffer, size_t bufferLen);
    bool DetectSignalData(int32_t *buffer, size_t bufferLen);
    void ResetDetectResult();
    int32_t channels_ = STEREO;
    int32_t sampleRate_ = SAMPLE_RATE_48000;
    int32_t sampleFormat_ = SAMPLE_S16LE;
    int32_t formatByteSize_;
    int32_t lastPeakSignal_ = SHRT_MIN;
    int32_t lastPeakSignalPos_ = 0;
    int32_t blankPeriod_ = 0;
    size_t frameCountIgnoreChannel_;
    bool hasFirstNoneZero_ = false;
    bool blankHaveOutput_ = true;
    bool dspTimestampGot_ = false;
    bool signalDetected_ = false;
    std::string lastPeakBufferTime_ = "";
    std::vector<int32_t> cacheAudioData_;
};

class AudioLatencyMeasurement {
public:
    // static methods, invoked without instantiation in sinks and sources
    static bool CheckIfEnabled();
    AudioLatencyMeasurement(const int32_t &sampleRate, const int32_t &channelCount,
        const int32_t &sampleFormat, const std::string &appName, const uint32_t &sessionId);
    ~AudioLatencyMeasurement();

    // non-static methods, invoked after instantiation in AudioRenderer and AudioCapturer
    void InitSignalData();
    bool MockPcmData(uint8_t *buffer, size_t bufferLen); // mute data and insert signal data
private:
    int32_t format_ = SAMPLE_S16LE;
    int32_t formatByteSize_;
    int32_t sampleRate_;
    int32_t channelCount_;
    uint32_t sessionId_;
    size_t mockedTime_ = 0;
    bool mockThisStream_ = false;
    std::string appName_;
    std::unique_ptr<int16_t[]> signalData_ = nullptr;
};

class LatencyMonitor {
public:
    static LatencyMonitor& GetInstance();
    void ShowTimestamp(bool isRenderer);
    void ShowBluetoothTimestamp();
    void UpdateClientTime(bool isRenderer, std::string &timestamp);
    void UpdateSinkOrSourceTime(bool isRenderer, std::string &timestamp);
    void UpdateDspTime(std::string dspTime);
private:
    std::string rendererMockTime_ = "";
    std::string sinkDetectedTime_ = "";
    std::string dspDetectedTime_ = "";
    std::string capturerDetectedTime_ = "";
    std::string sourceDetectedTime_ = "";
    std::string dspBeforeSmartPa_ = "";
    std::string dspAfterSmartPa_ = "";
    std::string dspMockTime_ = "";
    size_t extraStrLen_ = 0;
};

class AudioDump {
public:
    static AudioDump& GetInstance();
    void SetVersionType(const std::string& versionType);
    std::string GetVersionType();
private:
    AudioDump() {}
    ~AudioDump() {}
    std::string versionType_ = "commercial";
};

template <typename EnumType, typename V>
int32_t GetKeyFromValue(const std::unordered_map<EnumType, V> &map, const V &value)
{
    for (auto it : map) {
        if (it.second == value) {
            return it.first;
        }
    }
    return -1;
}

template <typename T, typename Compare>
bool CasWithCompare(std::atomic<T> &atomicVar, T newValue, Compare compare)
{
    T old = atomicVar;
    do {
        if (!compare(old, newValue)) {
            return false;
        }
    } while (!atomicVar.compare_exchange_weak(old, newValue));

    return true;
}

template <typename T>
class FixedSizeList {
public:
    FixedSizeList(size_t size) : maxSize_(size), currentSize_(0), index_(0)
    {
        data_.resize(size);
    }

    void Add(T value)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_[index_] = value;
        index_ = (index_ + 1) % maxSize_;
        if (currentSize_ < maxSize_) {
            ++currentSize_;
        }
    }

    std::optional<T> FindIf(const std::function<bool(const T&)>& predicate)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        for (size_t i = 0; i < currentSize_; ++i) {
            if (predicate(data_[i])) {
                T result = data_[i];
                RemoveAt(i);
                return result;
            }
        }
        return std::nullopt;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_.clear();
        data_.resize(maxSize_);
        currentSize_ = 0;
        index_ = 0;
    }

    std::vector<T> GetData()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<T> dataInfo;
        for (size_t i = 0; i < currentSize_; ++i) {
            dataInfo.push_back(data_[i]);
        }
        return dataInfo;
    }

private:
    void RemoveAt(size_t position)
    {
        if (position < currentSize_) {
            for (size_t i = position; i < currentSize_ - 1; ++i) {
                data_[i] = data_[i + 1];
            }
            --currentSize_;
            index_ = (index_ - 1 + maxSize_) % maxSize_;
        }
    }

    std::vector<T> data_;
    size_t maxSize_;
    size_t currentSize_;
    size_t index_;
    mutable std::mutex mtx_;  // mutable to allow const methods to lock the mutex
};

enum AudioHdiUniqueIDBase : uint32_t {
    // 0-4 is reserved for other modules
    AUDIO_HDI_RENDER_ID_BASE = 5,
    AUDIO_HDI_CAPTURE_ID_BASE = 6,
};

enum HdiCaptureOffset : uint32_t {
    HDI_CAPTURE_OFFSET_PRIMARY = 1,
    HDI_CAPTURE_OFFSET_FAST = 2,
    HDI_CAPTURE_OFFSET_REMOTE = 3,
    HDI_CAPTURE_OFFSET_REMOTE_FAST = 4,
    HDI_CAPTURE_OFFSET_USB = 5,
    HDI_CAPTURE_OFFSET_EC = 6,
    HDI_CAPTURE_OFFSET_MIC_REF = 7,
    HDI_CAPTURE_OFFSET_WAKEUP = 8,
    HDI_CAPTURE_OFFSET_BLUETOOTH = 9,
    HDI_CAPTURE_OFFSET_ACCESSORY = 10,
    HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION = 11,
};

enum HdiRenderOffset : uint32_t {
    HDI_RENDER_OFFSET_PRIMARY = 1,
    HDI_RENDER_OFFSET_FAST = 2,
    HDI_RENDER_OFFSET_REMOTE = 3,
    HDI_RENDER_OFFSET_REMOTE_FAST = 4,
    HDI_RENDER_OFFSET_BLUETOOTH = 5,
    HDI_RENDER_OFFSET_OFFLOAD = 6,
    HDI_RENDER_OFFSET_MULTICHANNEL = 7,
    HDI_RENDER_OFFSET_DIRECT = 8,
    HDI_RENDER_OFFSET_VOIP = 9,
    HDI_RENDER_OFFSET_DP = 10,
    HDI_RENDER_OFFSET_USB = 11,
    HDI_RENDER_OFFSET_VOIP_FAST = 12,
    HDI_RENDER_OFFSET_EAC3 = 13,
    HDI_RENDER_OFFSET_REMOTE_OFFLOAD = 14,
    HDI_RENDER_OFFSET_HEARING_AID = 15,
};

uint32_t GenerateUniqueID(AudioHdiUniqueIDBase base, uint32_t offset);

void CloseFd(int fd);

int32_t CheckSupportedParams(const AudioStreamInfo &info);

std::vector<std::map<AudioInterrupt, int32_t>> ToIpcInterrupts(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &from);

std::list<std::pair<AudioInterrupt, AudioFocuState>> FromIpcInterrupts(
    const std::vector<std::map<AudioInterrupt, int32_t>> &from);

std::string GetBundleNameByToken(const uint32_t &tokenIdNum);

std::string ConvertToStringForFormat(const AudioSampleFormat format);
std::string ConvertToStringForSampleRate(const AudioSamplingRate sampleRate);
std::string ConvertToStringForChannel(const AudioChannel channel);

uint8_t* ReallocVectorBufferAndClear(std::vector<uint8_t> &buffer, const size_t bufLength);
bool IsInjectEnable();
void SetInjectEnable(bool injectSwitch);

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_UTILS_H
