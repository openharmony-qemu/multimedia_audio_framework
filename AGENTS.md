# 音频框架仓库协作指南

## 适用范围

本仓库是 OpenHarmony 多媒体音频框架，包含音频公共接口、原生和多语言绑定、音频策略服务、音频流服务、音频引擎、设备路由、音效、提示音、播放采集以及相关测试。

仓库主要围绕以下职责组织：

1. API 接口定义。
2. 客户端流对象和管理器实现。
3. 系统服务、IPC 接口和系统能力。
4. 音频策略、设备、引擎及硬件适配。

## 仓库目录

- `interfaces/`：对外 Kit 和内部 API 声明。
- `frameworks/`：原生、N-API、CJ、Taihe、OpenSL ES 等框架实现。
- `services/audio_policy/`：音频策略、焦点、路由、设备、音量、音效和配置。
- `services/audio_service/`：音频流管理、流生命周期、缓冲区、端点和引擎接入。
- `services/audio_engine/`：引擎缓冲区、节点、管理器、插件、SIMD 和故障诊断。
- `services/audio_suite/`：可选音频编创能力。
- `plugins/`：跨平台或产品适配插件。
- `sa_profile/`：音频系统能力注册配置。
- `test/`：示例、模块测试、单元测试、基准测试和模糊测试。
- `docs/knowledge/`：音频框架知识库。

知识库入口见 [docs/knowledge/知识库索引.md](docs/knowledge/知识库索引.md)。

## 任务路由

| 任务或修改路径 | 必读文档和源码入口 |
| --- | --- |
| 修改 Renderer、sink、播放或输出 HDI 适配 | [音频播放.md](docs/knowledge/audio_stream/audio_renderer/音频播放.md)、`frameworks/native/audiorenderer/`、`services/audio_service/`、`frameworks/native/hdiadapter_new/sink/` |
| 修改 Capturer、source、EC、录制音效链或输入 HDI 适配 | [音频录制.md](docs/knowledge/audio_stream/audio_capturer/音频录制.md)、[安全与隐私.md](docs/knowledge/common/security_privacy/安全与隐私.md)、`services/audio_service/`、`frameworks/native/hdiadapter_new/source/` |
| 修改 pipe 选择、并发资源协调或通路恢复 | [通路选择.md](docs/knowledge/audio_stream/audio_pipe_selector/通路选择.md)、`services/audio_policy/server/domain/pipe/`、`services/audio_service/` |
| 修改焦点、中断、闪避或 AudioSession | [音频会话.md](docs/knowledge/audio_policy/audio_session/音频会话.md)、[音频焦点管理.md](docs/knowledge/audio_policy/audio_interrupt/音频焦点管理.md)、`services/audio_policy/server/domain/interrupt/`、`services/audio_policy/server/domain/session/` |
| 修改音量、静音、安全音量或铃声模式 | [音频音量.md](docs/knowledge/audio_policy/audio_volume/音频音量.md)、[音频路由和发声规则.md](docs/knowledge/audio_policy/audio_routing_rules/音频路由和发声规则.md)、`services/audio_policy/server/domain/volume/` |
| 修改设备发现、蓝牙、USB、有线、分布式或热插拔 | [音频设备管理.md](docs/knowledge/audio_policy/audio_device_manager/音频设备管理.md)、[安全与隐私.md](docs/knowledge/common/security_privacy/安全与隐私.md)、`services/audio_policy/server/domain/device/` |
| 修改设备选择、场景路由或播放/录制规则 | [音频路由和发声规则.md](docs/knowledge/audio_policy/audio_routing_rules/音频路由和发声规则.md)、`services/audio_policy/server/domain/router/`、`router_context/` |
| 修改处理图、引擎插件或在线音效 | [音频引擎.md](docs/knowledge/audio_engine/音频引擎.md)、`services/audio_engine/`、`frameworks/native/audioeffect/` |
| 修改音频编创能力 | [音频编创.md](docs/knowledge/audio_suite/音频编创.md)、`services/audio_suite/`、`frameworks/native/ohaudiosuite/` |
| 修改多音区或跨区会话 | [音频多音区.md](docs/knowledge/audio_policy/audio_multi_zone/音频多音区.md)、`services/audio_policy/server/domain/zone/` |
| 修改 DDL、线程调度、ROM/RAM 或 reclaim | [音频性能和内存.md](docs/knowledge/common/performance_memory/音频性能和内存.md)、`frameworks/native/audioschedule/`、`services/audio_engine/` |
| 修改 IPC、系统能力、IDL、代理或 Stub | [架构与构建说明.md](docs/knowledge/架构与构建说明.md)、[安全与隐私.md](docs/knowledge/common/security_privacy/安全与隐私.md)、对应 `services/*/idl/`、`client/`、`server/` |
| 修改构建、功能开关、配置或系统能力注册 | [架构与构建说明.md](docs/knowledge/架构与构建说明.md)、`bundle.json`、`config.gni`、`sa_profile/`、相关 `BUILD.gn` |
| 修改测试或验证策略 | 本指南的“验证要求”、相关主题文档、`test/`、对应模块 `test/` |
| 修改 DFX 公共机制、打点、日志或转储 | [DFX.md](docs/knowledge/common/dfx/DFX.md)、`hisysevent.yaml`、各模块 DFX 入口 |

当任务涉及播放、录制、pipe、并发、会话、设备、路由、音量、音频引擎、音频编创、多音区、DFX、性能或内存时，先阅读对应主题文档，再定位实现。

编辑前必须先说明：任务类别、已阅读的文档、涉及的公共契约或源码路径、发现的状态/权限/兼容性约束，以及计划执行的验证项。没有完成这一步，不应直接修改文件。

## 修改要求

- 修改实现前，应先阅读相关 `BUILD.gn`、公共头文件、IPC IDL、服务接口和测试目标。
- 公共 API 变更必须同步检查接口、原生客户端、多语言绑定、IPC 定义、服务端实现和测试。
- 必须保持 Renderer 和 Capturer 的状态机约束。每个边界都要校验状态、所有权、缓冲区长度、会话标识和服务可用性。
- 音频策略属于全局协调层。路由、焦点、音量、设备状态或隐私状态的变化可能影响不同应用的音频流。
- 音频数据具有敏感性。禁止新增包含 PCM 内容、麦克风数据、原始令牌、设备地址或未脱敏应用数据的日志。
- `config.gni` 及相关 `.gni` 文件是可选功能的配置来源。
- IPC 修改必须同步代理端和 Stub 端，保持序列化顺序和接口码兼容，并补充失败路径测试。
- 优先复用仓库已有的校验宏、错误码、日志风格、生命周期和线程模型。

## 高风险边界

- 公共 API 的签名、重载、错误码、版本标记和既有行为不得无理由改变；新增接口或字段必须评估绑定层和兼容性。
- IDL、代理、Stub、序列化顺序和接口码属于协议边界，不能只修改单端，也不能复用不兼容的接口码。
- `bundle.json`、`config.gni`、策略配置、系统能力配置和生成文件必须以仓库中的源文件为准，不直接修改生成产物替代源配置。
- 权限、UID、PID、令牌 ID、账户、远端设备授权和隐私状态属于信任边界；无法确认调用方身份或用途时必须失败关闭并保留原状态。
- 不得执行未经确认的破坏性命令、设备刷写、系统分区修改或大范围删除；涉及真实设备时先说明影响范围并等待确认。
- 修改第三方依赖、许可证、公共符号、持久化配置或跨版本协议前，必须先说明影响并请求确认。

## 验证要求

针对局部修改，优先执行对应的 GN 目标和单元测试。跨越策略、服务、引擎或 IPC 边界的修改，还应检查相关模糊测试和服务级测试。构建环境不可用时，必须说明未执行的命令、原因、替代检查和残余风险。

最小验证矩阵：

| 变更类型 | 至少验证 |
| --- | --- |
| 仅文档或注释 | Markdown 链接、路径和格式检查；`git diff --check` |
| 原生 Renderer/Capturer/音效 | 对应 `frameworks/native/*/test/` 或服务测试目标，覆盖正常、非法状态、重复调用和服务不可用 |
| 策略、焦点、音量、路由、配置 | 对应 `services/audio_policy/test/`、领域测试和设备连接/断开/恢复场景 |
| IPC、IDL、回调或序列化 | 代理/Stub/服务端联调、非法输入和服务死亡测试，以及对应 `test/fuzztest/` |
| 公共绑定或 API | N-API、C API、CJ、Taihe 或 OpenSL ES 对应测试，并检查接口行为和错误码 |
| 功能开关或构建配置 | 开启和关闭路径、依赖缺失降级路径，以及对应 GN 目标 |

重点验证：

- Renderer 和 Capturer 生命周期；
- 音频策略中的设备、路由、音量、焦点和中断；
- 音频流管理中的流、端点、缓冲区和服务恢复；
- 公共绑定变化对应的 N-API、C API、CJ 或 Taihe 测试；
- IPC、配置解析、回调和服务入口的模糊测试；
- OpenHarmony 构建流程。

## 构建与静态检查

- 构建命令必须从完整 OpenHarmony 源码根目录执行，不能在本仓库子目录执行。
- 整体构建命令：

  ```sh
  ./build.sh --product-name rk3568 --ccache --build-target make_all
  ```

- 仅构建本组件的命令：

  ```sh
  ./build.sh --product-name rk3568 --ccache --build-target audio_framework
  ```

- 若产品名或构建参数因环境不同而变化，必须以用户指定值或当前源码根目录的构建配置为准。
- 静态检查至少执行：

  ```sh
  git diff --check
  ```

- 涉及 C/C++ 时，还必须执行仓库规定的格式检查和静态检查。

## 文档要求

- 文档以设计规格和实现边界为主，避免堆积过细的代码描述。
- 优先说明不变量、状态、所有权、安全、隐私和失败行为，再列源码入口。
- 源码路径使用仓库相对路径。
- 依赖功能开关的行为必须明确写出开关名称。
- 新增知识文档后，必须更新对应的主题索引。
- 推断出的行为不能直接描述为稳定的公共 API 契约。

## 常见误区

- 不能把 `services/audio_policy` 的策略决策与 `services/audio_service` 的流执行混为同一层；修改一侧时要检查另一侧的调用契约。
- 不能只修改某一个语言绑定或某一个 IPC 端点；公共接口、代理、Stub、服务实现和测试必须一起核对。
- 不能因为功能开关默认关闭就跳过关闭路径、依赖缺失和服务恢复验证。
- 不能把主题文档中的推断、示例链路或当前实现细节直接当作公共 API 保证。
- 涉及有明确上下层关系的模块时，除回调外应避免反向调用；确需反向调用或回调时必须评估死锁风险，尤其是流执行层回调策略层时。

## 完成定义

任务只有同时满足以下条件才算完成：修改范围与需求一致；公共 API、IPC、权限、配置和依赖影响已检查；对应最小验证矩阵已执行；新增知识文档已更新索引；工作区无意外改动。最终回复必须列出修改文件、验证命令及结果、未执行项及原因、剩余风险和是否需要后续人工确认。
