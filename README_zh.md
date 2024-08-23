# 音频组件<a name="ZH-CN_TOPIC_0000001146901937"></a>

-   [简介](#section119mcpsimp)
    -   [基本概念](#section122mcpsimp)

-   [目录](#section179mcpsimp)
-   [使用说明](#section112738505318)
    -   [音频播放](#section1147510562812)
    -   [音频录制](#section295162052813)
    -   [音频管理](#section645572311287)
        -   [音量控制](#section645572311287_001)
        -   [设备控制](#section645572311287_002)
        -   [音频场景](#section645572311287_003)
        -   [音频流管理](#section645572311287_004)
        -   [JavaScript 用法](#section645572311287_005)
    -   [铃声管理](#section645572311287_006)
    -   [蓝牙SCO呼叫](#section645572311287_007)
-   [支持设备](#section645572311287_008)
-   [相关仓](#section340mcpsimp)

## 简介<a name="section119mcpsimp"></a>

音频组件用于实现音频相关的功能，包括音频播放，录制，音量管理和设备管理。

**图 1**  音频组件架构图<a name="fig483116248288"></a>


![](figures/zh-cn_image_0000001152315135.png)

### 基本概念<a name="section122mcpsimp"></a>

-   **采样**

采样是指将连续时域上的模拟信号按照一定的时间间隔采样，获取到离散时域上离散信号的过程。

-   **采样率**

采样率为每秒从连续信号中提取并组成离散信号的采样次数，单位用赫兹（Hz）来表示。通常人耳能听到频率范围大约在20Hz～20kHz之间的声音。常用的音频采样频率有：8kHz、11.025kHz、22.05kHz、16kHz、37.8kHz、44.1kHz、48kHz、96kHz、192kHz等。

-   **声道**

声道是指声音在录制或播放时在不同空间位置采集或回放的相互独立的音频信号，所以声道数也就是声音录制时的音源数量或回放时相应的扬声器数量。

-   **音频帧**

音频数据是流式的，本身没有明确的一帧帧的概念，在实际的应用中，为了音频算法处理/传输的方便，一般约定俗成取2.5ms\~60ms为单位的数据量为一帧音频。这个时间被称之为“采样时间”，其长度没有特别的标准，它是根据编解码器和具体应用的需求来决定的。

-   **PCM**

PCM（Pulse Code Modulation），即脉冲编码调制，是一种将模拟信号数字化的方法，是将时间连续、取值连续的模拟信号转换成时间离散、抽样值离散的数字信号的过程。

## 目录<a name="section179mcpsimp"></a>

仓目录结构如下：

```
/foundation/multimedia/audio_standard  # 音频组件业务代码
├── frameworks                         # 框架代码
│   ├── native                         # 内部接口实现
│   └── js                             # 外部接口实现
│       └── napi                       # napi 外部接口实现
├── interfaces                         # 接口代码
│   ├── inner_api                      # 内部接口
│   └── kits                           # 外部接口
├── sa_profile                         # 服务配置文件
├── services                           # 服务代码
├── LICENSE                            # 证书文件
└── bundle.json                        # 编译文件
```

## 使用说明<a name="section112738505318"></a>

### 音频播放<a name="section1147510562812"></a>

可以使用此仓库内提供的接口将音频数据转换为音频模拟信号，使用输出设备播放音频信号，以及管理音频播放任务。以下步骤描述了如何使用 **AudioRenderer** 开发音频播放功能：

1.  使用 **Create** 接口和所需流类型来获取 **AudioRenderer** 实例。

    ```
    AudioStreamType streamType = STREAM_MUSIC; // 流类型示例
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(streamType);
    ```

2.  （可选）静态接口 **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates**() 可用于获取支持的参数。
3.  准备设备，调用实例的 **SetParams** 。

    ```
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    audioRenderer->SetParams(rendererParams);
    ```

4.  （可选）使用 audioRenderer->**GetParams**(rendererParams) 来验证 SetParams。
5.  （可选）使用 **SetAudioEffectMode** 和 **GetAudioEffectMode** 接口来设置和获取当前音频流的音效模式。
    ```
    AudioEffectMode effectMode = EFFECT_DEFAULT;
    int32_t result = audioRenderer->SetAudioEffectMode(effectMode);
    AudioEffectMode mode = audioRenderer->GetAudioEffectMode();
    ```
6.  AudioRenderer 实例调用 audioRenderer->**Start**() 函数来启动播放任务。
7.  使用 **GetBufferSize** 接口获取要写入的缓冲区长度。

    ```
    audioRenderer->GetBufferSize(bufferLen);
    ```

8.  从源（例如音频文件）读取要播放的音频数据并将其传输到字节流中。重复调用Write函数写入渲染数据。

    ```
    bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
        bytesWritten += audioRenderer->Write(buffer + bytesWritten, bytesToWrite - bytesWritten);
        if (bytesWritten < 0)
            break;
    }
    ```

9.  调用audioRenderer->**Drain**()来清空播放流。
10.  调用audioRenderer->**Stop**()来停止输出。
11. 播放任务完成后，调用AudioRenderer实例的audioRenderer->**Release**()函数来释放资源。

以上提供了基本音频播放使用场景。


12. 使用 audioRenderer->**SetVolume(float)** 和 audioRenderer->**GetVolume()** 来设置和获取当前音频流音量, 可选范围为 0.0 到 1.0。

提供上述基本音频播放使用范例。更多接口说明请参考[**audio_renderer.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiorenderer/include/audio_renderer.h) 和 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h)。

### 音频录制<a name="section295162052813"></a>

可以使用此仓库内提供的接口，让应用程序可以完成使用输入设备进行声音录制，将语音转换为音频数据，并管理录制的任务。以下步骤描述了如何使用 **AudioCapturer** 开发音频录制功能：

1.  使用Create接口和所需流类型来获取 **AudioCapturer** 实例。

    ```
    AudioStreamType streamType = STREAM_MUSIC;
    std::unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(streamType);
    ```

2.  （可选）静态接口 **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates**() 可用于获取支持的参数。
3.  准备设备，调用实例的 **SetParams** 。

    ```
    AudioCapturerParams capturerParams;
    capturerParams.sampleFormat = SAMPLE_S16LE;
    capturerParams.sampleRate = SAMPLE_RATE_44100;
    capturerParams.channelCount = STEREO;
    capturerParams.encodingType = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);
    ```

4.  （可选）使用 audioCapturer->**GetParams**(capturerParams) 来验证 SetParams()。
5.  AudioCapturer 实例调用 AudioCapturer->**Start**() 函数来启动录音任务。
6.  使用 **GetBufferSize** 接口获取要写入的缓冲区长度。

    ```
    audioCapturer->GetBufferSize(bufferLen);
    ```

7.  读取录制的音频数据并将其转换为字节流。重复调用read函数读取数据直到主动停止。

    ```
    // set isBlocking = true/false for blocking/non-blocking read
    bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlocking);
    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead <= 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, recFile); // example shows writes the recorded data into a file
            numBuffersToCapture--;
        }
    }
    ```

8.  （可选）audioCapturer->**Flush**() 来清空录音流缓冲区。
9.  AudioCapturer 实例调用 audioCapturer->**Stop**() 函数停止录音。
10. 录音任务完成后，调用 AudioCapturer 实例的 audioCapturer->**Release**() 函数释放资源。

提供上述基本音频录制使用范例。更多API请参考[**audio_capturer.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h)和[**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h)。

### 音频管理<a name="section645572311287"></a>
可以使用 [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiomanager/include/audio_system_manager.h) 内的接口来控制音量和设备。
1. 使用 **GetInstance** 接口获取 **AudioSystemManager** 实例.
    ```
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    ```
#### 音量控制<a name="section645572311287_001"></a>
2. 使用 **GetMaxVolume** 和  **GetMinVolume** 接口去查询音频流支持的最大和最小音量等级，在此范围内设置音量。
    ```
    AudioVolumeType streamType = AudioVolumeType::STREAM_MUSIC;
    int32_t maxVol = audioSystemMgr->GetMaxVolume(streamType);
    int32_t minVol = audioSystemMgr->GetMinVolume(streamType);
    ```
3. 使用 **SetVolume** 和 **GetVolume** 接口来设置和获取指定音频流的音量等级。
    ```
    int32_t result = audioSystemMgr->SetVolume(streamType, 10);
    int32_t vol = audioSystemMgr->GetVolume(streamType);
    ```
4. 使用 **SetMute** 和 **IsStreamMute** 接口来设置和获取指定音频流的静音状态。
    ```
    int32_t result = audioSystemMgr->SetMute(streamType, true);
    bool isMute = audioSystemMgr->IsStreamMute(streamType);
5. 使用 **SetRingerMode** 和 **GetRingerMode** 接口来设置和获取铃声模式。参考在 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h)  定义的 **AudioRingerMode** 枚举来获取支持的铃声模式。
    ```
    int32_t result = audioSystemMgr->SetRingerMode(RINGER_MODE_SILENT);
    AudioRingerMode ringMode = audioSystemMgr->GetRingerMode();
    ```
6. 使用 **SetMicrophoneMute** 和 **IsMicrophoneMute** 接口来设置和获取麦克风的静音状态。
    ```
    int32_t result = audioSystemMgr->SetMicrophoneMute(true);
    bool isMicMute = audioSystemMgr->IsMicrophoneMute();
    ```
#### 设备控制<a name="section645572311287_002"></a>
7. 使用 **GetDevices**, **deviceType_** 和 **deviceRole_** 接口来获取音频输入输出设备信息。 参考 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h) 内定义的DeviceFlag, DeviceType 和 DeviceRole 枚举。
    ```
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors
        = audioSystemMgr->GetDevices(deviceFlag);
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = audioDeviceDescriptors[0];
    cout << audioDeviceDescriptor->deviceType_;
    cout << audioDeviceDescriptor->deviceRole_;
    ```
8. 使用 **SetDeviceActive** 和 **IsDeviceActive** 接口去激活/去激活音频设备和获取音频设备激活状态。
     ```
    ActiveDeviceType deviceType = SPEAKER;
    int32_t result = audioSystemMgr->SetDeviceActive(deviceType, true);
    bool isDevActive = audioSystemMgr->IsDeviceActive(deviceType);
    ```
9. 提供其他用途的接口如 **IsStreamActive**, **SetAudioParameter** and **GetAudioParameter**, 详细请参考 [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiomanager/include/audio_system_manager.h)
10. 应用程序可以使用 **AudioManagerNapi::On**注册系统音量的更改。 在此，如果应用程序监听到系统音量更改的事件,就会用以下参数通知应用程序:
volumeType : 更改的系统音量的类型
volume : 当前的音量等级
updateUi : 是否需要显示变化详细信息。（如果音量被增大/减小，将updateUi标志设置为true，在其他情况下，updateUi设置为false）。
    ```
    const audioManager = audio.getAudioManager();

    export default {
      onCreate() {
        audioManager.on('volumeChange', (volumeChange) ==> {
          console.info('volumeType = '+volumeChange.volumeType);
          console.info('volume = '+volumeChange.volume);
          console.info('updateUi = '+volumeChange.updateUi);
        }
      }
    }
    ```

#### 音频场景<a name="section645572311287_003"></a>
11. 使用 **SetAudioScene** 和 **getAudioScene** 接口去更改和检查音频策略。
    ```
    int32_t result = audioSystemMgr->SetAudioScene(AUDIO_SCENE_PHONE_CALL);
    AudioScene audioScene = audioSystemMgr->GetAudioScene();
    ```
有关支持的音频场景，请参阅 **AudioScene** 中的枚举[**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h)。
#### 音频流管理<a name="section645572311287_004"></a>
可以使用[**audio_stream_manager.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiomanager/include/audio_stream_manager.h)提供的接口用于流管理功能。
1. 使用 **GetInstance** 接口获得 **AudioSystemManager** 实例。
    ```
    AudioStreamManager *audioStreamMgr = AudioStreamManager::GetInstance();
    ```

2. 使用 **RegisterAudioRendererEventListener** 为渲染器状态更改注册侦听器。渲染器状态更改回调，该回调将在渲染器流状态更改时调用， 通过重写 **AudioRendererStateChangeCallback** 类中的函数 **OnRendererStateChange** 。
    ```
    const int32_t clientPid;

    class RendererStateChangeCallback : public AudioRendererStateChangeCallback {
    public:
        RendererStateChangeCallback = default;
        ~RendererStateChangeCallback = default;
    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override
    {
        cout<<"OnRendererStateChange entered"<<endl;
    }
    };

    std::shared_ptr<AudioRendererStateChangeCallback> callback = std::make_shared<RendererStateChangeCallback>();
    int32_t state = audioStreamMgr->RegisterAudioRendererEventListener(clientPid, callback);
    int32_t result = audioStreamMgr->UnregisterAudioRendererEventListener(clientPid);
    ```

3. 使用 **RegisterAudioCapturerEventListener** 为捕获器状态更改注册侦听器。 捕获器状态更改回调，该回调将在捕获器流状态更改时调用， 通过重写 **AudioCapturerStateChangeCallback** 类中的函数 **OnCapturerStateChange** 。
    ```
    const int32_t clientPid;

    class CapturerStateChangeCallback : public AudioCapturerStateChangeCallback {
    public:
        CapturerStateChangeCallback = default;
        ~CapturerStateChangeCallback = default;
    void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override
    {
        cout<<"OnCapturerStateChange entered"<<endl;
    }
    };

    std::shared_ptr<AudioCapturerStateChangeCallback> callback = std::make_shared<CapturerStateChangeCallback>();
    int32_t state = audioStreamMgr->RegisterAudioCapturerEventListener(clientPid, callback);
    int32_t result = audioStreamMgr->UnregisterAudioCapturerEventListener(clientPid);
    ```
4. 使用 **GetCurrentRendererChangeInfos** 获取所有当前正在运行的流渲染器信息，包括clientuid、sessionid、renderinfo、renderstate和输出设备详细信息。
    ```
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int32_t currentRendererChangeInfo = audioStreamMgr->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    ```

5. 使用 **GetCurrentCapturerChangeInfos** 获取所有当前正在运行的流捕获器信息，包括clientuid、sessionid、capturerInfo、capturerState和输入设备详细信息。
    ```
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t currentCapturerChangeInfo = audioStreamMgr->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    ```
    有关结构，请参阅[**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h) **audioRendererChangeInfos** 和 **audioCapturerChangeInfos**.

6. 使用 **IsAudioRendererLowLatencySupported** 检查低延迟功能是否支持。
    ```
    const AudioStreamInfo &audioStreamInfo;
    bool isLatencySupport = audioStreamMgr->IsAudioRendererLowLatencySupported(audioStreamInfo);
    ```
7. 使用 **GetEffectInfoArray** 接口查询指定[**StreamUsage**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_info.h)下可以支持的音效模式。
    ```
    AudioSceneEffectInfo audioSceneEffectInfo;
    int32_t status = audioStreamMgr->GetEffectInfoArray(audioSceneEffectInfo,streamUsage);
    ```
    有关支持的音效模式，请参阅[**audio_effect.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/interfaces/inner_api/native/audiocommon/include/audio_effect.h)中的枚举**AudioEffectMode**。

#### JavaScript 用法:<a name="section645572311287_005"></a>
JavaScript应用可以使用系统提供的音频管理接口，来控制音量和设备。\
请参考 [**js-apis-audio.md**](https://gitee.com/openharmony/docs/blob/OpenHarmony-5.0-Release/zh-cn/application-dev/reference/apis-audio-kit/js-apis-audio.md#audiomanager) 来获取音量和设备管理相关JavaScript接口的用法。

### 蓝牙SCO呼叫<a name="section645572311287_007"></a>
可以使用提供的接口 [**audio_bluetooth_manager.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/OpenHarmony-5.0-Release/frameworks/native/bluetoothclient/audio_bluetooth_manager.h) 实现同步连接导向链路（SCO）的蓝牙呼叫。

1. 为监听SCO状态更改，您可以使用 **OnScoStateChanged**.
```
const BluetoothRemoteDevice &device;
int state;
void OnScoStateChanged(const BluetoothRemoteDevice &device, int state);
```

2. (可选) 静态接口 **RegisterBluetoothScoAgListener**(), **UnregisterBluetoothScoAgListener**(), 可用于注册蓝牙SCO的侦听器。
## 支持设备<a name="section645572311287_008"></a>
以下是音频子系统支持的设备类型列表。

1. **USB Type-C Headset**\
    数字耳机，包括自己的DAC（数模转换器）和作为耳机一部分的放大器。
2. **WIRED Headset**\
    模拟耳机内部不包含任何DAC。它可以有3.5mm插孔或不带DAC的C型插孔。
3. **Bluetooth Headset**\
    蓝牙A2DP（高级音频分配模式）耳机，用于无线传输音频。
4. **Internal Speaker and MIC**\
    支持内置扬声器和麦克风，并将分别用作播放和录制的默认设备。

## 相关仓<a name="section340mcpsimp"></a>

[multimedia\_audio\_framework](https://gitee.com/openharmony/multimedia_audio_framework)
