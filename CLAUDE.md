# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This project uses **GN (Generate Ninja)** - Google's build system for OpenHarmony.

### Basic Build Commands

```bash
# Generate build files for RISC-V target
gn gen out/riscv64 --args='target_os="ohos"'

# Build all components
ninja -C out/riscv64
```

### Test Build Commands

```bash
# Unit tests
ninja -C out/riscv64 //foundation/multimedia/audio_framework/test:audio_unit_test

# Module/integration tests
ninja -C out/riscv64 //foundation/multimedia/audio_framework/test:audio_module_test

# Fuzz tests
ninja -C out/riscv64 //foundation/multimedia/audio_framework/test:audio_fuzz_test

# Benchmark tests
ninja -C out/riscv64 //foundation/multimedia/audio_framework/test:audio_benchmark_test
```

## Feature Flags

The `config.gni` file controls which features are enabled. Key flags include:

- `audio_framework_feature_wired_audio` - Wired headset support (default: true)
- `audio_framework_feature_usb_audio` - USB audio device support (default: false)
- `audio_framework_feature_opensl_es` - OpenSL ES API support (default: true)
- `audio_framework_feature_distributed_audio` - Distributed audio support (default: true)
- `audio_framework_feature_low_latency` - Low-latency mode support (default: true)
- `audio_framework_feature_hitrace_enable` - HiTrace integration (default: true)
- `sonic_enable` - Sonic audio processing library (default: true)
- `speex_enable` - Speex codec support (default: false)

## Architecture Overview

The audio framework follows a **layered service-oriented architecture**:

```
Application Layer
├── JavaScript APIs (NAPI) - frameworks/js/napi
├── C++ APIs (Inner API) - interfaces/inner_api
└── C APIs (FFI) - frameworks/cj

Framework Layer
├── OHAudio - High-level C++ API (frameworks/native/ohaudio)
├── OpenSL ES - Standard audio API (frameworks/native/opensles)
├── PulseAudio - Linux backend (frameworks/native/pulseaudio)
└── AudioPolicy - Policy management (frameworks/native/audiopolicy)

Service Layer
├── AudioEngine - Audio processing pipeline (services/audio_engine)
├── AudioService - Core audio services (services/audio_service)
├── AudioPolicy - Policy enforcement (services/audio_policy)
└── AudioSuite - Extended functionality (services/audio_suite)

Hardware Layer
├── HDI Adapter - Hardware Driver Interface
└── Audio HAL - Hardware Abstraction Layer
```

## Core Components

### AudioRenderer (Audio Playback)
- **Location**: `frameworks/native/audiorenderer/`
- **Header**: `interfaces/inner_api/native/audiorenderer/include/audio_renderer.h`
- **Usage**: Create with `AudioRenderer::Create(streamType)`, set parameters, start, write data, stop, release

### AudioCapturer (Audio Recording)
- **Location**: `frameworks/native/audiocapturer/`
- **Header**: `interfaces/inner_api/native/audiocapturer/include/audio_capturer.h`
- **Usage**: Create with `AudioCapturer::Create(streamType)`, set parameters, start, read data, stop, release

### AudioSystemManager (System Control)
- **Location**: `services/audio_service/client/`
- **Header**: `interfaces/inner_api/native/audiomanager/include/audio_system_manager.h`
- **Usage**: GetInstance() singleton for volume, device, ringer mode control

### AudioStreamManager (Stream Management)
- **Location**: `services/audio_service/client/`
- **Header**: `interfaces/inner_api/native/audiomanager/include/audio_stream_manager.h`
- **Usage**: Register listeners for renderer/capturer state changes, query active streams

## Audio Stream Types

Defined in `interfaces/inner_api/native/audiocommon/include/audio_info.h`:

- `STREAM_MUSIC` - Music playback
- `STREAM_RING` - Ringtones
- `STREAM_VOICE_CALL` - Voice calls
- `STREAM_VOICE_ASSISTANT` - Voice assistant
- `STREAM_ALARM` - Alarms
- `STREAM_NOTIFICATION` - Notifications
- And more...

## Device Types

Supported devices (see `audio_info.h`):
- `DEVICE_TYPE_SPEAKER` - Built-in speaker
- `DEVICE_TYPE_MIC` - Built-in microphone
- `DEVICE_TYPE_WIRED_HEADSET` - 3.5mm or USB-C analog headset
- `DEVICE_TYPE_USB_HEADSET` - USB digital headset with DAC
- `DEVICE_TYPE_BLUETOOTH_A2DP` - Bluetooth A2DP headset
- `DEVICE_TYPE_BLUETOOTH_SCO` - Bluetooth SCO (calls)

## Code Patterns

### Error Handling
- Return `int32_t` error codes (0 = success, negative = error)
- Use defined error constants from `audio_errors.h`
- Always check return values

### Resource Management
- Use RAII patterns (smart pointers)
- Call `Release()` on AudioRenderer/AudioCapturer when done
- Follow proper lifecycle: Create → SetParams → Start → Use → Stop → Release

### Callbacks
- Implement observer interfaces (e.g., `AudioRendererStateChangeCallback`)
- Register with `RegisterAudioRendererEventListener()` / `RegisterAudioCapturerEventListener()`
- Override `OnRendererStateChange()` / `OnCapturerStateChange()`

### Threading
- Audio operations are thread-safe
- Use mutexes for shared state access
- Support both blocking and non-blocking I/O operations

## Test Organization

- **Unit tests**: `test/unittest/` - 200+ test targets
- **Fuzz tests**: `test/fuzztest/` - 100+ fuzzing targets
- **Module tests**: `test/moduletest/` - Integration tests
- **Benchmark tests**: Various `benchmark/` subdirectories

## Dependencies

Key dependencies (from bundle.json):
- `hdf_core` - Hardware Driver Foundation
- `pulseaudio` - Linux audio backend
- `opensles` - OpenSL ES implementation
- `hicollie`, `hisysevent`, `hilog` - System services
- `safwk`, `samgr` - Service framework and system ability manager
- `napi` - Node-API for JavaScript bindings

## File Structure Conventions

- **Interfaces**: `interfaces/inner_api/native/` - Public headers
- **Implementation**: `frameworks/native/` - Native code
- **Services**: `services/` - Background services
- **JS Bindings**: `frameworks/js/napi/` - JavaScript API
- **Tests**: `test/` - All test code
- **Service profiles**: `sa_profile/` - Service configuration

## Audio Processing Pipeline

The audio engine (`services/audio_engine/`) provides:
- Sample rate conversion (using Sonic or custom resampler)
- Audio effect chains
- Volume control
- Channel mapping
- Format conversion

## Distributed Audio

When `audio_framework_feature_distributed_audio = true`:
- Supports routing audio to networked devices
- Uses HDI distributed audio interface
- Device discovery via DeviceManager
