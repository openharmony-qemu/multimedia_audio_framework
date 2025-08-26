# 引擎组件

## 简介
引擎部件作为音频组件的一部分，用于实现处理播放以及录制相关音频数据的混音、重采样、格式转换、音效以及跨设备音频流转等功能

**图1** 引擎架构图
TODO

### 架构理念
节点化处理音频数据，每个节点负责相对独立的功能模块，节点之间可以组合链接，实现预期的对音频数据的处理，具有时延低、扩展性强的特点

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
