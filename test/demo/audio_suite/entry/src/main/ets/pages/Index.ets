/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */

import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import window from '@ohos.window';
import { BusinessError } from '@kit.BasicServicesKit';
import { display } from '@kit.ArkUI';
import { NavigationParams, OFFLINEPIPELINE, PIPELINECASCADMODE_0, REALTIMEPIPELINE } from '../utils/importSongs/InterfaceInfo';

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

  private scrollerIndex: Scroller = new Scroller();

  aboutToAppear(): void {
    this.setSystemBarProperties();
    // Listen to screen orientation switch
    display.on('change', this.onDisplayChange);
  }

  aboutToDisappear(): void {
    Logger.info(TAG, 'aboutToDisappear');
    // Destroy screen listeners
    display.off('change', this.onDisplayChange);
  }

  // Listener function
  private onDisplayChange = (): void => {
    const defaultDisplay = display.getDefaultDisplaySync();
    this.isHorizontalScreen =
      (defaultDisplay.orientation === display.Orientation.PORTRAIT ||
        defaultDisplay.orientation === display.Orientation.PORTRAIT_INVERTED)
        ? false
        : true;
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
                  .margin({ bottom: $r('app.float.margin_10') })
                Text($r('app.string.music_editor_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
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
                  .margin({ bottom: $r('app.float.margin_10') })
                Text($r('app.string.mixing_console_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
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
                  .margin({ bottom: $r('app.float.margin_10') })
                Text($r('app.string.mixing_console_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
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
                  .margin({ bottom: $r('app.float.margin_10') })
                Text($r('app.string.pipeline_cascad_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
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
                  .margin({ bottom: $r('app.float.margin_10') })
                Text($r('app.string.auto_test_desc'))
                  .fontSize($r('app.float.font_size_20'))
                  .fontColor($r('app.color.index_title_text_color'))
                  .opacity(0.5)
              }
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
        }
      }
    }
    .width(StyleConst.FULL_WIDTH)
    .height(StyleConst.FULL_HEIGHT)
    .title('音频编辑', { mainTitleModifier: new TitleTextModifier() })
    .mode(NavigationMode.Stack)
    // .padding({ left: $r('app.float.padding_20'), right: $r('app.float.padding_20') })
    .backgroundColor(Color.Black)
  }
}