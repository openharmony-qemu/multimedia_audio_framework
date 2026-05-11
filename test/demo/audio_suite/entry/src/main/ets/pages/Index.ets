/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. All rights reserved.
 */

import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import window from '@ohos.window';
import { BusinessError } from '@kit.BasicServicesKit';
import { display } from '@kit.ArkUI';
import { NavigationParams, OFFLINEPIPELINE, PIPELINECASCADMODE_0, REALTIMEPIPELINE } from '../utils/InterfaceInfo';
import audioNapi from 'libentry.so';
import { DirInitHelper } from '../utils/DirInit';

const TAG: string = 'AudioEditTestApp_Index';

class TitleTextModifier implements AttributeModifier<TextAttribute> {
  public applyNormalAttribute(instance: TextAttribute): void {
    instance.fontColor(Color.White)
      .fontSize($r('app.float.font_size_30'))
      .fontWeight(FontWeight.Bold)
  }
}

@Entry
@Component
struct Index {
  @Provide('pageInfos') pageInfos: NavPathStack = new NavPathStack()

  // Determine whether it is in landscape mode
  @StorageLink('isHorizontalScreen') isHorizontalScreen: boolean = false;

  @StorageLink('showDebugButton') showDebugButton: boolean = true;
  @StorageLink('debugButtonX') debugButtonX: number = 200;
  @StorageLink('debugButtonY') debugButtonY: number = 100;

  private scrollerIndex: Scroller = new Scroller();
  private initialButtonX: number = 0;
  private initialButtonY: number = 0;
  private screenWidth: number = 0;
  private screenHeight: number = 0;
  private buttonSize: number = 60;
  private saveButtonTimer: number = -1; // Timer for throttling save operations


  aboutToAppear(): void {
    // Initializes the status of the debugging button.
    AppStorage.setOrCreate('showDebugButton', true);
    AppStorage.setOrCreate('debugButtonX', 200);
    AppStorage.setOrCreate('debugButtonY', 100);
    // Obtaining the Screen Size
    const defaultDisplay = display.getDefaultDisplaySync();
    this.screenWidth = defaultDisplay.width;
    this.screenHeight = defaultDisplay.height;

    // Monitor screen size changes (including rotation)
    display.on('change', this.onDisplayChange);

    this.setSystemBarProperties();
  }

  aboutToDisappear(): void {
    Logger.info(TAG, 'aboutToDisappear');
    // Destroy screen listeners
    display.off('change', this.onDisplayChange);
    // Clear timer if exists
    if (this.saveButtonTimer !== -1) {
      clearTimeout(this.saveButtonTimer);
      this.saveButtonTimer = -1;
    }
  }

  // Listener function
  private onDisplayChange = (): void => {
    const defaultDisplay = display.getDefaultDisplaySync();
    this.isHorizontalScreen =
      (defaultDisplay.orientation === display.Orientation.PORTRAIT ||
        defaultDisplay.orientation === display.Orientation.PORTRAIT_INVERTED)
        ? false
        : true;

    // Updating the screen size.
    this.screenWidth = defaultDisplay.width;
    this.screenHeight = defaultDisplay.height;
    this.clampButtonPosition();
  };

  // Ensure that the button is within the visible range of the screen.
  private clampButtonPosition(): void {
    const maxX = this.screenWidth - this.buttonSize;
    const maxY = this.screenHeight - this.buttonSize;
    let clampedX = Math.min(Math.max(this.debugButtonX, 0), maxX);
    let clampedY = Math.min(Math.max(this.debugButtonY, 0), maxY);

    if (clampedX !== this.debugButtonX || clampedY !== this.debugButtonY) {
      this.debugButtonX = clampedX;
      this.debugButtonY = clampedY;

      AppStorage.setOrCreate('debugButtonX', this.debugButtonX);
      AppStorage.setOrCreate('debugButtonY', this.debugButtonY);

      Logger.info(TAG, `按钮位置已调整到可见范围: (${this.debugButtonX}, ${this.debugButtonY})`);
    }
  };


  // Set the background color of the status bar
  setSystemBarProperties() {
    let windowClass: window.Window | undefined = undefined;
    window.getLastWindow(getContext(), (err: BusinessError, data) => {
      windowClass = data
      let SystemBarProperties: window.SystemBarProperties = {
        statusBarColor: '#8161c4',
        statusBarContentColor: '#FFFFFF'
      };
      try {
        let promise = windowClass.setWindowSystemBarProperties(SystemBarProperties);
        promise.then(() => {
          Logger.info(TAG, 'Succeeded in setting the system bar properties.');
        }).catch((err: BusinessError) => {
          Logger.error(TAG,
            `Failed to set the system bar properties. Cause code: ${err.code}, message: ${err.message}`);
        });
      } catch (exception) {
        Logger.error(TAG,
          `Failed to set the system bar properties. Cause code: ${exception.code}, message: ${exception.message}`);
      }
    })
  }

  build() {
    Navigation(this.pageInfos) {
      Scroll(this.scrollerIndex) {
        Column() {
          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.icon_home_clip'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.music_editor'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.music_editor_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({ name: 'audioEdit' })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))
          .margin({bottom: $r('app.float.margin_10')})

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.icon_home_clip'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.mixing_console_playing'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.mixing_console_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({
                name: 'mixingConsole',
                param: { mode: REALTIMEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_0 } as NavigationParams
              })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .margin({bottom: $r('app.float.margin_10')})
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.icon_home_clip'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.mixing_console_cascade'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.mixing_console_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({
                name: 'mixingConsole',
                param: { mode: OFFLINEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_0 } as NavigationParams
              })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .margin({bottom: $r('app.float.margin_10')})
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.hand_tap'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.pipeline_cascad'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.pipeline_cascad_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({ name: 'pipelineCascad' })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .margin({ bottom: $r('app.float.margin_10') })
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.hand_tap'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.auto_test'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.auto_test_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({ name: 'autoTest' })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.icon_home_clip'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text($r('app.string.audio_track_edit'))
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text($r('app.string.audio_track_edit_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({
                name: 'audioTrackEdit',
                param: { mode: OFFLINEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_0 } as NavigationParams
              })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .margin({bottom: $r('app.float.margin_10'), top: $r('app.float.margin_10')})
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))

          Column() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
              Column() {
                Image($r('app.media.icon_home_clip'))
                  .width(StyleConst.SIXTY_PERCENT_WIDTH)
                  .height(StyleConst.SIXTY_PERCENT_HEIGHT)
                  .objectFit(ImageFit.Contain)
              }
              .width(StyleConst.THIRTY_PERCENT_WIDTH)

              Column() {
                Text('音频格式转换')
                  .fontSize($r('app.float.font_size_24'))
                  .fontColor($r('app.color.index_title_text_color'))
                Text('采样率、位深、声道转换')
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
              .width('100%')
              .alignItems(HorizontalAlign.Center)
              .justifyContent(FlexAlign.Center)
            }
            .width(StyleConst.FULL_WIDTH)
            .height(StyleConst.FULL_HEIGHT)
            .onClick(() => {
              this.pageInfos.pushPath({ name: 'audioConversion' })
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .height($r('app.float.height_100'))
          .padding({
            left: $r('app.float.padding_20'),
            right: $r('app.float.padding_20')
          })
          .margin({bottom: $r('app.float.margin_10'), top: $r('app.float.margin_10')})
          .border({ radius: $r('app.float.border_16') })
          .backgroundColor($r('app.color.index_audio_clip_card'))
        }
      }
    }
    .width(StyleConst.FULL_WIDTH)
    .height(StyleConst.FULL_HEIGHT)
    .title('音频编辑', { mainTitleModifier: new TitleTextModifier() })
    .mode(NavigationMode.Stack)
    // .padding({ left: $r('app.float.padding_20'), right: $r('app.float.padding_20') })
    .backgroundColor(Color.Black)
    .overlay(this.DebugDilogButton())
  }

    @Builder
  DebugDilogButton() {
    Column() {
      if (this.showDebugButton) {
        Button('调试')
          .fontSize(14)
          .fontWeight(FontWeight.Medium)
          .fontFamily('HarmonyHeiTi')
          .width(60)
          .height(60)
          .borderRadius(30)
          .backgroundColor('#409EFF')
          .fontColor(Color.White)
          .shadow({ radius: 10, color: '#00000040' })
          .position({ x: this.debugButtonX, y: this.debugButtonY })
          .hitTestBehavior(HitTestMode.Transparent)
          .gesture(
            PanGesture({ direction: PanDirection.All })
              .onActionStart(() => {
                this.initialButtonX = this.debugButtonX;
                this.initialButtonY = this.debugButtonY;
                Logger.info(TAG, 'Start dragging the debug button.');
              })
              .onActionUpdate((event: GestureEvent) => {
                let newX = this.initialButtonX + event.offsetX;
                let newY = this.initialButtonY + event.offsetY;

                const maxX = this.screenWidth - this.buttonSize;
                const maxY = this.screenHeight - this.buttonSize;

                this.debugButtonX = Math.min(Math.max(newX, 0), maxX);
                this.debugButtonY = Math.min(Math.max(newY, 0), maxY);

                if (this.saveButtonTimer !== -1) {
                  clearTimeout(this.saveButtonTimer);
                }
                this.saveButtonTimer = setTimeout(() => {
                  AppStorage.setOrCreate('debugButtonX', this.debugButtonX);
                  AppStorage.setOrCreate('debugButtonY', this.debugButtonY);
                  this.saveButtonTimer = -1;
                }, 100);
              })
              .onActionEnd(() => {
                Logger.info(TAG, `Drag end, position: (${this.debugButtonX}, ${this.debugButtonY})`);
              })
          )
          .onClick(async () => {
            Logger.info(TAG, '=== Manually triggering debugging ===');
            try {
              await DirInitHelper.ensureDirExists();
              let ret = audioNapi.testPrintInfoToFile();
              console.info(`Debugging information is printed in the log. Result code: ${ret}`);
            } catch (err) {
              Logger.error(TAG, `The debugging operation failed: ${err}`);
            }
          })
      }
    }
    .width('100%')
    .height('100%')
    .hitTestBehavior(HitTestMode.Transparent)
  }
}