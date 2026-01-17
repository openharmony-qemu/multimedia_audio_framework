/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { AudioSeparationType, NodeType } from '../../utils/InterfaceInfo'
import { StyleConst } from '../../utils/StyleConst'
import { Logger } from '../../utils/Logger'

const TAG: string = 'AudioEditTestApp_AudioSeparation';

@Component
export struct AudioSeparationComponent {
  @Prop selectedAudioSeparation: AudioSeparationType;
  onCancel?: () => void;
  onAgree?: (audioSeparationPara: AudioSeparationType) => void;
  @State separatingShow: boolean = false;
  @State aissProgress: number = 0;
  @State audioSeparationPara: AudioSeparationType = this.selectedAudioSeparation;

  build() {
    Column() {
      Row() {
        Column() {
          Image($r('app.media.close_window'))
            .width($r('app.float.width_20'))
            .height($r('app.float.height_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)
        .onClick(() => {
          if (this.onCancel) {
            this.onCancel();
          }
        })

        Column() {
          Text('音源分离').fontSize($r('app.float.font_size_20'))
          Text('提取的音频类型将作于后续节点输入').fontSize($r('app.float.font_size_10')).opacity(50)
        }
        .width(StyleConst.FIFTY_PERCENT_WIDTH)

        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
          Button('提取').onClick(() => {
            if (this.onAgree) {
              this.onAgree(this.audioSeparationPara);
            }
          })
            .height($r('app.float.height_50'))
        }
      }
      .height($r('app.float.height_100'))
      .alignItems(VerticalAlign.Center)

      Row() {
        Column() {
          Row() {
            Text('人声').fontSize($r('app.float.font_size_20')).margin({ right: $r('app.float.margin_100') })
            if (this.separatingShow && this.aissProgress < 100) {
              Progress({ value: this.aissProgress, total: 150, type: ProgressType.Ring })
                .width($r('app.float.width_30'))
                .height($r('app.float.height_30'))
            } else {
              Radio({ value: 'human', group: 'separation' })
                .checked(this.audioSeparationPara ===
                AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE)
                .onChange((isOn: boolean) => {
                  this.audioSeparationPara =
                    AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE
                })
            }
          }
          .height($r('app.float.height_50'))

          Row() {
            Text('伴奏 ').fontSize($r('app.float.font_size_20')).margin({ right: $r('app.float.margin_100') })
            if (this.separatingShow && this.aissProgress <= 100) {
              Progress({ value: this.aissProgress, total: 150, type: ProgressType.Ring })
                .width($r('app.float.width_30'))
                .height($r('app.float.height_30'))
            } else {
              Radio({ value: 'backGround', group: 'separation' })
                .checked(this.audioSeparationPara ===
                AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE)
                .onChange((isOn: boolean) => {
                  if (isOn) {
                    this.audioSeparationPara =
                      AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE
                  }
                })
            }
          }
          .height($r('app.float.height_50'))
        }

      }
      .height($r('app.float.height_100'))

    }
  }
}