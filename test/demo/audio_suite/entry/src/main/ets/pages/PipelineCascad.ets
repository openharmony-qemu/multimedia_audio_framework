/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */

import audioNapi from 'libentry.so';
import { MainTitleTextModifier, saveMulFileBuffer } from "../utils/ExportFile";
import { SymbolGlyphModifier, window } from "@kit.ArkUI";
import {
  NavigationParams,
  OFFLINEPIPELINE,
  outputNodeSongNameMap,
  OutSong,
  PIPELINECASCADMODE_1,
  PIPELINECASCADMODE_2,
  PIPELINECASCADMODE_3
} from '../utils/importSongs/InterfaceInfo';
import { Logger } from "../utils/Logger";
import { taskpool, util } from "@kit.ArkTS";
import { PipelineInfo } from "../utils/autoTest/Interface";
import { common } from '@kit.AbilityKit';

@Builder
export function PipelineCascadBuilder() {
  PipelineCascad()
}

const TAG: string = 'AudioEditTestApp_PipelineCascad';

@Component
export struct PipelineCascad {
  pageInfos: NavPathStack = new NavPathStack();
  // 主标题样式修改器
  @State mainTitleModifier: MainTitleTextModifier = new MainTitleTextModifier();
  private symbolModifier: SymbolGlyphModifier =
    new SymbolGlyphModifier($r('sys.symbol.chevron_backward')).fontColor([$r('app.color.index_title_text_color')]);

  @StorageLink('pipelineCascadMap') pipelineCascadMap: Map<string, OutSong> = new Map();

  @StorageLink('isCurrentMulPipeline') isCurrentMulPipeline: boolean = false;
  @StorageLink('currentPipelineId') currentPipelineId: string = '';

  // 是否显示第三个+号
  @State isShow: boolean = false;

  @State outputName_1: string = '+';
  @State outputName_2: string = '+';

  private pipelineIdToPipelineInfoMap: Map<string, PipelineInfo> = new Map();
  private preparedPipelineList: OutSong[] = [];
  private isDestroyFinished: boolean = false;

  aboutToAppear(): void {
    Logger.info(TAG, 'aboutToAppear start');
    let context = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
    window.getLastWindow(context).then((windowClass) => {
      // 设置屏幕横屏
      windowClass.setPreferredOrientation(window.Orientation.AUTO_ROTATION_LANDSCAPE);
    });
    if (!this.initAllPipeline()) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: '初始化管线失败'
      });
    }
  }

  aboutToDisappear(): void {
    Logger.info(TAG, 'aboutToDisappear start');
    if (this.isDestroyFinished) {
      audioNapi.destroyMultiPipeline();
    }
    let context = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
    window.getLastWindow(context).then((windowClass) => {
      windowClass.setPreferredOrientation(window.Orientation.AUTO_ROTATION);
    });
  }

  build() {
    NavDestination() {
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceEvenly, alignItems: ItemAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Button(this.outputName_1)
            .width(150)
            .height(100)
            .fontColor($r('app.color.index_title_text_color'))
            .fontSize(20)
            .backgroundColor($r('app.color.index_audio_clip_card'))
            .margin({bottom: $r('app.float.margin_35')})
            .onClick(() => {
              this.pageInfos.pushPath({
                name: 'mixingConsole',
                param: { mode: OFFLINEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_1, pipelineId: this.preparedPipelineList[0].pipelineId } as NavigationParams,
                onPop: (popInfo: PopInfo) => {
                  Logger.info(TAG, 'PIPELINECASCADMODE_1 popInfo');
                  let outSong = popInfo.result as OutSong;
                  this.outputName_1 = outputNodeSongNameMap.get(outSong.pipelineCascadMode);
                  this.preparedPipelineList[0] = {
                    pipelineId: this.preparedPipelineList[0].pipelineId,
                    channels: outSong.channels,
                    sampleRate: outSong.sampleRate,
                    bitsPerSample: outSong.bitsPerSample,
                    pipelineCascadMode: PIPELINECASCADMODE_1,
                    isRenderer: true,
                  };
                }
              });
            })
          Button(this.outputName_2)
            .width(150)
            .height(100)
            .fontColor($r('app.color.index_title_text_color'))
            .fontSize(20)
            .backgroundColor($r('app.color.index_audio_clip_card'))
            .onClick(() => {
              this.pageInfos.pushPath({
                name: 'mixingConsole',
                param: { mode: OFFLINEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_2, pipelineId: this.preparedPipelineList[1].pipelineId } as NavigationParams,
                onPop: (popInfo: PopInfo) => {
                  Logger.info(TAG, 'PIPELINECASCADMODE_2 popInfo');
                  let outSong = popInfo.result as OutSong;
                  this.outputName_2 = outputNodeSongNameMap.get(outSong.pipelineCascadMode);
                  this.preparedPipelineList[1] = {
                    pipelineId: this.preparedPipelineList[1].pipelineId,
                    channels: outSong.channels,
                    sampleRate: outSong.sampleRate,
                    bitsPerSample: outSong.bitsPerSample,
                    pipelineCascadMode: PIPELINECASCADMODE_2,
                    isRenderer: true,
                  };

                }
              });
            })
        }
        .padding($r('app.float.padding_30'))
        .width('%30')

        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Button($r('app.string.renderer'))
            .width(150)
            .height(100)
            .fontColor($r('app.color.index_title_text_color'))
            .fontSize(20)
            .backgroundColor($r('app.color.index_audio_clip_card'))
            .onClick(() => {
              for (let i = 0; i < this.preparedPipelineList.length; i++) {
                if (this.preparedPipelineList.every((item => (item.isRenderer === undefined || item.isRenderer === false)))) {
                  this.getUIContext().showAlertDialog({
                    title: '提示',
                    message: '左侧请先添加音频'
                  });
                  return;
                }
                if (this.preparedPipelineList[i].isRenderer) {
                  Logger.info(TAG,
                    `renderer start, pipelineCascadMode: ${this.preparedPipelineList[i].pipelineCascadMode}, pipelineId: ${this.preparedPipelineList[i].pipelineId}`);
                  audioNapi.multiPipelineEnvPrepare(this.preparedPipelineList[i].pipelineId);
                  audioNapi.multiSetFormat(this.preparedPipelineList[i].channels,
                    this.preparedPipelineList[i].sampleRate, this.preparedPipelineList[i].bitsPerSample);
                  taskpool.execute(saveMulFileBuffer, this.preparedPipelineList[i].pipelineId).then(async (value: Object) => {
                    if ((value as ArrayBuffer).byteLength !== 0) {
                      this.pipelineCascadMap.set(outputNodeSongNameMap.get(this.preparedPipelineList[i].pipelineCascadMode) || '', {
                        pipelineId: this.preparedPipelineList[i].pipelineId ?? '',
                        sampleRate: this.preparedPipelineList[i].sampleRate,
                        channels: this.preparedPipelineList[i].channels,
                        bitsPerSample: this.preparedPipelineList[i].bitsPerSample,
                        pipelineCascadMode: this.preparedPipelineList[i].pipelineCascadMode,
                        buffer: value as ArrayBuffer,
                        bufferLength: (value as ArrayBuffer).byteLength,
                      });
                      let temp = this.preparedPipelineList[i];
                      temp.isRendererFinished = true;
                      this.preparedPipelineList[i] = temp;

                      // 判断是否全部渲染完成了
                      this.isShow = this.preparedPipelineList.every((item => ((item.isRenderer === true &&
                        item.isRendererFinished === true) || (item.isRenderer === undefined))));
                    }
                  })
                }
              }
            })
        }
        .padding($r('app.float.padding_30'))
        .width('%30')

        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Button($r('app.string.add'))
            .width(150)
            .height(100)
            .enabled(this.isShow)
            .fontColor($r('app.color.index_title_text_color'))
            .fontSize(20)
            .backgroundColor($r('app.color.index_audio_clip_card'))
            .onClick(() => {
              audioNapi.destroyMultiPipeline();
              this.isDestroyFinished = true;
              this.pageInfos.pushPath({
                name: 'mixingConsole',
                param: { mode: OFFLINEPIPELINE, pipelineCascadMode: PIPELINECASCADMODE_3 } as NavigationParams
              });
            })
        }
        .padding($r('app.float.padding_30'))
        .width('%30')
      }
    }
    .onReady((context: NavDestinationContext) => {
      this.pageInfos = context.pathStack;
    })
    .title($r('app.string.pipeline_cascad'), {mainTitleModifier: this.mainTitleModifier})
    .height('100%')
    .width('100%')
    .backgroundColor($r('app.color.main_backgroundColor'))
    .backButtonIcon(this.symbolModifier)
  }

  initAllPipeline(): boolean {
    Logger.info(TAG, `initAllPipeline start`);
    for (let i = 0; i < 2; i++) {
      const pipelineId = util.generateRandomUUID(true);
      const pipelineInfo: PipelineInfo = {
        pipelineId: pipelineId,
        inputIdList: [],
        outputId: util.generateRandomUUID(true),
        mixerId: util.generateRandomUUID(true),
        multiRenderFrameFlag: false,
        sceneInfoId: ''
      };

      this.pipelineIdToPipelineInfoMap.set(pipelineId, pipelineInfo as PipelineInfo);
      let pipelineStatus = audioNapi.audioEditNodeInitMultiPipeline(pipelineId);
      if (pipelineStatus != 0) {
        Logger.error(TAG, `pipeline init ERROR, pipelineId :${pipelineId}`);
        return false;
      }
      this.preparedPipelineList.push({pipelineId: pipelineId, pipelineCascadMode: 0});
      Logger.info(TAG, `pipeline inited, pipelineId: ${pipelineId}`);
    }
    Logger.info(TAG, `all pipeline inited`);
    return true;
  }

}