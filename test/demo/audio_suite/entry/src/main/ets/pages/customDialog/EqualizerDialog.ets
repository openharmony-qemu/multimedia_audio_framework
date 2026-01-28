/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import {
  NodeType,
} from "../../utils/InterfaceInfo";
import { StyleConst } from '../../utils/StyleConst'

const TAG: string = 'AudioEditTestApp_EqualizerDialog';

@CustomDialog
@Component
export struct EqualizerDialog {
  // Balanced frequency band
  @Link balanceBandGains: number[];
  @State balanceBandGainsOfComponent: number[] = [...this.balanceBandGains];
  onCancel?: () => void;
  onAgree?: (balanceBandGains: number[]) => void;

  controller?: CustomDialogController;

  build() {
    Column() {
      Text(`${NodeType.EQ}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Flex({
        direction: FlexDirection.Row,
        justifyContent: FlexAlign.SpaceBetween,
        alignItems: ItemAlign.Center
      }) {
        ForEach(this.balanceBandGainsOfComponent, (item: number, index: number) => {
          Column() {
            Text(`${item}`).fontSize($r('app.float.font_size_12'))
            Slider({
              value: item,
              step: 1,
              style: SliderStyle.OutSet,
              direction: Axis.Vertical,
              min: -10,
              max: 10,
              reverse: true
            })
              .blockSize({ width: $r('app.float.width_30'), height: $r('app.float.height_30') })
              .blockBorderWidth(5)
              .blockColor(Color.White)
              // .showSteps(true)
              .onChange((value: number, mode: SliderChangeMode) => {
                this.balanceBandGainsOfComponent[index] = value;
              })
          }.width(StyleConst.FIVE_PERCENT_WIDTH).height($r('app.float.height_200'))
        })
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
            this.controller?.close();
          })
        Button('确定')
          .onClick(() => {
            if (this.onAgree) {
              this.onAgree(this.balanceBandGainsOfComponent);
            }
            this.controller?.close();
          })
      }.margin({ top: $r('app.float.margin_10') })
    }.padding($r('app.float.padding_10'))
  }
}