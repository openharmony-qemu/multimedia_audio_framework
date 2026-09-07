# ohos-audioManager

## 概述

ohos-audioManager 是一个用于管理音频设备和音量的命令行工具。它提供了获取和设置音量、查询静音状态、获取设备列表以及切换输入输出设备等功能。

## 功能列表

- 获取指定音量类型的当前音量
- 设置指定音量类型的音量
- 获取指定音量类型的最大音量
- 查询指定音量类型的静音状态
- 设置指定音量类型的静音状态
- 获取音频设备列表
- 切换音频输出设备
- 切换音频输入设备

## 依赖

- 系统能力：AudioSystemManager (audio_client)
- 权限：
  - ACCESS_NOTIFICATION_POLICY_PERMISSION（设置 STREAM_RING 或 STREAM_VOICE_RING 类型的静音状态需要此权限）
  - 系统权限（GetDevices 部分设备标志和 SelectOutputDevice/SelectInputDevice 需要）

## 基本用法

```bash
ohos-audioManager <command> [options]
```

## 命令列表

| 命令 | 说明 | 参数 | 权限 | 前置依赖 |
|------|------|------|------|----------|
| get-volume | 获取音量 | --type | 无 | 无 |
| set-volume | 设置音量 | --volume, --type | 无 | 无 |
| get-max-volume | 获取最大音量 | --type | 无 | 无 |
| is-mute | 获取静音状态 | --type | 无 | 无 |
| set-mute | 设置静音 | --mute, --type | ACCESS_NOTIFICATION_POLICY_PERMISSION（STREAM_RING 或 STREAM_VOICE_RING 类型） | 无 |
| get-devices | 获取设备列表 | --flag | 系统权限（部分 flag） | 无 |
| select-output-device | 切换输出设备 | --type | 系统权限 | 无 |
| select-input-device | 切换输入设备 | --type | 系统权限 | 无 |

**前置依赖说明**：
- **无**：该命令可直接执行，无需前置条件
- **必须**：必须先执行前置命令才能成功执行
- **可选**：建议先执行前置命令以确保状态正确

## 示例

```bash
# 获取音量（无前置依赖）
ohos-audioManager get-volume
ohos-audioManager get-volume --type STREAM_RING
ohos-audioManager get-volume --type STREAM_MUSIC

# 设置音量（无前置依赖）
ohos-audioManager set-volume --volume 50
ohos-audioManager set-volume --volume 10 --type STREAM_RING
ohos-audioManager set-volume --volume 80 --type STREAM_MUSIC

# 获取最大音量（无前置依赖）
ohos-audioManager get-max-volume
ohos-audioManager get-max-volume --type STREAM_RING
ohos-audioManager get-max-volume --type STREAM_MUSIC

# 获取静音状态（无前置依赖）
ohos-audioManager is-mute
ohos-audioManager is-mute --type STREAM_RING
ohos-audioManager is-mute --type STREAM_MUSIC

# 设置静音（无前置依赖）
ohos-audioManager set-mute --mute true
ohos-audioManager set-mute --mute false --type STREAM_RING
ohos-audioManager set-mute --mute true --type STREAM_MUSIC

# 获取设备列表（无前置依赖）
ohos-audioManager get-devices
ohos-audioManager get-devices --flag OUTPUT_DEVICES_FLAG
ohos-audioManager get-devices --flag INPUT_DEVICES_FLAG

# 切换输出设备（无前置依赖）
ohos-audioManager select-output-device --type DEVICE_TYPE_SPEAKER
ohos-audioManager select-output-device --type DEVICE_TYPE_BLUETOOTH_A2DP
ohos-audioManager select-output-device --type DEFAULT

# 切换输入设备（无前置依赖）
ohos-audioManager select-input-device --type DEVICE_TYPE_MIC
ohos-audioManager select-input-device --type DEVICE_TYPE_USB_HEADSET
ohos-audioManager select-input-device --type DEVICE_TYPE_BLUETOOTH_SCO
```

## 参数说明

### 音量类型 (AudioVolumeType)

| 值 | 说明 |
|----|------|
| STREAM_MUSIC | 音乐流 |
| STREAM_RING | 铃声流 |
| STREAM_VOICE_CALL | 语音通话流 |
| STREAM_VOICE_COMMUNICATION | VoIP 通话流 |
| STREAM_VOICE_ASSISTANT | 语音助手流 |
| STREAM_ALARM | 闹钟流 |
| STREAM_SYSTEM | 系统音流 |
| STREAM_VOICE_RING | 语音铃声流 |
| STREAM_NAVIGATION | 导航流 |
| STREAM_NOTIFICATION | 通知流 |
| STREAM_ULTRASONIC | 超声波流 |
| STREAM_ANNOUNCEMENT | 广播流 |
| STREAM_EMERGENCY | 紧急流 |
| STREAM_ALL | 所有流 |

### 设备类型 (DeviceType)

| 值 | 说明 |
|----|------|
| DEVICE_TYPE_INVALID | 无效设备 |
| DEVICE_TYPE_EARPIECE | 听筒 |
| DEVICE_TYPE_SPEAKER | 扬声器 |
| DEVICE_TYPE_WIRED_HEADSET | 有线耳机（带麦） |
| DEVICE_TYPE_WIRED_HEADPHONES | 有线耳机（不带麦） |
| DEVICE_TYPE_BLUETOOTH_SCO | 蓝牙 SCO 设备 |
| DEVICE_TYPE_BLUETOOTH_A2DP | 蓝牙 A2DP 设备 |
| DEVICE_TYPE_MIC | 麦克风 |
| DEVICE_TYPE_USB_HEADSET | USB 耳机 |
| DEVICE_TYPE_DP | DP 显示设备 |
| DEVICE_TYPE_REMOTE_CAST | 远程投屏设备 |
| DEVICE_TYPE_USB_DEVICE | USB 设备 |
| DEVICE_TYPE_HDMI | HDMI 设备 |
| DEVICE_TYPE_LINE_DIGITAL | 线路数字设备 |
| DEVICE_TYPE_REMOTE_DAUDIO | 分布式音频设备 |
| DEVICE_TYPE_HEARING_AID | 助听设备 |
| DEVICE_TYPE_NEARLINK | 星闪设备 |
| DEVICE_TYPE_SYSTEM_PRIVATE | 系统私有设备 |
| DEFAULT | 默认设备 |

### 设备标志 (DeviceFlag)

| 值 | 说明 |
|----|------|
| OUTPUT_DEVICES_FLAG | 输出设备 |
| INPUT_DEVICES_FLAG | 输入设备 |
| ALL_DEVICES_FLAG | 所有设备 |