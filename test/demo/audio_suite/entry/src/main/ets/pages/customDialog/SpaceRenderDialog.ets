/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import {
  ClockWise,
  SpaceRender,
  SpaceRenderMode,
  SpaceRenderModeMap,
  ClockWiseMap,
  SpacePosition
} from "../../utils/InterfaceInfo";
import { SpaceRenderPositionView } from '../common/SpaceRender';
import { StyleConst } from '../../utils/StyleConst';

const TAG: string = 'AudioEditTestApp_SpaceRenderDialog';

@CustomDialog
@Component
export struct SpaceRenderDialog {
  @Prop spaceRenderPara: SpaceRender;
  onCancel?: () => void;
  onAgree?: (spaceRenderState: SpaceRender) => void;
  controller?: CustomDialogController;
  @State fixedPositionOpen: boolean = false
  @State dynamicRenderOpen: boolean = false
  @State extensionOpen: boolean = false
  @State spaceRenderState: SpaceRender = {
    mode: this.spaceRenderPara.mode ?? SpaceRenderMode.CLOSE,
    fixedPosition: {
      x: this.spaceRenderPara.fixedPosition?.x ?? 0,
      y: this.spaceRenderPara.fixedPosition?.y ?? 0,
      z: this.spaceRenderPara.fixedPosition?.z ?? 0
    },
    dynamicPosition: {
      x: this.spaceRenderPara.dynamicPosition?.x ?? 0,
      y: this.spaceRenderPara.dynamicPosition?.y ?? 0,
      z: this.spaceRenderPara.dynamicPosition?.z ?? 0
    },
    singleWeekCycleTime: this.spaceRenderPara.singleWeekCycleTime ?? 2,
    clockWise: this.spaceRenderPara.clockWise ?? ClockWise.CLOCK_WISE,
    expansionRadius: this.spaceRenderPara.expansionRadius ?? 1,
    expansionAngle: this.spaceRenderPara.expansionAngle ?? 1
  };
  scroller: Scroller = new Scroller();

  private getFormattedCoords(): string {
    let x = 0, y = 0, z = 0;

    if (this.spaceRenderState.mode === SpaceRenderMode.FIXED_POSITION && this.spaceRenderState.fixedPosition) {
      x = this.spaceRenderState.fixedPosition.x;
      y = this.spaceRenderState.fixedPosition.y;
      z = this.spaceRenderState.fixedPosition.z;
    } else if (this.spaceRenderState.mode === SpaceRenderMode.DYNAMIC_RENDER && this.spaceRenderState.dynamicPosition) {
      x = this.spaceRenderState.dynamicPosition.x;
      y = this.spaceRenderState.dynamicPosition.y;
      z = this.spaceRenderState.dynamicPosition.z;
    }

    return `x=${x.toFixed(2)}, y=${y.toFixed(2)}, z=${z.toFixed(2)}`;
  }

  build() {
    Scroll(this.scroller) {
      Column() {
        // First Line: Fixed Positioning
        Row() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text("固定摆位")
              .height($r('app.float.height_50'))
              .padding({ left: $r('app.float.padding_10') })
              .fontSize($r('app.float.font_size_16'))
              .textAlign(TextAlign.Start)
              .backgroundColor(0xFFFFFF)

            Toggle({ type: ToggleType.Switch, isOn: this.spaceRenderState.mode === 0 })
              .onClick((value) => {
                if (this.spaceRenderState.mode === 0) {
                  this.spaceRenderState.mode = -1;
                } else {
                  this.spaceRenderState.mode = 0;
                }
              })
          }
        }
        .backgroundColor(0xFFFFFF)
        .margin($r('app.float.margin_5'))

        // This line of content is displayed only when the button is turned on.
        if (this.spaceRenderState.mode === 0) {
          this.RenderView()
        }

        // Second Line: Dynamic Rendering
        Row() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text("动态渲染")
              .height($r('app.float.height_50'))
              .padding({ left: $r('app.float.padding_10') })
              .fontSize($r('app.float.font_size_16'))
              .textAlign(TextAlign.Start)
              .backgroundColor(0xFFFFFF)
            Toggle({ type: ToggleType.Switch, isOn: this.spaceRenderState.mode === 1 })
              .onClick((value) => {
                if (this.spaceRenderState.mode === 1) {
                  this.spaceRenderState.mode = -1;
                } else {
                  this.spaceRenderState.mode = 1;
                }
              })
          }
        }
        .backgroundColor(0xFFFFFF)
        .margin($r('app.float.margin_5'))

        if (this.spaceRenderState.mode === 1) {
          this.RenderView()
          Column() {
            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center
            }) {
              Text('单周环绕时间')
                .fontSize($r('app.float.font_size_14'))
                .margin({ left: $r('app.float.margin_10') })
              Text(`${this.spaceRenderState.singleWeekCycleTime}`)
                .fontSize($r('app.float.font_size_14'))
                .margin({ right: $r('app.float.margin_10') })
            }
            .margin({ top: $r('app.float.margin_10') })

            Row() {
              Slider({
                value: this.spaceRenderState.singleWeekCycleTime,
                min: 2,
                max: 40,
                style: SliderStyle.OutSet
              })
                .onChange((value: number) => {
                  this.spaceRenderState.singleWeekCycleTime = value;
                })
            }
          }
          .backgroundColor(0xFFFFFF)
          .margin($r('app.float.margin_5'))
          .borderRadius($r('app.float.borderRadius_15'))

          Row() {
            Radio({
              value: 'Radio1',
              group: 'radioGroup',
              indicatorType: RadioIndicatorType.DOT
            })
              .margin({ left: $r('app.float.margin_10') })
              .checked(this.spaceRenderState.clockWise === undefined ||
                this.spaceRenderState.clockWise === ClockWise.CLOCK_WISE)
              .onChange((isChecked: boolean) => {
                if (isChecked) {
                  this.spaceRenderState.clockWise = ClockWise.CLOCK_WISE;
                }
              })
            Text('顺时针')
              .fontSize($r('app.float.font_size_14'))
            Radio({
              value: 'Radio2',
              group: 'radioGroup',
              indicatorType: RadioIndicatorType.DOT
            })
              .margin({ left: $r('app.float.margin_10') })
              .checked(this.spaceRenderState.clockWise === ClockWise.Counter_Clock_Wise)
              .onChange((isChecked: boolean) => {
                if (isChecked) {
                  this.spaceRenderState.clockWise = ClockWise.Counter_Clock_Wise;
                }
              })
            Text('逆时针')
              .fontSize($r('app.float.font_size_14'))
          }
          .width(StyleConst.FULL_WIDTH)
          .margin({ bottom: $r('app.float.margin_10') })
          .justifyContent(FlexAlign.Start)
        }

        // Line 3: Expansion
        Row() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text("扩展")
              .height(50)
              .padding({ left: 10 })
              .fontSize(16)
              .textAlign(TextAlign.Start)
              .backgroundColor(0xFFFFFF)
            Toggle({ type: ToggleType.Switch, isOn: this.spaceRenderState.mode === 2 })
              .onClick((value) => {
                if (this.spaceRenderState.mode === 2) {
                  this.spaceRenderState.mode = -1;
                } else {
                  this.spaceRenderState.mode = 2;
                }
              })
          }
        }
        .backgroundColor(0xFFFFFF)
        .margin($r('app.float.margin_5'))

        if (this.spaceRenderState.mode === 2) {
          Column() {
            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center
            }) {
              Text('扩展半径')
                .fontSize(14)
                .margin({ left: $r('app.float.margin_10') })
              Text(`${this.spaceRenderState.expansionRadius}`)
                .fontSize(14)
                .margin({ right: $r('app.float.margin_10') })
            }
            .margin({ top: $r('app.float.margin_10') })

            Row() {
              Slider({
                value: this.spaceRenderState.expansionRadius,
                min: 1.0,
                max: 5.0,
                style: SliderStyle.OutSet
              })
                .onChange((value: number) => {
                  if (parseInt(value.toFixed(1)) === 0) {
                    this.spaceRenderState.expansionRadius = 0.1;
                  } else {
                    this.spaceRenderState.expansionRadius = Math.round(value);
                  }
                })
            }
          }
          .backgroundColor(0xFFFFFF)
          .margin($r('app.float.margin_5'))
          .borderRadius(15)

          Column() {
            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center
            }) {
              Text('扩展角度')
                .fontSize(14)
                .margin({ left: $r('app.float.margin_10') })
              Text(`${this.spaceRenderState.expansionAngle}`)
                .fontSize(14)
                .margin({ right: $r('app.float.margin_10') })
            }
            .margin({ top: $r('app.float.margin_10') })

            Row() {
              Slider({
                value: this.spaceRenderState.expansionAngle,
                min: 1,
                max: 359,
                style: SliderStyle.OutSet
              })
                .onChange((value: number) => {
                  this.spaceRenderState.expansionAngle = value;
                })
            }
          }
          .backgroundColor(0xFFFFFF)
          .margin($r('app.float.margin_5'))
          .borderRadius(15)
        }

        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
          Button('取消')
            .margin({ left: $r('app.float.margin_10') })
            .backgroundColor(0xFFFFFF)
            .fontColor(Color.Black)
            .onClick(() => {
              if (this.onCancel) {
                this.onCancel()
              }
              this.controller?.close();
            })
          Button('确定')
            .margin({ right: $r('app.float.margin_10') })
            .backgroundColor(0xFFFFFF)
            .fontColor(Color.Black)
            .onClick(() => {
              if (this.onAgree) {
                this.onAgree(this.spaceRenderState);
              }
              this.controller?.close();
            })
        }.margin({ top: $r('app.float.margin_10') })
      }
      .padding($r('app.float.padding_10'))
    }
  }

  @Builder
  RenderView() {
    Column() {
      Row() {
        Column() {
          Text('正视图')
            .fontSize($r('app.float.font_size_14'))
        }
        .width(StyleConst.FIFTY_PERCENT_WIDTH)

        Column() {
          Text('俯视图')
            .fontSize($r('app.float.font_size_14'))
        }
        .width(StyleConst.FIFTY_PERCENT_WIDTH)
      }
      .margin({ top: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })

      Row() {
        Column() {
          Text(this.getFormattedCoords())
            .fontSize($r('app.float.font_size_14'))
            .fontColor('#ffffff')
            .fontWeight(FontWeight.Bold)
            .background('#2C3E50')
            .borderRadius($r('app.float.borderRadius_8'))
            .padding({
              left: 12,
              right: 12,
              top: 6,
              bottom: 6
            })
            .width('auto')
            .textAlign(TextAlign.Center)
            .shadow({
              color: '#00000020',
              radius: 4,
              offsetX: 0,
              offsetY: 2
            })
            .margin({ bottom: 8 })
        }
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
      }
      .justifyContent(FlexAlign.Center)
      .alignItems(VerticalAlign.Center)
      .width(StyleConst.FULL_WIDTH)

      Row() {
        SpaceRenderPositionView({
          spaceRender: this.spaceRenderState
        })
      }
      .margin({ bottom: $r('app.float.margin_10') })
    }
  }
}