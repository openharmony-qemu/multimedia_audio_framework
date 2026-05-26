# Virtual Keyboard Mode 重构修改文档

## 1. 修改概述

### 修改目的
将 `isVirtualKeyboard` 从布尔类型重构为枚举类型，以支持更灵活的虚拟键盘铃声模式控制。

### 修改范围
- 新增 `VkbMode` 枚举类型
- 新增 `AUDIO_FLAG_VKB_ALL_RING` 常量
- 修改相关数据结构、序列化方法、业务逻辑和单元测试

---

## 2. 新增定义

### 2.1 枚举类型 (audio_info.h)

```cpp
enum class VkbMode {
    INVALID = 0,      // 非法值，不执行虚拟键盘特殊逻辑
    ONLY_RING = 1,    // 只在响铃模式下响铃（静音时不响）
    ALL_RING = 2      // 所有模式都响铃（包括静音模式）
};
```

### 2.2 常量定义 (audio_info.h)

```cpp
constexpr int32_t AUDIO_FLAG_VKB_ALL_RING = 1026;
```

**映射关系：**
- `AUDIO_FLAG_VKB_NORMAL (1024)` → `VkbMode::ONLY_RING`
- `AUDIO_FLAG_VKB_FAST (1025)` → `VkbMode::ONLY_RING`
- `AUDIO_FLAG_VKB_ALL_RING (1026)` → `VkbMode::ALL_RING`

---

## 3. 核心变更

### 3.1 数据结构修改

#### AudioRendererInfo (audio_info.h:735)
```cpp
// 修改前：
bool isVirtualKeyboard = false;

// 修改后：
VkbMode isVirtualKeyboard = VkbMode::INVALID;
```

#### StreamVolumeParams (audio_info.h:413)
```cpp
// 修改前：
bool isVKB;

// 修改后：
VkbMode isVKB;
```

### 3.2 StreamVolume 类修改 (audio_volume.h)

```cpp
// 构造函数参数
StreamVolume(..., VkbMode isVKB);

// 成员变量
VkbMode isVirtualKeyboard_ = VkbMode::INVALID;

// Getter方法（重命名）
VkbMode GetVirtualKeyboardMode() { return isVirtualKeyboard_; }
```

### 3.3 音量计算逻辑修改

#### audio_volume.cpp (GetVolumeValues)
```cpp
// 修改前：
if (it != streamVolume_.end() && it->second.IsVirtualKeyboard() && itSV != pipeVolume_.end()) {
    sysVolume = itSV->second.isMuted_ ? 0.0f : 1.0f;
}

// 修改后：
if (it != streamVolume_.end() && itSV != pipeVolume_.end()) {
    VkbMode mode = it->second.GetVirtualKeyboardMode();
    if (mode == VkbMode::ONLY_RING) {
        sysVolume = itSV->second.isMuted_ ? 0.0f : 1.0f;  // 静音时不响
    } else if (mode == VkbMode::ALL_RING) {
        sysVolume = 1.0f;  // 忽略静音，所有模式都响
    }
}
```

#### audio_endpoint.cpp
```cpp
// 修改前：
if (isVirtualKeyboard) {
    result.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(baseVolume);
}

// 修改后：
if (isVirtualKeyboard == VkbMode::ONLY_RING) {
    result.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(baseVolume);
} else if (isVirtualKeyboard == VkbMode::ALL_RING) {
    result.volumeStart = static_cast<int32_t>(baseVolume);  // 忽略静音
}
```

### 3.4 序列化方法修改

#### AudioRendererInfo::Marshalling (audio_info.h:777)
```cpp
// 修改前：
parcel.WriteBool(isVirtualKeyboard)

// 修改后：
parcel.WriteInt32(static_cast<int32_t>(isVirtualKeyboard))
```

#### AudioRendererInfo::UnmarshallingSelf (audio_info.h:808)
```cpp
// 修改前：
isVirtualKeyboard = parcel.ReadBool();

// 修改后：
isVirtualKeyboard = static_cast<VkbMode>(parcel.ReadInt32());
```

---

## 4. 方法重命名

| 原方法名 | 新方法名 | 文件位置 |
|---------|---------|---------|
| `bool IsVirtualKeyboard(const int32_t flags)` | `VkbMode GetVirtualKeyboardMode(const int32_t flags)` | audio_renderer_private.h:177 |
| `bool IsVirtualKeyboard()` | `VkbMode GetVirtualKeyboardMode()` | audio_volume.h:158 |

---

## 5. 修改文件清单

### 核心文件（必须修改）
1. **interfaces/inner_api/native/audiocommon/include/audio_info.h**
   - 新增枚举和常量定义
   - 修改字段类型
   - 修改序列化方法

2. **services/audio_service/common/include/audio_volume.h**
   - 修改 StreamVolume 类定义

3. **frameworks/native/audiorenderer/include/audio_renderer_private.h**
   - 修改方法声明

4. **frameworks/native/audiorenderer/src/audio_renderer.cpp**
   - 重构方法实现
   - 更新调用点

5. **services/audio_service/common/src/audio_volume.cpp**
   - 修改音量计算逻辑

6. **services/audio_service/server/src/audio_endpoint.cpp**
   - 修改音量计算逻辑

### 单元测试文件
7. **frameworks/native/audiorenderer/test/unittest/renderer_test/src/audio_renderer_unit_test.cpp**
8. **services/audio_service/test/unittest/client/fast_audio_stream_unit_test.cpp**
9. **services/audio_service/test/unittest/common/audio_volume_unit_test.cpp**
10. **services/audio_policy/test/unittest/audio_ipc_serialization_unit_test/src/audio_ipc_serialization_unit_test.cpp**

---

## 6. 兼容性与注意事项

### 6.1 IPC兼容性
⚠️ **重要变更**
- 序列化格式从 `WriteBool/ReadBool` 改为 `WriteInt32/ReadInt32`
- 可能影响跨进程通信的兼容性
- 需确保所有依赖此数据结构的模块同步更新

### 6.2 默认值变更
- 原默认值：`false`
- 新默认值：`VkbMode::INVALID`
- 所有未显式设置的实例默认为 `INVALID` 模式

### 6.3 日志格式变更
- 原日志：`isVKB: T/F` (布尔值字符串)
- 新日志：`isVKB: 0/1/2` (枚举值整数)
- 需更新日志解析工具或监控脚本

### 6.4 向后兼容性
- 旧代码中 `isVirtualKeyboard ? "T" : "F"` 逻辑已失效
- 需检查所有布尔判断逻辑，改为枚举值判断

---

## 7. 业务逻辑说明

### 三种模式的行为差异

| 模式 | 枚举值 | 静音状态下的行为 | 适用场景 |
|------|-------|----------------|---------|
| INVALID | 0 | 不走特殊逻辑，正常音量计算 | 非虚拟键盘场景 |
| ONLY_RING | 1 | 静音时不响铃 | 标准虚拟键盘 |
| ALL_RING | 2 | 忽略静音，强制响铃 | 重要提示场景 |

### 音量计算公式
```
INVALID:    volume = baseVolume * vol.volumeFloat (正常逻辑)
ONLY_RING:  volume = isMuted ? 0 : baseVolume (静音控制)
ALL_RING:   volume = baseVolume (强制响铃)
```

---

## 8. 验证建议

### 8.1 编译检查
```bash
# 编译项目，检查编译错误
./build.sh
```

### 8.2 单元测试
```bash
# 运行相关单元测试
ctest -R audio_renderer_unit_test
ctest -R audio_volume_unit_test
ctest -R audio_ipc_serialization_unit_test
```

### 8.3 功能测试建议
1. 测试 `AUDIO_FLAG_VKB_NORMAL` 创建的渲染器
2. 测试 `AUDIO_FLAG_VKB_FAST` 创建的渲染器  
3. 测试 `AUDIO_FLAG_VKB_ALL_RING` 创建的渲染器
4. 测试三种模式在静音状态下的音量表现
5. 测试 IPC 序列化/反序列化正确性

---

## 9. 修改统计

| 修改类型 | 数量 |
|---------|------|
| 新增枚举 | 1 个 |
| 新增常量 | 1 个 |
| 修改字段类型 | 2 个 |
| 修改序列化方法 | 4 处 |
| 重命名方法 | 2 个 |
| 修改业务逻辑 | 3 处 |
| 更新日志输出 | 4 处 |
| 更新单元测试 | 30+ 处 |
| 总文件数 | 12 个 |

---

## 10. 版本信息

- **修改日期**: 2026-05-18
- **影响模块**: Audio Framework
- **风险等级**: 中等（涉及 IPC 序列化变更）

---

## 附录：代码示例

### A. 创建 ALL_RING 模式的渲染器
```cpp
AudioRendererOptions options;
options.rendererInfo.rendererFlags = AUDIO_FLAG_VKB_ALL_RING;
auto renderer = AudioRenderer::Create(options);
// 该渲染器将在所有模式下响铃，包括静音模式
```

### B. 检查虚拟键盘模式
```cpp
AudioRendererInfo info;
renderer->GetRendererInfo(info);

if (info.isVirtualKeyboard == VkbMode::ALL_RING) {
    // 强制响铃逻辑
} else if (info.isVirtualKeyboard == VkbMode::ONLY_RING) {
    // 静音控制逻辑
} else {
    // 正常音量逻辑
}
```

---

**文档结束**