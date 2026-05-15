# Design Proposal: Non-Standard Sample Rate forceData Logic in HpaeSinkInputNode

## 1. Overview

### Problem Statement
In `HpaeSinkInputNode::OnStreamInfoChange()`, the `forceData` flag currently only considers `offloadEnable_` and `isDirect_`:

```cpp
bool forceData = (offloadEnable_ | isDirect_) ? false : true;
```

When `forceData` is `true` and insufficient data is available, the system pads with silence and still writes data downstream. For non-standard sample rates (rates not in the `AudioSamplingRate` enum, or rates like 11025/22050 that the audio engine cannot resample), the audio engine needs resampling but the resampler may not handle these rates correctly, producing artifacts or silence. The `forceData` flag should also consider whether the input sample rate is non-standard, so that non-standard rate sessions do not force data writes when insufficient data is available -- preventing silence padding artifacts in the resampled output.

### Scope and Boundaries
- **In scope**: Add `isNonStandardSampleRate_` member to `HpaeSinkInputNode`; evaluate it in constructor and `SetNodeInfo`; modify `forceData` logic.
- **Out of scope**: Changes to the resampler, converter node, or downstream renderer stream implementation.

## 2. Architecture

### Current Data Flow
1. `HpaeSinkInputNode` is created by renderer managers (normal/fast/direct/offload) with an `HpaeNodeInfo` containing `samplingRate` (enum) and `customSampleRate` (uint32_t).
2. The effective sample rate is: `customSampleRate != 0 ? customSampleRate : samplingRate`.
3. `OnStreamInfoChange()` sets `forceData = true` unless `offloadEnable_` or `isDirect_` is true.
4. `forceData` is passed downstream via `streamInfo_.forceData` to `HpaeRendererStreamImpl::WriteDataFromRingBuffer()`, where it determines whether to pad silence when insufficient data is available.

### Proposed Change
Add a boolean member `isNonStandardSampleRate_` that is evaluated once at construction and updated whenever `SetNodeInfo()` is called. The `forceData` logic is extended to also check this flag.

### Definition of "Non-Standard Sample Rate" for forceData Purposes

A sample rate is considered **non-standard** if its effective rate (from `GetEffectiveSampleRate()`) is NOT in the following set of rates that the audio engine can handle without resampling issues:

**Rates NOT treated as non-standard (engine-supported):**
| Rate    | AudioSamplingRate Enum           |
|---------|----------------------------------|
| 8000    | SAMPLE_RATE_8000                 |
| 12000   | SAMPLE_RATE_12000                |
| 16000   | SAMPLE_RATE_16000                |
| 24000   | SAMPLE_RATE_24000                |
| 32000   | SAMPLE_RATE_32000                |
| 44100   | SAMPLE_RATE_44100                |
| 48000   | SAMPLE_RATE_48000                |
| 64000   | SAMPLE_RATE_64000                |
| 88200   | SAMPLE_RATE_88200                |
| 96000   | SAMPLE_RATE_96000                |
| 176400  | SAMPLE_RATE_176400               |
| 192000  | SAMPLE_RATE_192000               |
| 384000  | SAMPLE_RATE_384000               |

**Rates treated as non-standard (forceData = false):**
| Rate    | Reason                                            |
|---------|---------------------------------------------------|
| 11025   | Audio engine does not support resampling from 11.025kHz; uses 100ms threshold causing high latency |
| 22050   | Audio engine does not support resampling from 22.05kHz; same threshold issue as 11025 |
| Any other value not in the enum | E.g., 8010, 88310, 48010 -- require resampling with large thresholds and may cause artifacts |

### Key Design Decisions

1. **Why a member variable instead of inline check?** The effective sample rate is derived from `nodeInfo.customSampleRate` and `nodeInfo.samplingRate`. Computing this repeatedly in `OnStreamInfoChange()` (called every processing cycle) is wasteful. A cached member variable is cleaner and more efficient.

2. **Why check in both constructor AND `SetNodeInfo`?** The sample rate can change when `SetNodeInfo()` is called (e.g., during device switch, session move). The constructor sets the initial value; `SetNodeInfo()` updates it.

3. **Why use `GetEffectiveSampleRate()`?** This existing method on `HpaeDfxNodeInfo` already encapsulates the `customSampleRate != 0 ? customSampleRate : samplingRate` logic used throughout the codebase (see `hpae_node_common.cpp` lines 279, 288, 308, 314).

## 3. Files to Create/Modify

### [MODIFY] `services/audio_engine/manager/include/hpae_define.h`
- Add `STANDARD_SAMPLE_RATES_FOR_FORCE_DATA` shared constant set
- Add `IsNonStandardSampleRate()` inline helper function

### [MODIFY] `services/audio_engine/node/include/hpae_sink_input_node.h`
- Add `bool isNonStandardSampleRate_ = false;` private member variable
- Add `bool IsNonStandardSampleRate() const` public getter method

### [MODIFY] `services/audio_engine/node/src/hpae_sink_input_node.cpp`
- Add a helper function or inline set to determine non-standard sample rate
- Update constructor to initialize `isNonStandardSampleRate_`
- Update `SetNodeInfo()` to re-evaluate `isNonStandardSampleRate_`
- Modify `forceData` logic in `OnStreamInfoChange()`

### [MODIFY] `services/audio_engine/test/unittest/node/hpae_sink_input_node_test.cpp`
- Add unit tests for the new `isNonStandardSampleRate_` behavior

## 4. Interface Definitions

### New Member Variable

```cpp
// In HpaeSinkInputNode private section (hpae_sink_input_node.h)
bool isNonStandardSampleRate_ = false;
```

### New Shared Constant (in hpae_define.h)

```cpp
// Standard sample rates supported by the audio engine for forceData purposes.
// 11025 and 22050 are excluded because the audio engine cannot resample from them.
const std::set<uint32_t> STANDARD_SAMPLE_RATES_FOR_FORCE_DATA = {
    8000, 12000, 16000, 24000, 32000, 44100, 48000,
    64000, 88200, 96000, 176400, 192000, 384000
};

inline bool IsNonStandardSampleRate(uint32_t effectiveSampleRate)
{
    return STANDARD_SAMPLE_RATES_FOR_FORCE_DATA.find(effectiveSampleRate)
        == STANDARD_SAMPLE_RATES_FOR_FORCE_DATA.end();
}
```

### New Public Getter (in hpae_sink_input_node.h)

```cpp
// In HpaeSinkInputNode public section
bool IsNonStandardSampleRate() const { return isNonStandardSampleRate_; }
```

### Constructor Change

```cpp
HpaeSinkInputNode::HpaeSinkInputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      // ... existing initializer list ...
{
    // ... existing constructor body ...

    // Evaluate non-standard sample rate
    isNonStandardSampleRate_ = IsNonStandardSampleRate(nodeInfo.GetEffectiveSampleRate());
    if (isNonStandardSampleRate_) {
        AUDIO_INFO_LOG("Non-standard sample rate detected: %u", nodeInfo.GetEffectiveSampleRate());
    }
}
```

### SetNodeInfo Change

```cpp
void HpaeSinkInputNode::SetNodeInfo(HpaeNodeInfo& nodeInfo)
{
    // ... existing frameLen check and buffer reconfiguration ...

    HpaeNode::SetNodeInfo(nodeInfo);

    // Re-evaluate non-standard sample rate after nodeInfo update
    isNonStandardSampleRate_ = IsNonStandardSampleRate(nodeInfo.GetEffectiveSampleRate());
    if (isNonStandardSampleRate_) {
        AUDIO_INFO_LOG("Non-standard sample rate updated: %u", nodeInfo.GetEffectiveSampleRate());
    }
}
```

### Modified forceData Logic

```cpp
// In OnStreamInfoChange():
// offload enable, direct, or non-standard sample rate: never force data
bool forceData = (offloadEnable_ | isDirect_ | isNonStandardSampleRate_) ? false : true;
```

## 5. Dependencies

### Build Dependencies
- No new BUILD.gn changes required. The change only uses existing headers (`<set>`, `hpae_msg_channel.h`).

### Runtime Dependencies
- `HpaeNodeInfo::GetEffectiveSampleRate()` -- already exists in `hpae_msg_channel.h` (line 147-150).
- No new library dependencies.

## 6. Build Configuration

No build configuration changes required.

## 7. Error Handling Strategy

### Edge Cases
1. **Effective rate is 0**: `GetEffectiveSampleRate()` returns `customSampleRate != 0 ? customSampleRate : samplingRate`. If `samplingRate` defaults to `SAMPLE_RATE_8000` (value 8000), this is a standard rate. If `customSampleRate` is explicitly 0, it falls through to the enum default. No division-by-zero risk.

2. **SetNodeInfo called with rate change**: The `isNonStandardSampleRate_` is re-evaluated immediately after `HpaeNode::SetNodeInfo(nodeInfo)` updates the stored `nodeInfo_`. The next call to `OnStreamInfoChange()` will use the updated flag.

3. **Rate matches a standard enum value but is passed via customSampleRate**: E.g., `customSampleRate = 48000`. `GetEffectiveSampleRate()` returns 48000, which is in the standard set. `isNonStandardSampleRate_` will be `false`. This is correct behavior.

## 8. Test Plan

### Unit Test Scenarios

All tests in `services/audio_engine/test/unittest/node/hpae_sink_input_node_test.cpp`:

| Test Case | Input Rate | Expected `isNonStandardSampleRate_` | Expected `forceData` |
|-----------|-----------|--------------------------------------|---------------------|
| Standard rate 48000 | samplingRate=SAMPLE_RATE_48000, customSampleRate=0 | false | true (if not offload/direct) |
| Standard rate 44100 | samplingRate=SAMPLE_RATE_44100, customSampleRate=0 | false | true |
| Non-standard 11025 | samplingRate=SAMPLE_RATE_11025, customSampleRate=0 | true | false |
| Non-standard 22050 | samplingRate=SAMPLE_RATE_22050, customSampleRate=0 | true | false |
| Non-standard 8010 | samplingRate=SAMPLE_RATE_8000, customSampleRate=8010 | true | false |
| Non-standard 88310 | samplingRate=SAMPLE_RATE_96000, customSampleRate=88310 | true | false |
| Offload with non-standard rate | samplingRate=SAMPLE_RATE_11025, offloadEnable=true | true | false (offload takes precedence) |
| Direct with non-standard rate | samplingRate=SAMPLE_RATE_11025, isDirect=true | true | false (direct takes precedence) |
| SetNodeInfo rate change standard->non-standard | Initial 48000, SetNodeInfo to 11025 | changes to true | false |
| SetNodeInfo rate change non-standard->standard | Initial 11025, SetNodeInfo to 48000 | changes to false | true |

### Test Approach
- The `forceData` flag is private to `OnStreamInfoChange()`. Testing can be done via mock verification of the `streamInfo_.forceData` value passed to `writeCallback_->OnStreamData()`, or by making `isNonStandardSampleRate_` accessible for testing via a getter or friend class.
- **Recommendation**: Expose `isNonStandardSampleRate_` for testing via a public getter method: `bool IsNonStandardSampleRate() const;`

## 9. Confirmed Design Decisions

### D1: Scope of "non-standard" rates -- CONFIRMED: ALL non-enum rates are non-standard
All rates not in the `AudioSamplingRate` enum (e.g., 8010, 88310, 48010 -- passed via `customSampleRate`) are treated as non-standard, in addition to 11025 and 22050.

### D2: 384000 -- CONFIRMED: Standard
384000 remains a standard rate (it is a supported enum value).

### D3: Placement -- CONFIRMED: Shared in hpae_define.h
`STANDARD_SAMPLE_RATES_FOR_FORCE_DATA` is placed in `hpae_define.h` as a shared constant for reuse by other nodes.

### D4: Public getter -- CONFIRMED: Add getter
Add `bool IsNonStandardSampleRate() const` public method for unit testing and debugging.

### D5: Logging -- CONFIRMED: Add INFO log
Add `AUDIO_INFO_LOG` when `isNonStandardSampleRate_` is set to `true`.

### D6: isLowLatency_ -- CONFIRMED: Not included
`isLowLatency_` is NOT added to the `forceData` logic. Only `offloadEnable_`, `isDirect_`, and `isNonStandardSampleRate_` are checked.
