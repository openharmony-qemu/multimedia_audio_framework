/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { EnvironmentType, NodeType } from '../../utils/InterfaceInfo'
import { StyleConst } from '../../utils/StyleConst'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_Environment';

@Component
export struct EnvironmentComponent {
  @Prop selectedEnvironment: EnvironmentType;
  @State environmentType: EnvironmentType = this.selectedEnvironment;
  onCancel?: () => void;
  onAgree?: (environmentType: EnvironmentType) => void;

  build() {
    Column() {
      Text(`${NodeType.ENV}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Row() {
        Column() {
          Radio({ value: 'close', group: 'envEffect' })
            .checked(this.environmentType === EnvironmentType.ENVIRONMENT_TYPE_CLOSE)
            .onChange(() => {
              this.environmentType = EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
            })
          Text('无').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'broadcast', group: 'envEffect' })
            .checked(this.environmentType === EnvironmentType.ENVIRONMENT_TYPE_BROADCAST)
            .onChange(() => {
              this.environmentType = EnvironmentType.ENVIRONMENT_TYPE_BROADCAST;
            })
          Text('广播').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'earpiece', group: 'envEffect' })
            .checked(this.environmentType === EnvironmentType.ENVIRONMENT_TYPE_EARPIECE)
            .onChange(() => {
              this.environmentType = EnvironmentType.ENVIRONMENT_TYPE_EARPIECE;
            })
          Text('听筒').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'underWater', group: 'envEffect' })
            .checked(this.environmentType === EnvironmentType.ENVIRONMENT_TYPE_UNDERWATER)
            .onChange(() => {
              this.environmentType = EnvironmentType.ENVIRONMENT_TYPE_UNDERWATER;
            })
          Text('水下').fontSize($r('app.float.font_size_20'))
        }
        .width(StyleConst.TWENTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'gramophone', group: 'envEffect' })
            .checked(this.environmentType === EnvironmentType.ENVIRONMENT_TYPE_GRAMOPHONE)
            .onChange(() => {
              this.environmentType = EnvironmentType.ENVIRONMENT_TYPE_GRAMOPHONE;
            })
          Text('留声机').fontSize($r('app.float.font_size_20'))
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
              this.onAgree(this.environmentType);
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
    }
    .padding($r('app.float.padding_10'))
  }
}