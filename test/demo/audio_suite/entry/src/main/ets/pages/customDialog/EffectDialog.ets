/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import {
  NodeType,
  Node,
  ColorMap,
  SoundFiledType,
  EnvironmentType,
  VoiceBeautifierType,
  AudioSeparationType,
  AudioTrack,
} from "../../utils/InterfaceInfo";
import { EffectNodeDialog } from './EffectNodeDialog';

const TAG: string = 'AudioEditTestApp_EffectDialog';

@CustomDialog
@Component
export struct EffectDialog {
  @Prop nodeTypes: NodeType[];
  @Prop hasEffectNodes: NodeType[];
  @Prop selectedSoundFiled: SoundFiledType;
  @Prop selectedEnvironment: EnvironmentType;
  @Prop selectedVoiceBeautifier: VoiceBeautifierType;
  @Prop selectedAudioSeparation: AudioSeparationType;
  @Prop selectedSoundSpeed: number;
  @Prop selectedSoundTone: number;
  @Prop audioTrackList: AudioTrack[];
  @Prop selectedAudioTrackId: string;
  @Prop selectedAudioAssetStartTime: number;
  @Link selectedEffectNodeId: string;
  @State selectedEffectNode: NodeType = NodeType.FIELD;
  onCancel?: () => void;
  onAgree?: (node: Node) => void;
  controller?: CustomDialogController;
  effectNodeDialog: CustomDialogController | null = new CustomDialogController({
    builder: EffectNodeDialog({
      selectedEffectNode: this.selectedEffectNode,
      selectedSoundFiled: this.selectedSoundFiled,
      selectedEnvironment: this.selectedEnvironment,
      selectedVoiceBeautifier: this.selectedVoiceBeautifier,
      selectedAudioSeparation: this.selectedAudioSeparation,
      soundSpeed: this.selectedSoundSpeed,
      soundTone: this.selectedSoundTone,
      selectedEffectNodeId: this.selectedEffectNodeId,
      selectedAudioTrackId: this.selectedAudioTrackId,
      audioTrackList: this.audioTrackList,
      selectedAudioAssetStartTime: this.selectedAudioAssetStartTime,
      onAgree: (node: Node) => {
        if (this.onAgree) {
          this.effectNodeDialog?.close();
          this.onAgree(node)
        }
      }
    }),
  })

  build() {
    Column() {
      Text('')
        .fontSize($r('app.float.font_size_20'))
        .margin({ top: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })
      Flex({ justifyContent: FlexAlign.SpaceAround }) {
        ForEach(this.nodeTypes, ((type: NodeType) => {
          this.EffectNode(type, this.hasEffectNodes.includes(type))
        }))
      }.margin({ bottom: $r('app.float.margin_10') })
    }.borderRadius($r('app.float.borderRadius_10'))
  }

  @Builder
  EffectNode(type: NodeType, isShow?: boolean) {
    Row() {
      Column() {
        this.Node(type, isShow)
      }
      .onClick(() => {
        let audioTrack = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId === this.selectedAudioTrackId);
        let audioAsset =
          audioTrack?.audioAssetArray?.find(audioAsset => audioAsset.startTime === this.selectedAudioAssetStartTime);
        let node = audioAsset?.nodes?.find(node => node.type === type);
        this.selectedEffectNodeId = node?.id ?? '';
        this.selectedEffectNode = type;
        this.effectNodeDialog?.open();
      })
    }
  }

  @Builder
  Node(type: NodeType, isShow?: boolean) {
    Text(type ?? '-')
      .height($r('app.float.height_30'))
      .width($r('app.float.width_50'))
      .margin({ right: $r('app.float.margin_5') })
      .textAlign(TextAlign.Center)
      .backgroundColor(isShow ? ColorMap.get(type) : Color.Gray)
      .borderRadius(5)
      .fontColor(Color.White)
      .maxLines(2)
      .textOverflow({ overflow: TextOverflow.None })
  }
}