/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { SoundFiledType, NodeType } from '../../utils/InterfaceInfo'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_SoundSpeedTone';

@Component
export struct SoundSpeedToneComponent {
  // speed of sound
  @Prop selectedSoundSpeed: number;
  // pitch
  @Prop selectedSoundTone: number;
  @State soundSpeed: number = this.selectedSoundSpeed;
  @State soundTone: number = this.selectedSoundTone;
  onCancel?: () => void;
  onAgree?: (soundSpeed: number, soundTone: number) => void;

  build() {
    Column() {
      Row() {
        Text(`${NodeType.SOUND_SPEED_TONE}`)
          .margin({ bottom: $r('app.float.margin_10') })
      }
      .margin({ top: $r('app.float.margin_10') })

      Column() {
        Column() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text('音速')
            Text(`${this.soundSpeed.toFixed(1)}x`)
          }.padding($r('app.float.padding_10'))
          Slider({
            value: this.soundSpeed,
            step: 0.1,
            style: SliderStyle.OutSet,
            min: 0.5,
            max: 10,
          })
            .selectedColor($r('app.color.index_audio_clip_card'))
            .onChange((value: number, mode: SliderChangeMode) => {
              this.soundSpeed = value;
            })
        }
        .height($r('app.float.height_50'))
        .margin({bottom: $r('app.float.margin_10')})
        .padding($r('app.float.padding_10'))
        .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
        .borderRadius($r('app.float.borderRadius_10'))
        Column() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text('音调')
            Text(`+${this.soundTone.toFixed(1)}`)
          }.padding($r('app.float.padding_10'))
          Slider({
            value: this.soundTone,
            step: 0.1,
            style: SliderStyle.OutSet,
            min: 0.1,
            max: 5,
          })
            .selectedColor($r('app.color.index_audio_clip_card'))
            .onChange((value: number, mode: SliderChangeMode) => {
              this.soundTone = value;
            })
        }
        .height($r('app.float.height_50'))
        .padding($r('app.float.padding_10'))
        .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
        .borderRadius($r('app.float.borderRadius_10'))
      }
      .margin({right: $r('app.float.margin_20')})
      .padding({left: $r('app.float.padding_10'), right: $r('app.float.padding_10')})

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Button('取消')
          .onClick(() => {
            if (this.onCancel) {
              this.onCancel()
            }
          })
        Button('确定')
          .onClick(() => {
            if (this.onAgree) {
              this.onAgree(this.soundSpeed, this.soundTone);
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
      .padding($r('app.float.padding_20'))
    }
  }
}