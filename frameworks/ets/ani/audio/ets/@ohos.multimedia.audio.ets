/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { AsyncCallback, Callback, BusinessError } from '@ohos.base';

class AudioAniError extends BusinessError {
  constructor(code: number, message: string) {
    super();
    this.code = code;
    this.message = message;
  }
}

export namespace audio {
  loadLibrary("audio_ani");

  export native function getAudioManager(): AudioManager;

  export enum AudioState {
    STATE_INVALID = -1,
    STATE_NEW = 0,
    STATE_PREPARED = 1,
    STATE_RUNNING = 2,
    STATE_STOPPED = 3,
    STATE_RELEASED = 4,
    STATE_PAUSED = 5,
  }

  export enum AudioVolumeType {
    MEDIA = 3,
  }

  export enum DeviceFlag {
    NONE_DEVICES_FLAG = 0,
    OUTPUT_DEVICES_FLAG = 1,
    INPUT_DEVICES_FLAG = 2,
    ALL_DEVICES_FLAG = 3,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG = 4,
    DISTRIBUTED_INPUT_DEVICES_FLAG = 8,
    ALL_DISTRIBUTED_DEVICES_FLAG = 12,
  }

  export enum DeviceUsage {
    MEDIA_OUTPUT_DEVICES = 1,
    MEDIA_INPUT_DEVICES = 2,
    ALL_MEDIA_DEVICES = 3,
    CALL_OUTPUT_DEVICES = 4,
    CALL_INPUT_DEVICES = 8,
    ALL_CALL_DEVICES = 12,
  }

  export enum DeviceRole {
    OUTPUT_DEVICE = 2,
  }

  export enum DeviceType {
    SPEAKER = 2,
  }

  export enum ActiveDeviceType {
    SPEAKER = 2,
  }

  export enum AudioRingMode {
    RINGER_MODE_SILENT = 0,
    RINGER_MODE_VIBRATE = 1,
    RINGER_MODE_NORMAL = 2,
  }

  export enum AudioEncodingType {
    ENCODING_TYPE_INVALID = -1,
    ENCODING_TYPE_RAW = 0,
  }

  export enum ContentType {
    CONTENT_TYPE_UNKNOWN = 0,
    CONTENT_TYPE_SPEECH = 1,
    CONTENT_TYPE_MUSIC = 2,
    CONTENT_TYPE_MOVIE = 3,
    CONTENT_TYPE_SONIFICATION = 4,
    CONTENT_TYPE_RINGTONE = 5,
  }

  export enum StreamUsage {
    STREAM_USAGE_MUSIC = 1,
    STREAM_USAGE_MOVIE = 10,
  }

  export interface AudioRendererInfo {
    content?: ContentType;
    usage: StreamUsage;
    rendererFlags: number;
  }

  class AudioRendererInfoHandle implements AudioRendererInfo {
    constructor() {}
    content?: ContentType;
    usage: StreamUsage;
    rendererFlags: number = 0;
  }

  export enum InterruptMode {
    INDEPENDENT_MODE = 1,
  }

  export enum InterruptType {
    INTERRUPT_TYPE_BEGIN = 1,
    INTERRUPT_TYPE_END = 2,
  }

  export enum InterruptHint {
    INTERRUPT_HINT_RESUME = 1,
    INTERRUPT_HINT_PAUSE = 2,
    INTERRUPT_HINT_STOP = 3,
  }

  export interface InterruptEvent {
    eventType: InterruptType;
    hintType: InterruptHint;
  }

  class InterruptEventHandle implements InterruptEvent {
    eventType: InterruptType;
    hintType: InterruptHint;
  }

  export enum InterruptActionType {
    TYPE_ACTIVATED = 0,
    TYPE_INTERRUPT = 1,
  }

  export enum DeviceChangeType {
    CONNECT = 0,
    DISCONNECT = 1,
  }

  export enum AudioScene {
    AUDIO_SCENE_DEFAULT = 0,
    AUDIO_SCENE_RINGING = 1,
    AUDIO_SCENE_PHONE_CALL = 2,
    AUDIO_SCENE_VOICE_CHAT = 3
  }

  export enum VolumeAdjustType {
    VOLUME_UP = 0,
    VOLUME_DOWN = 1,
  }

  export interface AudioManager {
    getVolumeManager(): AudioVolumeManager;
  }

  class AudioManagerHandle implements AudioManager {
    private nativeAudioManagerContext: long = 0;

    constructor(nativePtr: long) {
      if (this.nativeAudioManagerContext == 0) {
        this.nativeAudioManagerContext = nativePtr;
      }
    }

    private native onInner<T, O>(type: string, callback: Callback<T>, obj?: O): void;
    native getVolumeManager(): AudioVolumeManager;
  }

  export enum DeviceBlockStatus {
    UNBLOCKED = 0,
    BLOCKED = 1,
  }

  export enum AudioConcurrencyMode {
    CONCURRENCY_DEFAULT = 0,
    CONCURRENCY_MIX_WITH_OTHERS = 1,
    CONCURRENCY_DUCK_OTHERS = 2,
    CONCURRENCY_PAUSE_OTHERS = 3
  }

  export enum AudioSessionDeactivatedReason {
    DEACTIVATED_LOWER_PRIORITY = 0
  }

  export interface AudioVolumeManager {
  }

  class AudioVolumeManagerHandle implements AudioVolumeManager {
    private nativeAudioVolumeManagerContext: long = 0;

    constructor(nativePtr: long) {
      if (this.nativeAudioVolumeManagerContext == 0) {
        this.nativeAudioVolumeManagerContext = nativePtr;
      }
    }

    private native onInner<T, O>(type: string, callback: Callback<T>, obj?: O): void;
  }

  export enum ConnectType {
    CONNECT_TYPE_LOCAL = 1,
    CONNECT_TYPE_DISTRIBUTED = 2
  }

  export type AudioRendererChangeInfoArray = Array<Readonly<AudioRendererChangeInfo>>;

  export interface AudioRendererChangeInfo {}

  export interface VolumeGroupInfo {}

  export type VolumeGroupInfos = Array<Readonly<VolumeGroupInfo>>;
  export type AudioCapturerChangeInfoArray = Array<Readonly<AudioCapturerChangeInfo>>;

  export interface AudioCapturerChangeInfo {}

  export interface AudioDeviceDescriptor {
    readonly deviceRole: DeviceRole;
    readonly deviceType: DeviceType;
  }

  class AudioDeviceDescriptorHandle implements AudioDeviceDescriptor {
    constructor() {}
    readonly deviceRole: DeviceRole;
    readonly deviceType: DeviceType;
  }

  export type AudioDeviceDescriptors = Array<Readonly<AudioDeviceDescriptor>>;

  export interface VolumeEvent {
    volumeType: AudioVolumeType;
    volume: number;
    updateUi: boolean;
  }

  class VolumeEventHandle implements VolumeEvent {
    constructor() {}
    volumeType: AudioVolumeType;
    volume: number = 0;
    updateUi: boolean = false;
  }

  export enum AudioStreamDeviceChangeReason {
    REASON_OLD_DEVICE_UNAVAILABLE = 2,
  }

  export interface AudioStreamDeviceChangeInfo {
    devices: AudioDeviceDescriptors;
    changeReason: AudioStreamDeviceChangeReason;
  }

  class AudioStreamDeviceChangeHandle implements AudioStreamDeviceChangeInfo {
    constructor() {}
    devices: AudioDeviceDescriptors = new Array<AudioDeviceDescriptor>();
    changeReason: AudioStreamDeviceChangeReason;
  }

  export enum SourceType {
    SOURCE_TYPE_INVALID = -1,
    SOURCE_TYPE_MIC = 0,
    SOURCE_TYPE_VOICE_RECOGNITION = 1,
    SOURCE_TYPE_PLAYBACK_CAPTURE = 2,
    SOURCE_TYPE_WAKEUP = 3,
    SOURCE_TYPE_VOICE_CALL = 4,
    SOURCE_TYPE_VOICE_COMMUNICATION = 7,
    SOURCE_TYPE_VOICE_MESSAGE = 10,
    SOURCE_TYPE_CAMCORDER = 13,
  }

  export type AudioEffectInfoArray = Array<AudioEffectMode>;

  export enum AudioEffectMode {
    EFFECT_NONE = 0,
    EFFECT_DEFAULT = 1,
  }

  export enum AudioSpatialDeviceType {
    SPATIAL_DEVICE_TYPE_NONE = 0,
    SPATIAL_DEVICE_TYPE_IN_EAR_HEADPHONE = 1,
    SPATIAL_DEVICE_TYPE_HALF_IN_EAR_HEADPHONE = 2,
    SPATIAL_DEVICE_TYPE_OVER_EAR_HEADPHONE = 3,
    SPATIAL_DEVICE_TYPE_GLASSES = 4,
    SPATIAL_DEVICE_TYPE_OTHERS = 5,
  }

  export enum AudioSpatializationSceneType {
    DEFAULT = 0,
    MUSIC = 1,
    MOVIE = 2,
    AUDIOBOOK = 3,
  }
}
