# 引擎组件

## 简介
引擎部件作为音频组件的一部分，用于实现处理播放以及录制相关音频数据的混音、重采样、格式转换、音效以及跨设备音频流转等功能

**图1** 引擎架构图
TODO

### 架构理念
架构特点是以节点的形化处理音频数据，每个节点负责相对独立的功能模块，节点之间可以组合链接，实现预期的对音频数据的处理，具有时延低、扩展性强的优势。

## 目录
部件目录结构如下
```
├─buffer                               # 缓存代码
├─dfx                                  # 维测代码
├─manager                              # 管理代码  
│  ├─include
│  └─src
├─node                                 # 节点代码
│  ├─include
│  └─src
├─plugin                               # 算法插件
│  ├─channel_converter
│  │  ├─include
│  │  └─src
│  └─resample
│      ├─include
│      └─proresampler
├─simd                                 # 指令优化
├─test                                 # 单元测试
│  └─unittest
│      ├─common
│      ├─dfx
│      ├─manager
│      ├─node
│      ├─plugin
│      │  ├─channel_converter
│      │  └─proresampler
│      ├─resource
│      └─utils
└─utils                                # 工具代码
```

## 使用说明
### 普通播放
以下步骤提供了在普通播放场景下如何使用引擎能力播放音频流的方法
1. 使用**HpaeManager**实例的**CreateStream**接口，根据音频流信息创建播放流实例
    ```
    HpaeStreamInfo streamInfo;                          // 音频流信息
    streamInfo.streamType = STREAM_MUSIC;               // 示例，流类型为音乐
    auto &hpaeManager = IHpaeManager::GetHpaeManager();
    int32_t ret = hpaeManager.CreateStream(streamInfo);
    ```
2. **HpaeManager**实例的注册状态回调以及写数据回调接口
    ```
    int32_t ret;
    ret = hpaeManager.RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, shared_from_this());  // 注册状态回调
    ret = hpaeManager.RegisterWriteCallback(streamInfo.sessionId, shared_from_this());                                // 注册写数据回调
    ```
3. 使用**HpaeManager**实例的**Start**接口，根据音频流id启动对应音频流的播放
    ```
    uint32_t sessionId = 123456;                        // 音频流Id示例
    int32_t ret = IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_PLAY, sessionId);
    ```
4. 使用**HpaeManager**实例的**Stop**接口，停止音频流的播放
    ```
    int32_t ret = IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, sessionId);
    ```

5. (可选) **Drain**接口，将缓存数据播放完毕，一般调用**Stop**前调用

6. (可选) **Flush**接口，清理缓存数据，一般用于seek场景

7.  播放结束后，使用**HpaeManager**实例的**DestroyStream**接口，释放对应音频流以及资源
    ```
    int32_t ret = IHpaeManager::GetHpaeManager().DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, sessionId);
    ```
提供上述基本音频播放使用范例。更多接口请参考[**i_hpae_manager.h**](https://gitee.com/openharmony/multimedia_audio_framework/blob/master/services/audio_engine/manager/include/i_hpae_manager.h)。


## 节点规格说明

1. **HpaeSinkInputNode**

    作为引擎数据流入的第一个节点（数据入口），主要负责读取客户端的音频数据（回调方式），以及将数据位深转化为32位浮点的功能，本身不对数据做任何处理。一条音频流对应一个SinkInputNode，**生命周期跟随音频流的生命周期**。

2. **HpaeAudioFormatConverterNode**

    负责采样率、位深以及声道数转化

    ProcessCluster中的ConverterNode与音频流一一对应，**在调用start时创建，在调用pause或者stop时被销毁**
    
    OutputCluster中的ConverterNode与ProcessCluster一一对应，**生命周期跟随ProcessCluster**

3. **HpaeLoudnessGainNode**

    负责调节响度增益，普通通路下不对数据做任何处理，一条音频流对应一个LoudnessGainNode，**在调用start时创建，在调用pause或者stop时被销毁**

4. **HpaeGainNode**

    负责音量处理以及淡入淡出，一条音频流对应一个GainNode，**在调用start时创建，在调用pause或者stop时被销毁**

5. **HpaeMixerNode**

    负责混音处理，绑定ProcessCluster或者OutputCluster


6. **HpaeRenderEffectNode**

    负责音效处理，绑定ProcessCluster

7. **HpaeSinkOutputNode**

    负责与南向HDI对接，一条通路对应一个SinkOutputNode，**生命周期跟随OutputCluster**


## 数据流转以及节点连接

**图2** 数据流转以及节点连接图


其中，节点的连接时机是在调用**start**函数时，节点的断连时机是在调用**pause**或者**stop**