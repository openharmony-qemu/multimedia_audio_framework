# Audio

## Introduction

The audio framework is used to implement audio-related features, including audio playback, audio recording, volume management, and device management.

**Figure 1** Architecture of the audio framework


![](figures/en-us_image_0000001152315135.png)

### Basic Concepts

-   **Sampling**

    Sampling is a process to obtain discrete-time signals by extracting samples from analog signals in a continuous time domain at a specific interval.

-   **Sampling rate**

    Sampling rate is the number of samples extracted from a continuous signal per second to form a discrete signal. It is measured in Hz. Generally, the human hearing range is from 20 Hz to 20 kHz. Common audio sampling rates include 8 kHz, 11.025 kHz, 22.05 kHz, 16 kHz, 37.8 kHz, 44.1 kHz, 48 kHz, 96 kHz, and 192 kHz.

-   **Channel**

    Channels refer to different spatial positions where independent audio signals are recorded or played. The number of channels is the number of audio sources used during audio recording, or the number of speakers used for audio playback.

-   **Audio frame**

    Audio data is in stream form. For the convenience of audio algorithm processing and transmission, it is generally agreed that a data amount in a unit of 2.5 to 60 milliseconds is one audio frame. This unit is called sampling time, and its length is specific to codecs and the application requirements.

-   **PCM**

    Pulse code modulation (PCM) is a method used to digitally represent sampled analog signals. It converts continuous-time analog signals into discrete-time digital signal samples.

## Directory Structure

The structure of the repository directory is as follows:

```
/foundation/multimedia/audio_standard # Service code of the audio framework
├── frameworks                         # Framework code
│   ├── native                         # Internal native API implementation
│   └── js                             # External JS API implementation
│       └── napi                       # External native API implementation
├── interfaces                         # API code
│   ├── inner_api                      # Internal APIs
│   └── kits                           # External APIs
├── sa_profile                         # Service configuration profile
├── services                           # Service code
├── LICENSE                            # License file
└── bundle.json                        # Build file
```

## Usage Guidelines

### Audio Playback

You can use the APIs provided in the current repository to convert audio data into audible analog signals, play the audio signals using output devices, and manage playback tasks. The following describes how to use the **AudioRenderer** class to develop the audio playback feature:

1.  Call **Create()** with the required stream type to create an **AudioRenderer** instance.

    ```
    AudioStreamType streamType = STREAM_MUSIC; // Stream type example.
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(streamType);
    ```

2.  (Optional) Call the static APIs **GetSupportedFormats()**, **GetSupportedChannels()**, **GetSupportedEncodingTypes()**, and **GetSupportedSamplingRates()** to obtain the supported values of parameters.
3.  Prepare the device and call **SetParams()** to set parameters.

    ```
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    audioRenderer->SetParams(rendererParams);
    ```

4.  (Optional) Call **GetParams(rendererParams)** to obtain the parameters set.
5.  Call **Start()** to start an audio playback task.
6.  Call **GetBufferSize()** to obtain the length of the buffer to be written.

    ```
    audioRenderer->GetBufferSize(bufferLen);
    ```

7.  Call **bytesToWrite()** to read the audio data from the source (such as an audio file) and pass it to a byte stream. You can repeatedly call this API to write rendering data.

    ```
    bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
        bytesWritten += audioRenderer->Write(buffer + bytesWritten, bytesToWrite - bytesWritten);
        if (bytesWritten < 0)
            break;
    }
    ```

8.  Call **Drain()** to clear the streams to be played.
9.  Call **Stop()** to stop the output.
10. After the playback task is complete, call **Release()** to release resources.

The preceding steps describe the basic development scenario of audio playback.


11. Call **SetVolume(float)** and **GetVolume()** to set and obtain the audio stream volume, which ranges from 0.0 to 1.0.

For details, see [**audio_renderer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiorenderer/include/audio_renderer.h) and [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).

### Audio Recording

You can use the APIs provided in the current repository to record audio via an input device, convert the audio into audio data, and manage recording tasks. The following describes how to use the **AudioCapturer** class to develop the audio recording feature:

1.  Call **Create()** with the required stream type to create an **AudioCapturer** instance.

    ```
    AudioStreamType streamType = STREAM_MUSIC;
    std::unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(streamType);
    ```

2.  (Optional) Call the static APIs **GetSupportedFormats()**, **GetSupportedChannels()**, **GetSupportedEncodingTypes()**, and **GetSupportedSamplingRates()** to obtain the supported values of parameters.
3.  Prepare the device and call **SetParams()** to set parameters.

    ```
    AudioCapturerParams capturerParams;
    capturerParams.sampleFormat = SAMPLE_S16LE;
    capturerParams.sampleRate = SAMPLE_RATE_44100;
    capturerParams.channelCount = STEREO;
    capturerParams.encodingType = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);
    ```

4.  (Optional) Call **GetParams(capturerParams)** to obtain the parameters set.
5.  Call **Start()** to start an audio recording task.
6.  Call **GetBufferSize()** to obtain the length of the buffer to be written.

    ```
    audioCapturer->GetBufferSize(bufferLen);
    ```

7.  Call **bytesRead()** to read the captured audio data and convert it to a byte stream. The application will repeatedly call this API to read data until it is manually stopped.

    ```
    // set isBlocking = true/false for blocking/non-blocking read
    bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlocking);
    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
            if (bytesRead < 0) {
                break;
            } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, recFile); // example shows writes the recorded data into a file
            numBuffersToCapture--;
        }
    }
    ```

8.  (Optional) Call **Flush()** to clear the recording stream buffer.
9.  Call **Stop()** to stop recording.
10. After the recording task is complete, call **Release()** to release resources.

For details, see [**audio_capturer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h) and [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).

### Audio Management
You can use the APIs provided in the [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiomanager/include/audio_system_manager.h) to control the volume and devices.
1. Call **GetInstance()** to obtain an **AudioSystemManager** instance.
    ```
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    ```
#### Volume Control
2. Call **GetMaxVolume()** and **GetMinVolume()** to obtain the maximum volume and minimum volume allowed for an audio stream.
    ```
    AudioVolumeType streamType = AudioVolumeType::STREAM_MUSIC;
    int32_t maxVol = audioSystemMgr->GetMaxVolume(streamType);
    int32_t minVol = audioSystemMgr->GetMinVolume(streamType);
    ```
3. Call **SetVolume()** and **GetVolume()** to set and obtain the volume of the audio stream, respectively.
    ```
    int32_t result = audioSystemMgr->SetVolume(streamType, 10);
    int32_t vol = audioSystemMgr->GetVolume(streamType);
    ```
4. Call **SetMute()** and **IsStreamMute** to set and obtain the mute status of the audio stream, respectively.
    ```
    int32_t result = audioSystemMgr->SetMute(streamType, true);
    bool isMute = audioSystemMgr->IsStreamMute(streamType);
    ```
5. Call **SetRingerMode()** and **GetRingerMode()** to set and obtain the ringer mode, respectively. The supported ringer modes are the enumerated values of **AudioRingerMode** defined in [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).
    ```
    int32_t result = audioSystemMgr->SetRingerMode(RINGER_MODE_SILENT);
    AudioRingerMode ringMode = audioSystemMgr->GetRingerMode();
    ```
6. Call **SetMicrophoneMute()** and **IsMicrophoneMute()** to set and obtain the mute status of the microphone, respectively.
    ```
    int32_t result = audioSystemMgr->SetMicrophoneMute(true);
    bool isMicMute = audioSystemMgr->IsMicrophoneMute();
    ```
#### Device Control
7. Call **GetDevices**, **deviceType_**, and **deviceRole_** to obtain information about the audio input and output devices. For details, see the enumerated values of **DeviceFlag**, **DeviceType**, and **DeviceRole** defined in [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).
    ```
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors
        = audioSystemMgr->GetDevices(deviceFlag);
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = audioDeviceDescriptors[0];
    cout << audioDeviceDescriptor->deviceType_;
    cout << audioDeviceDescriptor->deviceRole_;
    ```
8. Call **SetDeviceActive()** and **IsDeviceActive()** to activate or deactivate an audio device and obtain the device activation status, respectively.
     ```
    ActiveDeviceType deviceType = SPEAKER;
    int32_t result = audioSystemMgr->SetDeviceActive(deviceType, true);
    bool isDevActive = audioSystemMgr->IsDeviceActive(deviceType);
    ```
9. (Optional) Call other APIs, such as **IsStreamActive()**, **SetAudioParameter()**, and **GetAudioParameter()**, provided in [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiomanager/include/audio_system_manager.h) if required.
10. Call **AudioManagerNapi::On** to subscribe to system volume changes. If a system volume change occurs, the following parameters are used to notify the application:
**volumeType**: type of the system volume changed.
**volume**: current volume level.
**updateUi**: whether to show the change on the UI. (Set **updateUi** to **true** for a volume increase or decrease event, and set it to **false** for other changes.)
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

#### Audio Scene
11. Call **SetAudioScene()** and **getAudioScene()** to set and obtain the audio scene, respectively.
    ```
    int32_t result = audioSystemMgr->SetAudioScene(AUDIO_SCENE_PHONE_CALL);
    AudioScene audioScene = audioSystemMgr->GetAudioScene();
    ```
For details about the supported audio scenes, see the enumerated values of **AudioScene** defined in [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).
#### Audio Stream Management
You can use the APIs provided in [**audio_stream_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiomanager/include/audio_stream_manager.h) to implement stream management.
1. Call **GetInstance()** to obtain an **AudioSystemManager** instance.
    ```
    AudioStreamManager *audioStreamMgr = AudioStreamManager::GetInstance();
    ```

2. Call **RegisterAudioRendererEventListener()** to register a listener for renderer state changes. A callback will be invoked when the renderer state changes. You can override **OnRendererStateChange()** in the **AudioRendererStateChangeCallback** class.
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

3. Call **RegisterAudioCapturerEventListener()** to register a listener for capturer state changes. A callback will be invoked when the capturer state changes. You can override **OnCapturerStateChange()** in the **AudioCapturerStateChangeCallback** class.
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
4. Call **GetCurrentRendererChangeInfos()** to obtain information about all running renderers, including the client UID, session ID, renderer information, renderer state, and output device details.
    ```
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int32_t currentRendererChangeInfo = audioStreamMgr->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    ```

5. Call **GetCurrentCapturerChangeInfos()** to obtain information about all running capturers, including the client UID, session ID, capturer information, capturer state, and input device details.
    ```
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t currentCapturerChangeInfo = audioStreamMgr->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    ```
    For details, see **audioRendererChangeInfos** and **audioCapturerChangeInfos** in [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/inner_api/native/audiocommon/include/audio_info.h).

6. Call **IsAudioRendererLowLatencySupported()** to check whether low latency is supported.
    ```
    const AudioStreamInfo &audioStreamInfo;
    bool isLatencySupport = audioStreamMgr->IsAudioRendererLowLatencySupported(audioStreamInfo);
    ```
#### Using JavaScript APIs
JavaScript applications can call the audio management APIs to control the volume and devices.  
For details, see [**js-apis-audio.md**](https://gitee.com/openharmony/docs/blob/master/en/application-dev/reference/apis/js-apis-audio.md#audiomanager).

### Bluetooth SCO Call
You can use the APIs provided in [**audio_bluetooth_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/services/include/audio_bluetooth/client/audio_bluetooth_manager.h) to implement Bluetooth calls over synchronous connection-oriented (SCO) links.

1. Call **OnScoStateChanged()** to listen for SCO link state changes.
    ```
    const BluetoothRemoteDevice &device;
    int state;
    void OnScoStateChanged(const BluetoothRemoteDevice &device, int state);
    ```

2. (Optional) Call the static API **RegisterBluetoothScoAgListener()** to register a Bluetooth SCO listener, and call **UnregisterBluetoothScoAgListener()** to unregister the listener when it is no longer required.
## Supported Devices
The following lists the device types supported by the audio framework.

1. **USB Type-C Headset**
   
    A digital headset that consists of its own digital-to-analog converter (DAC) and amplifier that functions as part of the headset.
    
2. **WIRED Headset**

    An analog headset that does not contain any DAC. It can have a 3.5 mm jack or a USB-C socket without DAC.

3. **Bluetooth Headset**

    A Bluetooth Advanced Audio Distribution Mode (A2DP) headset for wireless audio transmission.

4. **Internal Speaker and MIC**

    A device with a built-in speaker and microphone, which are used as default devices for playback and recording, respectively.

## Repositories Involved

[multimedia\_audio\_framework](https://gitee.com/openharmony/multimedia_audio_framework)
