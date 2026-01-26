/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { SoundFiledType, NodeType } from '../../utils/InterfaceInfo'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_NoiseReduction';

@Component
export struct NoiseReductionComponent {
  onCancel?: () => void;
  onAgree?: () => void;

  build() {
    Column() {
      Text(`${NodeType.NR}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Button('添加降噪')
          .onClick(() => {
            if (this.onAgree) {
              this.onAgree();
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
    }
    .padding($r('app.float.padding_10'))
  }
}