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
  // Callback for real-time coordinate changes.
  onPositionChange?: (x: number, y: number, z: number, effectNodeId: string) => void;
  controller?: CustomDialogController;
  @State fixedPositionOpen: boolean = false
  @State dynamicRenderOpen: boolean = false
  @State extensionOpen: boolean = false
  @State fixedX: number = this.spaceRenderPara.fixedPosition?.x ?? 0;
  @State fixedY: number = this.spaceRenderPara.fixedPosition?.y ?? 0;
  @State fixedZ: number = this.spaceRenderPara.fixedPosition?.z ?? 0;
  // Refreshing a Trigger
  @State refreshTrigger: number = 0;
  @State spaceRenderState: SpaceRender = {
    mode: this.spaceRenderPara.mode ?? SpaceRenderMode.CLOSE,
    fixedPosition: {
      x: this.spaceRenderPara.fixedPosition?.x ?? 0,
      y: this.spaceRenderPara.fixedPosition?.y ?? 0,
      z: this.spaceRenderPara.fixedPosition?.z ?? 0
    } as SpacePosition,
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
  @Prop selectedEffectNodeId: string = '';

  private getFormattedCoords(): string {
    let x = 0, y = 0, z = 0;

    if (this.spaceRenderState.mode === SpaceRenderMode.FIXED_POSITION && this.spaceRenderState.fixedPosition) {
      x = this.fixedX;
      y = this.fixedY;
      z = this.fixedZ;
    } else if (this.spaceRenderState.mode === SpaceRenderMode.DYNAMIC_RENDER && this.spaceRenderState.dynamicPosition) {
      x = this.spaceRenderState.dynamicPosition.x;
      y = this.spaceRenderState.dynamicPosition.y;
      z = this.spaceRenderState.dynamicPosition.z;
    }

    return `x=${x.toFixed(2)}, y=${y.toFixed(2)}, z=${z.toFixed(2)}`;
  }

  // Processing real-time changes of fixed fixed coordinates
  private handleFixedPositionChange(axis: 'x' | 'y' | 'z', value: number) {
    if (this.spaceRenderState.mode !== SpaceRenderMode.FIXED_POSITION) {
      console.warn('handleFixedPositionChange: not in fixed position mode, skip');
      return;
    }

    const minValue = -5.0;
    const maxValue = 5.0;
    const clampedValue = Math.max(minValue, Math.min(maxValue, parseFloat(value.toFixed(2))));

    // Check whether fixedPosition exists. If it does not exist, initialize it.
    if (!this.spaceRenderState.fixedPosition) {
      this.spaceRenderState.fixedPosition = { x: 0, y: 0, z: 0 } as SpacePosition;
    }

    if (axis === 'x') {
      this.fixedX = clampedValue;
      this.spaceRenderState.fixedPosition.x = clampedValue;
    } else if (axis === 'y') {
      this.fixedY = clampedValue;
      this.spaceRenderState.fixedPosition.y = clampedValue;
    } else if (axis === 'z') {
      this.fixedZ = clampedValue;
      this.spaceRenderState.fixedPosition.z = clampedValue;
    }

    this.spaceRenderState.fixedPosition = {
      x: this.fixedX,
      y: this.fixedY,
      z: this.fixedZ
    };

    // Real-time callback, which is used to transfer the latest coordinates and effectNodeId.
    if (this.onPositionChange) {
      console.log(`handleFixedPositionChange: ${axis}=${value}, current x=${this.spaceRenderState.fixedPosition.x}, y=${this.spaceRenderState.fixedPosition.y}, z=${this.spaceRenderState.fixedPosition.z}`);
      this.onPositionChange(
        this.fixedX,
        this.fixedY,
        this.fixedZ,
        this.selectedEffectNodeId
      );
    }
    this.refreshTrigger = (this.refreshTrigger + 1) % 1000;
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
                  // When switching to a fixed position, an update is immediately triggered.
                  const pos = this.spaceRenderState.fixedPosition;
                  this.fixedX = this.spaceRenderState.fixedPosition?.x ?? 0;
                  this.fixedY = this.spaceRenderState.fixedPosition?.y ?? 0;
                  this.fixedZ = this.spaceRenderState.fixedPosition?.z ?? 0;
                  if (pos) {
                    if (this.onPositionChange) {
                      this.onPositionChange(
                        this.fixedX,
                        this.fixedY,
                        this.fixedZ,
                        this.selectedEffectNodeId
                      );
                    }
                  }
                }
                this.refreshTrigger = (this.refreshTrigger + 1) % 1000;
              })
          }
        }
        .backgroundColor(0xFFFFFF)
        .margin($r('app.float.margin_5'))

        // This line of content is displayed only when the button is turned on.
        if (this.spaceRenderState.mode === 0) {
          this.RenderView()
          this.RenderFixedPositionControls()
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
            .key(`coords-text-${this.refreshTrigger}`)
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

  @Builder
  RenderFixedPositionControls() {

    Column() {
      Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Text('X 坐标')
          .fontSize($r('app.float.font_size_14'))
          .margin({ left: $r('app.float.margin_10') })
        Text(`${this.spaceRenderState.fixedPosition?.x.toFixed(2) ?? '0.00'}`)
          .fontSize($r('app.float.font_size_14'))
          .margin({ right: $r('app.float.margin_10') })
      }
      Slider({
        value: this.fixedX,
        min: -5.0,
        max: 5.0,
        step: 0.01,
        style: SliderStyle.OutSet
      })
        .onChange((value: number) => {
          this.handleFixedPositionChange('x', value);
        })
        .margin({ left: $r('app.float.margin_10'), right: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })

      Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Text('Y 坐标')
          .fontSize($r('app.float.font_size_14'))
          .margin({ left: $r('app.float.margin_10') })
        Text(`${this.spaceRenderState.fixedPosition?.y.toFixed(2) ?? '0.00'}`)
          .fontSize($r('app.float.font_size_14'))
          .margin({ right: $r('app.float.margin_10') })
      }
      Slider({
        value: this.fixedY,
        min: -5.0,
        max: 5.0,
        step: 0.01,
        style: SliderStyle.OutSet
      })
        .onChange((value: number) => {
          this.handleFixedPositionChange('y', value);
        })
        .margin({ left: $r('app.float.margin_10'), right: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })

      Flex({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Text('Z 坐标')
          .fontSize($r('app.float.font_size_14'))
          .margin({ left: $r('app.float.margin_10') })
        Text(`${this.spaceRenderState.fixedPosition?.z.toFixed(2) ?? '0.00'}`)
          .fontSize($r('app.float.font_size_14'))
          .margin({ right: $r('app.float.margin_10') })
      }
      Slider({
        value: this.fixedZ,
        min: -5.0,
        max: 5.0,
        step: 0.01,
        style: SliderStyle.OutSet
      })
        .onChange((value: number) => {
          this.handleFixedPositionChange('z', value);
        })
        .margin({ left: $r('app.float.margin_10'), right: $r('app.float.margin_10') })
    }
    .backgroundColor(0xFFFFFF)
    .margin($r('app.float.margin_5'))
    .borderRadius($r('app.float.borderRadius_15'))
    .padding($r('app.float.padding_10'))
    .key(`fixed-position-${this.refreshTrigger}`)
  }

}