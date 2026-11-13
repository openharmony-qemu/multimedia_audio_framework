# PCM 格式转换

## 范围

PCM 格式转换器独立于引擎和管线，只处理 PCM 数据，用于采样率、声道布局和采样格式之间的转换。基础接口从 API 26.0.0 起公开，扩展接口从 26.1.0 起为 `@systemapi`。编创引擎和管线的整体模型见 [音频编创.md](../音频编创.md)。

## 设计规格

### 基础转换器（26.0.0，公开）

- `OH_AudioConverter_Create()` 接收输入格式和输出格式，创建转换器实例；实例必须显式调用 `OH_AudioConverter_Destroy()` 销毁。
- 支持 14 档采样率（8、11.025、12、16、22.05、24、32、44.1、48、64、88.2、96、176.4、192 kHz）、32 种声道布局和 5 种采样格式（U8、S16LE、S24LE、S32LE、F32LE）。
- 编码类型只支持 `AUDIO_ENCODING_TYPE_RAW`。
- 输入通过 `OH_AudioConverter_RequestDataCallback()` 拉取，单次回调最多返回 400 KB；回调返回的数据指针必须保持有效直到 `OH_AudioConverter_Process()` 返回。
- 回调状态：`AUDIOCONVERTER_INPUT_HAVE_DATA`（有数据，返回值必须大于 0）、`AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA`（暂无数据，返回 0）、`AUDIOCONVERTER_INPUT_DATA_FINISHED`（输入结束，返回 0）。
- `OH_AudioConverter_Process()` 的输出缓冲区由调用方分配，容量至少容纳一个采样帧（声道数 × 采样字节数），不足返回 `AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL`。
- 输入结束后必须继续调用 `Process()`，直到返回成功且 `outputSize` 为 0，才能确认缓存数据已全部输出；不能仅凭单次 `outputSize` 为 0 判断整个转换完成。

### 扩展转换器（26.1.0，`@systemapi`）

- `OH_AudioSuite_ConverterCreateEx()` 支持任意采样率：范围 [8000, 5644800]，必须是 10 Hz 的整数倍，11025 Hz 例外。
- `OH_AudioSuite_ConverterGetMinInOutSizes()` 返回最小输入字节数和对应输出字节数；`ConverterDirectProcess()` 的 `inData->size` 必须是该最小输入字节数的整数倍，输出容量必须按比例足够。
- `ConverterDirectProcess()` 每次最多输入 1000 ms 音频；同一转换器不能同时使用回调模式和 DirectProcess 模式。
- 采样率不同时，第一次处理和每次 `ConverterFlush()` 后的第一个输出前会填充静音；填充字节数由 `ConverterGetDelaySize()` 查询。
- `ConverterFlush()` 用于快速前后跳、新流重开或数据不连续时清空内部缓存和处理状态；清空后可继续复用转换器，无需销毁重建。
- 扩展接口的错误码增加 202（非系统应用）和 6800104（操作不支持）。

## 约束与常见误区

- 转换器不是"每次调用处理完整段音频"的接口：回调模式需要反复调用 `Process()` 直到缓存排空。
- 输出缓冲区至少为一个采样帧，但更大的容量可以减少调用次数；容量不足返回 `AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL`。
- 回调模式与 DirectProcess 模式互斥，切换前需要重新创建转换器。
- 采样率转换存在固有延迟和静音填充，不能用输出字节数直接推算输入消耗。
- 转换器销毁后不得继续调用 `Process()`；所有回调数据只在下一次调用返回前有效。

## 安全与隐私

- 校验格式字段、采样率范围、声道布局、回调和输出缓冲区长度；拒绝不支持的格式组合。
- 校验输出容量和实际写入量，避免缓冲区溢出。
- 用户音频和转换结果按敏感数据处理，日志不得输出原始 PCM。

## 源码入口

- `frameworks/native/ohaudiosuite/OHAudioSuiteFormatConverter.cpp`
- `services/audio_suite/client/converter/`
- `interfaces/inner_api/native/audiosuite/include/audio_format_converter.h`
- `interfaces/kits/c/audio_suite/native_audio_converter.h`
- 跨仓声明：`interface_sdk_c/multimedia/audio_framework/audio_suite_engine/native_audio_converter.h`
