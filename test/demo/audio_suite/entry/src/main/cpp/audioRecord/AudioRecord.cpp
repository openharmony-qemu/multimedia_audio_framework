//
// Created on 2025/12/2.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "AudioRecord.h"
#include <fstream>
#include "audioEffectNode/Output.h"
#include "callback/RegisterCallback.h"
#include "hilog/log.h"
#include "ohaudio/native_audiocapturer.h"
#include "ohaudio/native_audiorenderer.h"
#include "ohaudio/native_audiostreambuilder.h"
#include "ohaudio/native_audiostream_base.h"
#include "timeline/Timeline.h"
#include "utils/Utils.h"
#include "audioEffectNode/Input.h"

static OH_AudioCapturer *audioCapturer;
static OH_AudioStreamBuilder *builder;
static std::string g_filePath = "/data/storage/el2/base/haps/entry/files/oh_test_audio.pcm";
static std::mutex g_bufferMutex;
static std::unique_ptr<uint8_t[]> g_recordBuffer = nullptr;
static size_t g_audioBufferSize = 0;

const int GLOBAL_RESMGR = 0xFF00;
const char *RECORD_TAG = "[AudioEditTestApp_Record_cpp]";
FILE *g_file = nullptr;
// 预留逻辑，最终是用户在设置页面选择以后传过来的，不选的话，也可以是默认值
int32_t g_samplingRate = 44100;
int32_t g_channelCount = 2;
int32_t g_bitsPerSample = 4;
//录制纯音的默认参数
int32_t pure_samplingRate = 44100;
int32_t pure_channelCount = 2;
int32_t pure_bitsPerSample = 4;
int32_t pure_sampleFormat = 4;
OH_AudioStream_SampleFormat g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
size_t g_audioBufferTotalSize = 0;
bool g_real_playing = false;
std::string g_key = "";
bool isPure = false;


void SetAudioData(const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock(g_bufferMutex);
    if (data == nullptr || size == 0) {
        OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, RECORD_TAG, "SetAudioData: invalid data or size (size=%zu)",
                     size);
        return;
    }
    // Calculate the total size of new data
    size_t newTotalSize = g_audioBufferTotalSize + size;
    //
    if (g_recordBuffer == nullptr || g_audioBufferSize < newTotalSize) {
        // Reallocate a larger buffer
        std::unique_ptr<uint8_t[]> newBuffer = std::make_unique<uint8_t[]>(newTotalSize);
        if (g_recordBuffer) {
            // If there is previous data, copy the old data first.
            memcpy(newBuffer.get(), g_recordBuffer.get(), g_audioBufferTotalSize);
        }
        // Swap: New buffer becomes the current buffer
        g_recordBuffer = std::move(newBuffer);
        g_audioBufferSize = newTotalSize; // 更新缓冲区容量
    }
    // Copy the new data to the end of g_audioBuffer.
    memcpy(g_recordBuffer.get() + g_audioBufferTotalSize, data, size);
    // Total Update Size
    g_audioBufferTotalSize += size;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG,
                 "SetAudioData: appended %{public}zu bytes, total size now %{public}zu", size, g_audioBufferTotalSize);
}

int32_t AudioCapturerOnReadData(OH_AudioCapturer *capturer, void *userData, void *buffer, int32_t bufferLen)
{
    size_t count = 1;
    if (fwrite(buffer, bufferLen, count, g_file) != count) {
        printf("buffer fwrite err");
    }
    if (g_real_playing) {
        //缓冲区原有大小+bufferLen，扩展完缓冲区以后，再写入数据，缓冲区初始大小1024
        auto& bufferVec = g_writeDataBufferMap[g_key]; // 获取引用，避免重复查找
        size_t currentSize = bufferVec.size();
        size_t newSize = currentSize + static_cast<size_t>(bufferLen);
        // 扩展 vector 大小（自动填充为 0）
        bufferVec.resize(newSize);
        // 将新数据拷贝到扩展区域的末尾,zhangkan不清楚这里是复制g_record还是复制buffer,修改写法
        std::copy(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + bufferLen, bufferVec.data() + bufferVec.size() - bufferLen);
        auto it = g_writeDataBufferMap.find(g_key);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG,
                 "copy data to g_writeDataBufferMap,it size %{public}d, g_key is %{public}s", it->second.size(), g_key.c_str());
    } else {
        if (buffer != nullptr && bufferLen > 0) {
            SetAudioData(static_cast<const uint8_t *>(buffer), static_cast<size_t>(bufferLen));
        }
    }
    return 0;
}

// init capturer
napi_value AudioCapturerInit(napi_env env, napi_callback_info info)
{
    size_t argc = 6;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status = napi_get_value_bool(env, argv[ARG_0], &g_real_playing);
    std::string inputId;
    status = ParseNapiString(env, argv[ARG_1], inputId);
    std::string mixerId;
    status = ParseNapiString(env, argv[ARG_2], mixerId);
    // 当前音轨仅有1个音频，无混音及output,则需要创建，用完即销毁
    std::string outputId;
    status = ParseNapiString(env, argv[ARG_3], outputId);
    napi_value napiValue;
    setAudioFormat(g_samplingRate, g_channelCount, g_bitsPerSample);
    g_totalSize = g_audioBufferTotalSize;
    long startTime = 0;
    //zhangkan  startTime传递的是currentTime,之前说是音频的开始时间
    status = napi_get_value_int64(env, argv[ARG_4], &startTime);
    status = napi_get_value_bool(env, argv[ARG_5], &isPure);
    g_key = inputId;
    if (startTime > 0) {
        g_key = inputId.c_str() + std::to_string(startTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG, "zhangkan-inputId is %{public}s, mixerId %{public}s, outputId %{public}s", inputId.c_str(), mixerId.c_str(), outputId.c_str());
    g_writeDataBufferMap[g_key] = std::vector<uint8_t>(1024);
    delete [] argv;
    
    if (audioCapturer) {
        OH_AudioCapturer_Release(audioCapturer);
        OH_AudioStreamBuilder_Destroy(builder);
        audioCapturer = nullptr;
        builder = nullptr;
    }
    if (g_file) {
        fclose(g_file);
        g_file = nullptr;
    }
    g_file = fopen(g_filePath.c_str(), "wb");
    // 1. create builder
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStreamBuilder_Create(&builder, type);
    convertFormat();
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG, "zhangkan--g_samplingRate: %{public}d, g_channelCount:%{public}d, g_bitsPerSample: %{public}d", g_samplingRate, g_channelCount, g_bitsPerSample);

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, isPure ? pure_samplingRate : g_samplingRate);
    OH_AudioStreamBuilder_SetChannelCount(builder, isPure ? pure_channelCount : g_channelCount);
    OH_AudioStreamBuilder_SetChannelLayout(builder, SetChannelLayout(isPure ? pure_channelCount : g_channelCount));
    OH_AudioStreamBuilder_SetSampleFormat(builder,  isPure ? ConvertInt2AudioStream(SetSampleFormat(pure_sampleFormat)) : g_sampleFormat);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    callbacks.OH_AudioCapturer_OnStreamEvent = nullptr;
    callbacks.OH_AudioCapturer_OnInterruptEvent = nullptr;
    callbacks.OH_AudioCapturer_OnError = nullptr;
    OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, nullptr);
    // 3. create OH_AudioCapturer
    OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    return nullptr;
}

//调用完AudioCapturerInit，掉实时播放初始化
napi_value MixPlayInitBuffer(napi_env env, napi_callback_info info)
{
    //stop pipeline
    OH_AudioSuite_Result result;
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    std::string mixerId;
    status = ParseNapiString(env, argv[ARG_1], mixerId);
    // 当前音轨仅有1个音频，无混音及output,则需要创建，用完即销毁
    std::string outputId;
    status = ParseNapiString(env, argv[ARG_2], outputId);
    napi_value napiValue;
    setAudioFormat(g_samplingRate, g_channelCount, g_bitsPerSample);
    g_totalSize = g_audioBufferTotalSize;
    long startTime = 0;
    //zhangkan  startTime传递的是currentTime,之前说是音频的开始时间
    status = napi_get_value_int64(env, argv[ARG_3], &startTime);
    g_key = inputId;
    if (startTime > 0) {
        g_key = inputId.c_str() + std::to_string(startTime);
    }
    g_writeDataBufferMap[g_key] = std::vector<uint8_t>(1024);
    AudioAsset asset{
        //相对于时间轴的开始时间
        startTime: startTime,
        endTime: startTime + getAudioDuration(g_audioBufferTotalSize, g_samplingRate, g_channelCount, g_bitsPerSample),
        pcmBufferLength: g_totalSize,
        sampleRate: g_samplingRate,
        channels: g_channelCount,
        bitsPerSample: g_bitsPerSample,
    };
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG, "MixPlayInitBuffer - g_samplingRate: %{public}d, g_channelCount:%{public}d, g_bitsPerSample: %{public}d, inputId:%{public}s", g_samplingRate, g_channelCount, g_bitsPerSample,inputId.c_str());
    AudioTrack track{trackId: inputId, isSilent: false, assets: {{startTime, asset}}, maxEndTime: asset.endTime, currentTime: startTime};
    Timeline::getInstance().addAudioTrack(track);
    // create input node
    CreateInputNode(env, inputId, napiValue, result);
    ManageOutputNodes(env, inputId, outputId, mixerId, result);
    //restart pipeline
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

// start capturer
napi_value AudioCapturerStart(napi_env env, napi_callback_info info)
{
    // start
    OH_AudioCapturer_Start(audioCapturer);
    return nullptr;
}

// stop capturer
napi_value AudioCapturerStop(napi_env env, napi_callback_info info)
{
    OH_AudioCapturer_Stop(audioCapturer);
    return nullptr;
}

napi_value RealPlayRecordBuffer(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    auto it = g_writeDataBufferMap.find(inputId);
    if (it == g_writeDataBufferMap.end()) {
        return nullptr;
    }
    napi_value napiValue = nullptr;
    void *data = nullptr;
    const std::vector<uint8_t>& recordBuffer = it->second;
    size_t dataSize = recordBuffer.size();
    status = napi_create_arraybuffer(env, it->second.size(), &data, &napiValue);
    if (status != napi_ok || data == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG,
            "RealPlayRecordBuffer napi_create_arraybuffer failed, status: %d", static_cast<int>(status));
        delete[] argv;
        return nullptr;
    }
    std::copy(recordBuffer.begin(), recordBuffer.end(), static_cast<uint8_t*>(data));
    delete[] argv;
    setAudioFormat(g_samplingRate, g_channelCount, g_bitsPerSample);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG,
            "RealPlayRecordBuffer g_bitsPerSample is: %{public}d", g_bitsPerSample);
    return napiValue;
}

// release capturer
napi_value AudioCapturerRelease(napi_env env, napi_callback_info info)
{
    if (audioCapturer) {
        OH_AudioCapturer_Release(audioCapturer);
        OH_AudioStreamBuilder_Destroy(builder);
        audioCapturer = nullptr;
        builder = nullptr;
    }
    if (g_file) {
        fclose(g_file);
        g_file = nullptr;
    }
    return nullptr;
}

// 不需要混音的直接调该方法
napi_value GetAudioFrames(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[1];
    napi_status status;

    // get callback info
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "EINVAL", "Failed to get callback info");
        return nullptr;
    }

    // acquire data lock
    std::lock_guard<std::mutex> lock(g_bufferMutex);

    // check for data availability
    if (g_recordBuffer.get() == nullptr || g_audioBufferSize == 0) {
        napi_throw_error(env, "ENODATA", "No audio data available");
        return nullptr;
    }

    napi_value arrayBuffer = nullptr;
    status = napi_create_external_arraybuffer(
        env,
        g_recordBuffer.get(), // original data
        g_audioBufferSize,    // original data size
        [](napi_env env, void *data, void *hint) {}, nullptr, &arrayBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, "ENOMEM", "Failed to create external arraybuffer");
        return nullptr;
    }
    return arrayBuffer;
}

// 处理混音逻辑--把g_audioBuffer转成inputnode，链接到mixer前边
napi_value MixRecordBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG, "mix record buffer start");
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    std::string mixerId;
    status = ParseNapiString(env, argv[ARG_1], mixerId);
    // 当前音轨仅有1个音频，无混音及output,则需要创建，用完即销毁
    std::string outputId;
    status = ParseNapiString(env, argv[ARG_2], outputId);
    napi_value napiValue;
    OH_AudioSuite_Result result;
    setAudioFormat(g_samplingRate, g_channelCount, g_bitsPerSample);
    g_totalSize = g_audioBufferTotalSize;
    long startTime = 1000;
    std::string key = inputId;
    if (startTime > 0) {
        key = inputId.c_str() + std::to_string(startTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, RECORD_TAG, "zhangkan-inputId is %{public}s, mixerId %{public}s, outputId %{public}s", inputId.c_str(), mixerId.c_str(), outputId.c_str());
    StoreTotalBuffToMap(reinterpret_cast<const char *>(g_recordBuffer.get()), g_audioBufferTotalSize, key);
    AudioAsset asset{
        startTime: startTime,
        endTime: startTime + getAudioDuration(g_audioBufferTotalSize, g_sampleFormat, g_channelCount, g_bitsPerSample),
        pcmBufferLength: g_totalSize,
        sampleRate: g_sampleFormat,
        channels: g_channelCount,
        bitsPerSample: g_bitsPerSample,
    };
    AudioTrack track{trackId: inputId, isSilent: false, assets: {{0, asset}}, maxEndTime: asset.endTime, currentTime: 0};
    Timeline::getInstance().addAudioTrack(track);
    // create input node
    CreateInputNode(env, inputId, napiValue, result);
    ManageOutputNodes(env, inputId, outputId, mixerId, result);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

napi_value AudioCapturerPause(napi_env env, napi_callback_info info)
{
    OH_AudioCapturer_Pause(audioCapturer);
    return nullptr;
}

// 转换函数
void convertFormat()
{
    if (g_writeDataBufferMap.size() > 0) {
        g_samplingRate = g_audioFormatInput.samplingRate;
        g_channelCount = g_audioFormatInput.channelCount;
        switch (g_audioFormatInput.sampleFormat) {
            case OH_Audio_SampleFormat::AUDIO_SAMPLE_U8:
                    g_bitsPerSample = 0;
                    g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_U8;
                    break;
            case OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE:
                    g_bitsPerSample = 1;
                    g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S16LE;
                    break;
            case OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE:
                    g_bitsPerSample = 2;
                    g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S24LE;
                    break;
            case OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE:
                    g_bitsPerSample = 3;
                    g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S32LE;
                    break;
            case OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE:
                    g_bitsPerSample = 4;
                    g_sampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
                    break;
        }
    } else {
        std::vector<std::string> audioFormat = {
            std::to_string(g_samplingRate), std::to_string(g_channelCount), std::to_string(g_bitsPerSample)
        };
        CallStringArrayCallback(audioFormat);
    }
}

napi_value ClearRecordBuffer(napi_env env, napi_callback_info info)
{
    // 释放 g_recordBuffer
    g_recordBuffer.reset();
    g_recordBuffer = nullptr;
    g_audioBufferTotalSize = 0;
    return nullptr;
}