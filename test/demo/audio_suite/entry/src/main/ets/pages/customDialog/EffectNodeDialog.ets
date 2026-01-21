/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import {
  NodeType,
  AudioTrack,
  SoundFiledType,
  EnvironmentType,
  VoiceBeautifierType,
  AudioSeparationType,
  Node,
  ColorMap
} from "../../utils/InterfaceInfo";
import { Logger } from '../../utils/Logger';
import { FieldComponent } from '../common/FieldComponent';
import { EnvironmentComponent } from '../common/EnvironmentComponent'
import { NoiseReductionComponent } from '../common/NoiseReductionComponent'
import { VoiceBeautifierComponent } from '../common/VoiceBeautifierComponent'
import { AudioSeparationComponent } from '../common/AudioSeparationComponent'
import { SoundSpeedToneComponent } from '../common/SoundSpeedToneComponent'
import audioNapi from 'libentry.so';

const TAG: string = 'AudioEditTestApp_EffectDialog';

@CustomDialog
@Component
export struct EffectNodeDialog {
  @StorageLink('isCurrentMulPipeline') isCurrentMulPipeline: boolean = false;
  @StorageLink('currentPipelineId') currentPipelineId: string = '';
  @Prop selectedEffectNode: NodeType;
  @Prop selectedEffectNodeId: string;
  @Prop selectedAudioTrackId: string;
  @Prop selectedAudioAssetStartTime: number;
  @Prop audioTrackList: AudioTrack[];
  onAgree?: (node: Node) => void;
  // Sound field data, default is none.
  @Prop selectedSoundFiled: SoundFiledType;
  // Environmental data, default is none.
  @Prop selectedEnvironment: EnvironmentType;
  // Beautify data, default is normal
  @Prop selectedVoiceBeautifier: VoiceBeautifierType;
  // Noise reduction data, default is none.
  @State selectedNoiseReduction: number = -1;
  // Audio source separation, default is vocals.
  @Prop selectedAudioSeparation: AudioSeparationType;
  // speed of sound
  @Prop soundSpeed: number;
  // pitch
  @Prop soundTone: number;
  // Real-time playback (audio source separation)
  @Consume isSeparation: boolean = false;
  controllerTwo?: CustomDialogController;

  build() {
    Column() {
      if (this.selectedEffectNode === NodeType.FIELD) {
        FieldComponent({
          selectedSoundFiled: this.selectedSoundFiled,
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: (soundFieldType: SoundFiledType) => {
            // Call the NAPI interface to add the sound field node to the corresponding audioAsset.
            let result = audioNapi.resetFieldEffect(this.selectedAudioTrackId, soundFieldType,
              this.selectedEffectNodeId);
            if (result !== 0) {
              this.getUIContext().showAlertDialog({
                message: '设置声场节点失败',
                autoCancel: true
              });
              return;
            }

            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.FIELD)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.FIELD,
              isByPass: soundFieldType === SoundFiledType.SOUND_FIELD_CLOSE ? false : true,
              soundFiledType: soundFieldType
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
          }
        })
      } else if (this.selectedEffectNode === NodeType.ENV) {
        EnvironmentComponent({
          selectedEnvironment: this.selectedEnvironment,
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: (environmentType: EnvironmentType) => {
            // Call the NAPI interface to add the environment node to the corresponding audioAsset.
            let result = audioNapi.resetEnvEffect(this.selectedAudioTrackId, this.selectedEffectNodeId,
              environmentType);
            if (result !== 0) {
              this.getUIContext().showAlertDialog({
                message: '设置环境节点失败',
                autoCancel: true
              });
              return;
            }

            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.ENV)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.ENV,
              isByPass: environmentType === EnvironmentType.ENVIRONMENT_TYPE_CLOSE ? false : true,
              environmentType: environmentType
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
          }
        })
      } else if (this.selectedEffectNode === NodeType.NR) {
        NoiseReductionComponent({
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: () => {
            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.ENV)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.NR,
              isByPass: true
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
          }
        })
      } else if (this.selectedEffectNode === NodeType.VB) {
        VoiceBeautifierComponent({
          selectedVoiceBeautifier: this.selectedVoiceBeautifier,
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: (voiceBeautifierType: VoiceBeautifierType) => {
            // Call the NAPI interface to add the beautified node to the corresponding audioAsset.
            let result =
              audioNapi.resetVBEffect(this.selectedAudioTrackId, voiceBeautifierType,
                this.selectedEffectNodeId);
            if (result !== 0) {
              this.getUIContext().showAlertDialog({
                message: '添加美化节点失败',
                autoCancel: true
              });
              return;
            }

            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.VB)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.VB,
              isByPass: voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL ? false : true,
              voiceBeautifierType: voiceBeautifierType
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
          }
        })
      } else if (this.selectedEffectNode === NodeType.SEPARATION) {
        AudioSeparationComponent({
          selectedAudioSeparation: this.selectedAudioSeparation,
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: (audioSeparationType: AudioSeparationType) => {
            // Call the NAPI interface to add the audio source separation node to the corresponding audioAsset.
            let result = audioSeparationType === AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE ?
              audioNapi.setSeparationMode(0) : audioNapi.setSeparationMode(1);
            if (result !== 0) {
              this.getUIContext().showAlertDialog({
                message: '设置音源分离模式失败',
                autoCancel: true
              });
              return;
            }

            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.SEPARATION)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.SEPARATION,
              isByPass: true,
              audioSeparationType: audioSeparationType
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
            this.isSeparation = true;
          }
        })
      } else if (this.selectedEffectNode === NodeType.SOUND_SPEED_TONE) {
        SoundSpeedToneComponent({
          soundSpeed: this.soundSpeed,
          soundTone: this.soundTone,
          onCancel: () => {
            this.controllerTwo?.close();
          },
          onAgree: (soundSpeed: number, soundTone: number) => {
            // Call the NAPI interface to add the audio speed and volume adjustment points to the corresponding audioAsset.
            let result =
              audioNapi.setSoundSpeedTone(this.selectedAudioTrackId, this.selectedEffectNodeId, soundSpeed, soundTone);
            if (result !== 0) {
              // 音速音调效果节点设置失败
              this.getUIContext().showAlertDialog({
                message: '修改音速音调失败',
                autoCancel: true
              });
              return;
            }
            // Bypass control node via the addAudioAssetEffectNode interface
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.SOUND_SPEED_TONE)) {
              return;
            }

            let node: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.SOUND_SPEED_TONE,
              isByPass: true,
              soundSpeed: soundSpeed,
              soundTone: soundTone
            }
            if (this.onAgree) {
              this.onAgree(node);
            }
            this.controllerTwo?.close();
          }
        })
      }
    }
  }

  isSuccessAddEffectToAudioAssetNapi(type: NodeType) {
    Logger.info(TAG, `enter isSuccessAddEffectToAudioAssetNapi, nodeType:${type}`);
    if (this.isNodeTypeInAudioAsset(type)) {
      let addAssetResult =
        audioNapi.addAudioAssetEffectNode(this.selectedAudioTrackId, this.selectedAudioAssetStartTime,
          this.selectedEffectNodeId);
      if (!addAssetResult) {
        this.getUIContext().showAlertDialog({
          message: `设置空间渲染节点失败`,
          autoCancel: true
        });
        return false;
      }
      Logger.info(TAG, `enter bypass set`);
      let ret = audioNapi.setEffectNodeBypass(this.selectedAudioTrackId, this.selectedEffectNodeId, false);
      if (!ret) {
        this.getUIContext().showAlertDialog({
          message: `设置空间渲染节点Bypass失败`,
          autoCancel: true
        });
        return false;
      }
    }
    return true;
  }

  isNodeTypeInAudioAsset(type: NodeType) {
    let nodes = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId ===
    this.selectedAudioTrackId)?.audioAssetArray?.find(audioAsset => audioAsset.startTime ===
    this.selectedAudioAssetStartTime)?.nodes;
    return nodes?.find(node => node.type === type) ? true : false;
  }
}