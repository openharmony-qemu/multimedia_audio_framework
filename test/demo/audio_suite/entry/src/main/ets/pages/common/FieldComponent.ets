/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { SoundFiledType, NodeType } from '../../utils/InterfaceInfo'
import { StyleConst } from '../../utils/StyleConst'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_Field';

@Component
export struct FieldComponent {
  @Prop selectedSoundFiled: SoundFiledType;
  @State soundField: SoundFiledType = this.selectedSoundFiled;
  onCancel?: () => void;
  onAgree?: (soundFieldType: SoundFiledType) => void;

  build() {
    Column() {
      Text(`${NodeType.FIELD}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Row() {
        Column() {
          Radio({ value: 'close', group: 'fieldEffect' })
            .checked(this.soundField === SoundFiledType.SOUND_FIELD_CLOSE)
            .onChange(() => {
              this.soundField = SoundFiledType.SOUND_FIELD_CLOSE;
            })
          Text('无').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'front', group: 'fieldEffect' })
            .checked(this.soundField === SoundFiledType.SOUND_FIELD_FRONT_FACING)
            .onChange(() => {
              this.soundField = SoundFiledType.SOUND_FIELD_FRONT_FACING;
            })
          Text('前置').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'grand', group: 'fieldEffect' })
            .checked(this.soundField === SoundFiledType.SOUND_FIELD_GRAND)
            .onChange(() => {
              this.soundField = SoundFiledType.SOUND_FIELD_GRAND;
            })
          Text('宏大').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'near', group: 'fieldEffect' })
            .checked(this.soundField === SoundFiledType.SOUND_FIELD_NEAR)
            .onChange(() => {
              this.soundField = SoundFiledType.SOUND_FIELD_NEAR;
            })
          Text('聆听').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {

          Radio({ value: 'broad', group: 'fieldEffect' })
            .checked(this.soundField === SoundFiledType.SOUND_FIELD_WIDE)
            .onChange(() => {
              this.soundField = SoundFiledType.SOUND_FIELD_WIDE;
            })
          Text('宽广').fontSize($r('app.float.font_size_20'))
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
              this.onAgree(this.soundField);
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
    }
    .padding($r('app.float.padding_10'))
  }
}