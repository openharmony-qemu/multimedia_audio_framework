/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { VoiceBeautifierType, NodeType } from '../../utils/InterfaceInfo'
import { StyleConst } from '../../utils/StyleConst'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_VoiceBeautifier';

@Component
export struct VoiceBeautifierComponent {
  @Prop selectedVoiceBeautifier: VoiceBeautifierType;
  @State voiceBeautifierType: VoiceBeautifierType = this.selectedVoiceBeautifier;
  onCancel?: () => void;
  onAgree?: (voiceBeautifierType: VoiceBeautifierType) => void;

  build() {
    Column() {
      Text(`${NodeType.VB}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Row() {
        Column() {
          Radio({
            value: 'normal',
            group: 'voiceBeautifier'
          })
            .checked(this.voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL)
            .onChange(() => {
              this.voiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL;
            })
          Text('无').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({
            value: 'clear',
            group: 'voiceBeautifier'
          })
            .checked(this.voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR)
            .onChange(() => {
              this.voiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR;
            })
          Text('清澈').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)


        Column() {
          Radio({ value: 'theatre', group: 'voiceBeautifier' })
            .checked(this.voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_THEATRE)
            .onChange(() => {
              this.voiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_THEATRE;
            })
          Text('剧场').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'cd', group: 'voiceBeautifier' })
            .checked(this.voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CD)
            .onChange(() => {
              this.voiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CD;
            })
          Text('CD音效').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {

          Radio({ value: 'recordingStudio', group: 'voiceBeautifier' })
            .checked(this.voiceBeautifierType === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_STUDIO)
            .onChange(() => {
              this.voiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_STUDIO;
            })
          Text('录音棚').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)
      }

      Flex({
        direction: FlexDirection.Row,
        justifyContent: FlexAlign.SpaceBetween,
        alignItems: ItemAlign.Center
      }) {
        Button('取消')
          .onClick(() => {
            if (this.onCancel) {
              this.onCancel()
            }
          })
        Button('确定')
          .onClick(() => {
            if (this.onAgree) {
              this.onAgree(this.voiceBeautifierType);
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
    }
    .padding($r('app.float.padding_10'))
  }
}