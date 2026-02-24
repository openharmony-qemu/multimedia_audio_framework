/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */
 
import { StyleConst } from "../utils/StyleConst";
import {
    Node,
    NodeType,
    ColorMap,
    SongInfo,
    OutSong,
    NavigationParams,
    PIPELINECASCADMODE_1,
    outputNodeSongNameMap,
    PIPELINECASCADMODE_2,
    PIPELINECASCADMODE_3,
    BitsPerSampleMode
} from "../utils/InterfaceInfo";
import { Logger } from '../utils/Logger';
import audioNapi from 'libentry.so';
import { importSingleSong } from '../utils/importSongs/ImportSongs';
import { SetFormatDialog } from './customDialog/SetFormatDialog';
import { CheckNodeDialog } from './customDialog/CheckNodeDialog';
import { JSON, util } from '@kit.ArkTS';
import { saveFileBuffer, RealTimeSaveFileBuffer, MainTitleTextModifier } from '../utils/ExportFile';
import { taskpool } from '@kit.ArkTS';
import { fileUri, picker } from '@kit.CoreFileKit';
import { common } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';
import fs, { ReadOptions } from '@ohos.file.fs';
import { AlertDialog, LoadingDialog, SymbolGlyphModifier, window } from '@kit.ArkUI';
import display from '@ohos.display';

import { getDateStringWithTimeStamp } from '../utils/Util';

const TAG: string = 'AudioEditTestApp_MixingConsole';

@Builder
export function MixingConsoleBuilder() {
    MixingConsole()
}

@Component
export struct MixingConsole {
    pageInfos: NavPathStack = new NavPathStack();
    // Main Title Style Modifier
    @State mainTitleModifier: MainTitleTextModifier = new MainTitleTextModifier();
    private symbolModifier: SymbolGlyphModifier =
        new SymbolGlyphModifier($r('sys.symbol.chevron_backward')).fontColor([$r('app.color.index_title_text_color')]);
    // Determine whether it is in landscape mode
    @StorageLink('isHorizontalScreen') isHorizontalScreen: boolean = false;

    @State param: NavigationParams = { mode: 0, pipelineCascadMode: 0 };
    @State paramMode: number = 0;
    @State inputList: SongInfo[] = [];
    private MAX_TRACKS: number = 5; // Maximum number of audio tracks
    @State nodeList: Map<string, SongInfo> = new Map();
    private wavBuffer: ArrayBuffer = new ArrayBuffer(0);
    private fd: number = -1;
    // Record or not?
    @State g_isRecord: boolean = false;
    // Exporting...
    @State isExported: boolean = false;
    @State mixerList: Array<Node> = [];
    @State mixerNode: Node = {
        id: util.generateRandomUUID(true),
        type: NodeType.MIXER,
        color: ColorMap[NodeType.MIXER]
    }
    @State outputNode: Node = {
        id: util.generateRandomUUID(true),
        type: NodeType.OUTPUT,
        color: ColorMap[NodeType.OUTPUT]
    }
    // Play or not?
    @State isPlay: boolean = false;
    @State isPlayRealTime: boolean = false;
    // Real-time playback (audio source separation)
    @Provide isSeparation: boolean = false;
    @State voiceType: string = NodeType.SEPARATION;
    // Export Style
    @State newAudioFileName: string = '';
    @State audioFormate: string = '.wav';
    @State sampleRate: number = -1;
    @State channels: number = -1;
    @State bitsPerSample: number = -1;
    // Node Operation Pop-up Parameters
    @State isLeft: boolean = true;
    @State selectedNode: string = '';
    @State nodeType: NodeType = NodeType.INPUT;
    @State finished: boolean = false;
    // Pipeline cascaded or not
    @State isPipelineCascad: boolean = false;
    // Pipeline cascading
    @StorageLink('pipelineCascadMap') pipelineCascadMap: Map<string, OutSong> = new Map();
    @StorageLink('isCurrentMulPipeline') isCurrentMulPipeline: boolean = false;
    @StorageLink('currentPipelineId') currentPipelineId: string = '';
    @State bitsPerSampleMode: number = BitsPerSampleMode.FLOAT;
    private isSaving: boolean = false;
    @State isAudioCache: boolean = false;

    aboutToAppear(): void {
        Logger.info(TAG, 'aboutToAppear');
        let context = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
        window.getLastWindow(context).then((windowClass) => {
            // Set screen to landscape mode
            windowClass.setPreferredOrientation(window.Orientation.AUTO_ROTATION_LANDSCAPE);
        });
        this.isPlayRealTime = false;
        // Initialize an empty list (reserve 5 empty slots)
        this.inputList = Array(this.MAX_TRACKS).fill(null);
        
        audioNapi.registerFinishedCallback((result: boolean) => {
            this.finished = result;
            Logger.info(TAG, `registerFinishedCallback finished: ${JSON.stringify(this.finished)}`);
            if (this.finished) {
                if (this.g_isRecord) {
                    taskpool.execute(RealTimeSaveFileBuffer).then(async (value: object) => {
                        Logger.info(TAG, 'taskpool onClick then');
                        this.SaveBuffer(value as ArrayBuffer);
                    });
                } else {
                    audioNapi.resetTotalWriteAudioDataSize();
                }
                this.isExported = false;
                this.g_isRecord = false;
                this.isPlay = false;
                audioNapi.audioRendererStop();
            }
        })

        audioNapi.registerAudioFormatCallback((result: string[]) => {
            Logger.info(TAG, `registerAudioFormatCallback string size: ${JSON.stringify(result.length)}, result: ${result}`);
            this.sampleRate = Number(result[0]);
            this.channels = Number(result[1]);
            switch (Number(result[2])) {
                case 0:
                    this.bitsPerSample = 8;
                    break;
                case 1:
                    this.bitsPerSample = 16;
                    break;
                case 2:
                    this.bitsPerSample = 24;
                    break;
                case 3:
                    this.bitsPerSample = 32;
                    this.bitsPerSampleMode = BitsPerSampleMode.INT;
                    break;
                case 4:
                    this.bitsPerSample = 32;
                    this.bitsPerSampleMode = BitsPerSampleMode.FLOAT;
                    break;
            }
        });
        audioNapi.registerAudioCacheCallback((result: boolean) => {
            Logger.info(TAG, `registerAudioCacheCallback result: ${result}`);
            if (!result) {
                this.importAudioProgress.close();
                this.isAudioCache = result;
                try {
                    fs.closeSync(this.fd);
                } catch (e) {
                    Logger.error(TAG, `Failed to close fd: ${e}`);
                }
            }
        });
    }

    aboutToDisappear(): void {
        Logger.info(TAG, 'aboutToDisappear');
        // Destroy the player
        try {
            let result = audioNapi.audioRendererDestory();
            Logger.info(TAG, `audioRendererDestory result: ${result}`);
        } catch (e) {
            Logger.info(TAG, `audioRendererDestory error: ${JSON.stringify(e)}`);
        }
        // Destroy the pipelines, destroy the engines.
        try {
            let result = audioNapi.audioEditDestory();
            Logger.info(TAG, `audioEditDestory result: ${result}`);
        } catch (e) {
            Logger.info(TAG, `audioEditDestory error: ${JSON.stringify(e)}`);
        }
        let context = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
        window.getLastWindow(context).then((windowClass) => {
            windowClass.setPreferredOrientation(window.Orientation.AUTO_ROTATION);
        });
        audioNapi.clear();
        try{
            audioNapi.unregisterFinishedCallback();
        }catch (e){
            Logger.error(TAG, `unregisterFinishedCallback faild`);
        }
        try{
            audioNapi.unregisterAudioFormatCallback();
        }catch (e){
            Logger.error(TAG, `unregisterAudioFormatCallback faild`);
        }
        try{
            audioNapi.unregisterAudioCacheCallback();
        }catch (e){
            Logger.error(TAG, `unregisterAudioCacheCallback faild`);
        }
    }

    importAudioProgress: CustomDialogController = new CustomDialogController({
        builder: LoadingDialog({
            content: '添加音频中...',
        }),
        autoCancel: false,
    })

    saveAudioProgress: CustomDialogController = new CustomDialogController({
        builder: LoadingDialog({
            content: '保存音频中...',
        }),
        autoCancel: false,
    })

    // Set up pop-up window
    dialogControllerSet: CustomDialogController | null = new CustomDialogController({
        builder: SetFormatDialog({
            newAudioFileName: this.newAudioFileName,
            sampleRate: this.sampleRate,
            channels: this.channels,
            bitsPerSample: this.bitsPerSample,
            bitsPerSampleMode: this.bitsPerSampleMode,
            nodeList: this.nodeList,
            cancel: () => {

            },
            confirm: () => {

            }
        }),
        maskColor: $r('app.color.mixing_console_mask_color'),
    });

    // View Effect Node Information Pop-up Window
    checkNodeDialogController: CustomDialogController | null = new CustomDialogController({
        builder: CheckNodeDialog({
            isLeft: this.isLeft,
            selectedNode: this.selectedNode,
            mixerList: this.mixerList,
            inputList: this.inputList,
            nodeList: this.nodeList,
            nodeType: this.nodeType,
            isPlayRealTime: this.isPlayRealTime,
            param: this.paramMode,
            cancel: () => {

            },
            confirm: () => {

            }
        }),
    });

    // Add Output Node Pop-up Window
    addOutputDialogController: CustomDialogController = new CustomDialogController({
        builder: AlertDialog({
            primaryTitle: '添加OUTPUT节点',
            primaryButton: {
                value: '取消'
            },
            secondaryButton: {
                value: '确认',
                role: ButtonRole.ERROR,
                action: () => {
                    const outputId = util.generateRandomUUID(true);
                    this.outputNode.id = outputId;
                }
            }
        }),
    });

    build() {
        NavDestination() {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
                Row() {
                    Image($r('app.media.ic_set'))
                        .width('30')
                        .height('30')
                        .onClick(() => {
                            // Pop-up, exported audio format
                            this.dialogControllerSet?.open();
                        })

                }
                Row() {
                    // save
                    Image($r('app.media.ic_save'))
                        .width('30')
                        .height('30')
                        .onClick(() => {
                            this.isSaving = true;
                            this.saveAudioProgress.open();
                            // Rendering on the pipeline cascade page
                            if (this.param.pipelineCascadMode === PIPELINECASCADMODE_1 || this.param.pipelineCascadMode === PIPELINECASCADMODE_2) {
                                this.pipelineCascadMap.set(outputNodeSongNameMap.get(this.param.pipelineCascadMode) || '', {
                                    pipelineId: this.param.pipelineId ?? '',
                                    sampleRate: this.sampleRate,
                                    channels: this.channels,
                                    bitsPerSample: this.bitsPerSample,
                                    pipelineCascadMode: this.param.pipelineCascadMode,
                                });
                                Logger.info(TAG, `this.pipelineCascadMap: ${this.pipelineCascadMap.size}`);
                                this.pageInfos.pop(this.pipelineCascadMap.get(outputNodeSongNameMap.get(this.param.pipelineCascadMode) || ''));
                                this.isSaving = false;
                                this.saveAudioProgress.close();                                
                                return;
                            }
                            if (this.sampleRate === -1) {
                                this.getUIContext().showAlertDialog({
                                    title: '提示',
                                    message: '请在设置页面输入采样率'
                                });
                                this.isSaving = false;
                                this.saveAudioProgress.close();                                
                                return;
                            }
                            if (this.channels === -1) {
                                this.getUIContext().showAlertDialog({
                                    title: '提示',
                                    message: '请在设置页面输入声道'
                                });
                                this.isSaving = false;
                                this.saveAudioProgress.close();
                                return;
                            }
                            if (this.bitsPerSample === -1) {
                                this.getUIContext().showAlertDialog({
                                    title: '提示',
                                    message: '请在设置页面输入位深'
                                });
                                this.isSaving = false;
                                this.saveAudioProgress.close();
                                return;
                            }
                            audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, this.bitsPerSampleMode);
                            taskpool.execute(saveFileBuffer).then(async (value: object) => {
                                Logger.info(TAG,
                                    `taskpool onClick save mixingConsole start, param.mode: ${this.param.mode}, length: ${(value as ArrayBuffer).byteLength}`);
                                this.SaveBuffer(value as ArrayBuffer);
                                // Set the totalWriteAudioDataSize in the buffer's map to 0
                                audioNapi.resetTotalWriteAudioDataSize();
                                this.isSaving = false;
                                this.saveAudioProgress.close();
                                Logger.info(TAG, 'save successfully');
                            });
                            audioNapi.stopPipeline();
                            audioNapi.resetTotalWriteAudioDataSize();
                        })
                }.visibility(this.param.mode === 1 ? Visibility.Visible : Visibility.None)
                Row() {
                    // (human) voice
                    Button($r('app.string.human_voice'), { type: ButtonType.Normal, stateEffect: true })
                        .backgroundColor($r('app.color.index_audio_clip_card'))
                        .fontColor(this.voiceType !== NodeType.VOICE ? $r('app.color.index_title_text_color') :
                            $r('app.color.selected_button'))
                        .fontSize(14)
                        .borderRadius(10)
                        .margin({ right: $r('app.float.margin_10') })
                        .visibility(this.isSeparation ? Visibility.Visible : Visibility.None)
                        .onClick(() => {
                            if (this.isSeparation) {
                                this.voiceType = NodeType.VOICE;
                                audioNapi.setSeparationMode(0);
                            }
                        })
                    // Accompaniment
                    Button($r('app.string.accompaniment'), { type: ButtonType.Normal, stateEffect: true })
                        .backgroundColor($r('app.color.index_audio_clip_card'))
                        .fontColor(this.voiceType !== NodeType.ACCOMPANIMENT ? $r('app.color.index_title_text_color') :
                            $r('app.color.selected_button'))
                        .fontSize(14)
                        .borderRadius(10)
                        .margin({ right: $r('app.float.margin_10') })
                        .visibility(this.isSeparation ? Visibility.Visible : Visibility.None)
                        .onClick(() => {
                            if (this.isSeparation) {
                                this.voiceType = NodeType.ACCOMPANIMENT;
                                audioNapi.setSeparationMode(1);
                            }
                        })
                    // Recording
                    Image(this.g_isRecord ? $r('app.media.ic_record_circle_2') : $r('app.media.ic_record_circle'))
                        .width('30')
                        .height('30')
                        .margin({right: $r('app.float.margin_5')})
                        .onClick(() => {
                            audioNapi.record();
                            this.g_isRecord = !this.g_isRecord;
                            audioNapi.setIsRecord(this.g_isRecord);
                        })
                    // Play
                    Image(!this.isPlay ? $r('app.media.ic_record_playing') : $r('app.media.ic_record_paused'))
                        .width('30')
                        .height('30')
                        .margin({right: $r('app.float.margin_5')})
                        .onClick(() => {
                            if (this.nodeList.size === 0) {
                                this.getUIContext().showAlertDialog({
                                    title: '提示',
                                    message: '至少选中一个音频才可以播放'
                                });
                                return;
                            }
                            if (this.isSeparation && this.voiceType === NodeType.SEPARATION) {
                                this.getUIContext().showAlertDialog({
                                    title: '提示',
                                    message: '请选择左侧播放人声或伴奏'
                                });
                                return;
                            }
                            this.isPlayRealTime = true;
                            this.isPlay = !this.isPlay;
                            audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, this.bitsPerSampleMode);
                            // Play
                            if (this.isPlay) {
                                // initialize Renderer
                                audioNapi.audioRendererInit();
                                // Click to play, then start AudioRenderer
                                audioNapi.audioRendererStart();
                            } else {
                                // Pause
                                audioNapi.audioRendererStop();
                                // Set the totalWriteAudioDataSize in the buffer's map to 0.
                                audioNapi.resetTotalWriteAudioDataSize();
                            }
                        })
                }
                .alignItems(VerticalAlign.Center)
                .visibility(this.param.mode === 2 ? Visibility.Visible : Visibility.None)
            }
            .height(this.isHorizontalScreen ? '15%' : "10%")
            .width(StyleConst.NINETY_FIVE_PERCENT_WIDTH)
            .borderRadius('10')
            .margin({bottom: $r('app.float.margin_10')})
            .padding({ left: $r('app.float.margin_35'), right: $r('app.float.margin_35') })
            .backgroundColor($r('app.color.index_audio_clip_card'))

            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween }) {
                // Left side
                Column() {
                    ForEach(this.inputList, (info: SongInfo, index: number) => {
                        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start }) {
                            Row() {
                                if(info && this.nodeList.has(info.nodeId || '')) {
                                    ForEach(Array.from(this.nodeList.get(info?.nodeId || '')?.nodes || []), (node: Node) => {
                                        Column() {
                                            this.EffectNode(true, node.type, node.id, info)
                                        }.margin({top: $r('app.float.margin_5')})
                                    })
                                } else {
                                    Column() {
                                        this.Node(true, info);
                                    }.onClick((event: ClickEvent) => {
                                        const inputId: string = util.generateRandomUUID(true);
                                        Logger.info(TAG, `inputId: ${inputId}`);
                                        importSingleSong(this.nodeList, inputId).then((songMap: Map<string, SongInfo>) => {
                                            // Add the input node to the nodes as well.
                                            let inputInfo: Node = {
                                                id: inputId,
                                                type: NodeType.INPUT,
                                                color: ColorMap.get(NodeType.INPUT)
                                            }
                                            let nodesArray: Array<Node> = [];
                                            nodesArray.push(inputInfo);
                                            let tmpInfo: SongInfo = songMap.entries().next().value;
                                            tmpInfo.nodes = nodesArray;
                                            // Store the imported audio files in nodeList.
                                            for (let value of songMap.values()) {
                                                Logger.error(TAG, `${value.nodeId} --- ${inputId}`);
                                                if (value.nodeId === inputId) {
                                                    this.inputList[index] = value;
                                                    this.getPcmFileBuffer(this.inputList[index]);
                                                    if (this.wavBuffer.byteLength !== 0) {
                                                        // Pipeline cascading
                                                        if (this.param.pipelineCascadMode !== 0 && this.param.pipelineCascadMode !== 3) {
                                                            if (!this.param.pipelineId || this.param.pipelineId === '') {
                                                                this.getUIContext().showAlertDialog({
                                                                    title: '提示',
                                                                    message: 'pipelineId是空'
                                                                });
                                                                return;
                                                            }
                                                            if (audioNapi.multiPipelineEnvPrepare(this.param.pipelineId) !== 0) {
                                                                this.getUIContext().showAlertDialog({
                                                                    title: '提示',
                                                                    message: '创建管线失败'
                                                                });
                                                                return;
                                                            }
                                                        }
                                                        this.isAudioCache = true;
                                                        this.importAudioProgress.open();
                                                        let nodeIds: string[] = [inputId, this.outputNode.id, this.mixerNode.id];
                                                        audioNapi.audioInAndOutInit(nodeIds, this.fd, this.wavBuffer.byteLength, 0);
                                                        Logger.error(TAG, `fd: ${this.fd}`);
                                                    }
                                                    setTimeout(() => {
                                                        if (tmpInfo) {
                                                            tmpInfo.sampleRate = this.sampleRate;
                                                            tmpInfo.channels = this.channels;
                                                            tmpInfo.bitsPerSample = this.bitsPerSample;
                                                        }
                                                    }, 100);
                                                    this.nodeList.set(inputId, tmpInfo);
                                                }
                                            }

                                            Logger.error(TAG, `Node list size: ${this.nodeList.size}`);
                                            Logger.error(TAG, `input list size: ${this.inputList.length}`);
                                            // Add a mixing node for each additional input.
                                            if (this.nodeList.size > 1 && this.mixerNode.id === '') {
                                                Logger.info(TAG, 'LEFT ADD');
                                                const mixerNodeId = util.generateRandomUUID(true);
                                                this.mixerNode.id = mixerNodeId;
                                            }
                                        });
                                    })
                                    .backgroundColor($r('app.color.index_audio_clip_card'))
                                    .margin({top: $r('app.float.margin_5')})
                                }
                            }
                        }
                        .height('20%')
                        .width('100%')
                    })
                }
                .height('100%')
                .layoutWeight(1)
                .backgroundColor($r('app.color.mixing_console_backgroundColor'))
                .padding({left: $r('app.float.margin_10'), right: $r('app.float.margin_10')})

                Blank().width(1).backgroundColor($r('app.color.index_audio_clip_card'))

                // Right side
                Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                    Row() {
                        if (this.nodeList.size > 1) {
                            Column() {
                                this.EffectNode(false, NodeType.MIXER, this.mixerNode.id);
                            }
                        }
                        ForEach(this.mixerList, (node: Node) => {
                            Column() {
                                this.EffectNode(false, node.type, node.id);
                            }
                        })
                        if (this.nodeList.size > 0) {
                            Column() {
                                this.EffectNode(false, NodeType.OUTPUT, this.outputNode.id);
                            }
                        }

                        if (this.outputNode.id === '') {
                            Column() {
                                this.Node(false);
                            }.onClick(() => {
                                if (this.nodeList.size > 0) {
                                    this.addOutputDialogController.open();
                                } else {
                                    this.getUIContext().showAlertDialog({
                                        title: '提示',
                                        message: '添加输入节点后才可以添加输出节点'
                                    });
                                }
                            })
                        }
                    }
                }
                .height('100%')
                .width('38%')
                .backgroundColor($r('app.color.mixing_console_backgroundColor'))
            }
            .height('90%')
            .width(StyleConst.NINETY_FIVE_PERCENT_WIDTH)
            .backgroundColor($r('app.color.index_audio_clip_card'))
        }
        .onReady((context: NavDestinationContext) => {
            this.pageInfos = context.pathStack;
            this.param = (context.pathInfo.param) as NavigationParams;
            if (this.param.pipelineCascadMode === 0 || this.param.pipelineCascadMode === 3) {
                this.isCurrentMulPipeline = false;
            }
            if (this.param.pipelineCascadMode === 1 || this.param.pipelineCascadMode === 2) {
                this.isCurrentMulPipeline = true;
                if (this.param.pipelineId) {
                    this.currentPipelineId = this.param.pipelineId;
                }
            }
            this.paramMode = this.param.mode;
            Logger.info(TAG, `this.param.mode is ${this.param.mode}`);
            // Work Mode
            if (this.param.mode !== 0 && this.param.pipelineCascadMode !== 1 && this.param.pipelineCascadMode !== 2) {
                try {
                    let result = audioNapi.audioEditNodeInit(this.param.mode);
                    Logger.info(TAG, `audioEditNodeInit result: ${result}`);
                } catch (e) {
                    Logger.error(TAG, `audioEditNodeInit error: ${JSON.stringify(e)}`);
                }
            } else {
                // Pop-up window
               Logger.info(TAG, `this.param.mode is invalid :${this.param.mode}`);
            }
            // Pipeline cascading
            if (this.param.pipelineCascadMode === PIPELINECASCADMODE_3 && this.pipelineCascadMap.size > 0) {
                this.pipelineCascadMap.forEach((value, key) => {
                    const inputId: string = util.generateRandomUUID(true);
                    let inputInfo: Node = {
                        id: inputId,
                        type: NodeType.INPUT,
                        color: ColorMap.get(NodeType.INPUT)
                    };
                    let tempInfo: SongInfo = {
                        uri: inputId,
                        sampleRate: value.sampleRate,
                        channels: value.channels,
                        bitsPerSample: value.bitsPerSample,
                        songName: key,
                        songType: NodeType.INPUT,
                        nodeId: inputId,
                        nodes: [inputInfo]
                    };
                    this.nodeList.set(inputId, tempInfo);
                    this.inputList[value.pipelineCascadMode - 1] = tempInfo;
                    let audioFormat = `${value.sampleRate} ${value.channels} ${value.bitsPerSample} ${value.bufferLength}`;
                    audioNapi.initByPipelineCascad(inputId, this.outputNode.id, this.mixerNode.id, audioFormat, value.buffer);
                });
            }
        })
        .title(this.param.mode === 1 ? $r('app.string.mixing_console_cascade') : $r('app.string.mixing_console_playing'),
            {mainTitleModifier: this.mainTitleModifier})
        .height('100%')
        .width('100%')
        .backgroundColor($r('app.color.main_backgroundColor'))
        .backButtonIcon(this.symbolModifier)
    }

    @Builder 
    EffectNode(isLeft : boolean, type : NodeType, nodeId : string, info ?: SongInfo) {
        Column() {
            this.Node(isLeft, info, type);
        }.onClick(() => {
            this.isLeft = isLeft;
            this.selectedNode = nodeId;
            this.nodeType = type;
            Logger.info(TAG, `EffectNode nodeId: ${nodeId} ${isLeft}`);
            if (type !== NodeType.INPUT) {
                this.checkNodeDialogController?.open();
            } else {
                // Copy a copy of the nodes data in advance
                let nodesArray = this.nodeList.get(nodeId)?.nodes ?? [];
                importSingleSong(this.nodeList, nodeId).then((songMap: Map<string, SongInfo>) => {
                    Logger.error(TAG, `---- nodesArray length is ${nodesArray.length}`);
                    let songInfo = songMap.get(nodeId)
                    if(songInfo){
                        let inputInfo: Node = {
                            id: nodeId,
                            type: NodeType.INPUT,
                            color: ColorMap.get(NodeType.INPUT)
                        }
                        if(nodesArray.length > 0){
                            nodesArray[0] = inputInfo
                        } else {
                            nodesArray = [inputInfo]
                        }
                        Logger.error(TAG, `nodesArray length is ${nodesArray.length}`);
                        songInfo.nodes = nodesArray;
                        // get audio parameters
                        this.getPcmFileBuffer(songInfo);
                        if(this.wavBuffer.byteLength !== 0) {
                            // Pipeline cascading
                            if (this.param.pipelineCascadMode !== 0 && this.param.pipelineCascadMode !== 3) {
                                if (!this.param.pipelineId || this.param.pipelineId === '') {
                                    this.getUIContext().showAlertDialog({
                                        title: '提示',
                                        message: 'pipelineId是空'
                                    });
                                    return;
                                }
                                if (audioNapi.multiPipelineEnvPrepare(this.param.pipelineId) !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        title: '提示',
                                        message: '创建管线失败'
                                    });
                                    return;
                                }
                            }
                            this.isAudioCache = true;
                            this.importAudioProgress.open();
                            let nodeIds: string[] = [nodeId, this.outputNode.id, this.mixerNode.id];
                            audioNapi.audioInAndOutInit(nodeIds, this.fd, this.wavBuffer.byteLength, 0);
                            Logger.error(TAG, `fd: ${this.fd}`);
                            setTimeout(() => {
                                if (songInfo) {
                                    songInfo.sampleRate = this.sampleRate;
                                    songInfo.channels = this.channels;
                                    songInfo.bitsPerSample = this.bitsPerSample;
                                }
                            }, 100);
                        }
                        this.nodeList.set(nodeId, songInfo)
                        const index = this.inputList.findIndex(song => song?.nodeId === nodeId);
                        if(index !== -1 && songInfo.uri !== undefined){
                            this.inputList[index] = songInfo
                        }
                    }
                });
                Logger.error(TAG,`nodeList size is ${this.nodeList.size}`)
            }
        })
        .gesture(
            LongPressGesture()
                .onAction((event: GestureEvent) => {
                    this.isLeft = isLeft;
                    this.selectedNode = nodeId;
                    this.nodeType = type;
                    this.checkNodeDialogController?.open();
                })
        )
    }

    @Builder
    Node(isLeft: boolean, info?:SongInfo | null, type?:NodeType) {
        if ( info && info.songName && type === NodeType.INPUT){
            Column(){
                Text(info.songName)
                    .height('60%')
                    .width("15%")
                    .margin({right: $r('app.float.margin_5')})
                    .textAlign(TextAlign.Center)
                    .backgroundColor(ColorMap.get(type))
                    .fontSize($r('app.float.font_size_10'))
                    .maxLines(1)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .borderRadius(5)
                    .maxLines(isLeft ? 1 : 2)
                    .textOverflow({ overflow: isLeft ? TextOverflow.Ellipsis : TextOverflow.None })
            }
        } else if (type) {
            Text(type)
                .height(isLeft ? '60%' : '20%')
                .width("10%")
                .margin({right: $r('app.float.margin_5')})
                .textAlign(TextAlign.Center)
                .backgroundColor(ColorMap.get(type))
                .borderRadius(5)
                .maxLines(isLeft ? 1 : 2)
                .textOverflow({ overflow: isLeft ? TextOverflow.Ellipsis : TextOverflow.None })
        }   else{
            Text("+")
                .height(isLeft ? '80%' : '20%')
                .width("10%")
                .margin({right: $r('app.float.margin_5')})
                .textAlign(TextAlign.Center)
                .fontColor(Color.White)
        }
    }

    getPcmFileBuffer(info: SongInfo): boolean{
        try {
            let path: string = new fileUri.FileUri(info.uri).path;
            Logger.info(TAG, `getPcmFileBuffer path: ${JSON.stringify(path)}`);
            let file  = fs.openSync(path, fs.OpenMode.READ_ONLY | fs.OpenMode.CREATE);
            let fsStat  = fs.statSync(path);
            Logger.info(TAG, `failSize : ${fsStat.size}`);
            // Don't delete --- let buffer = audioNapi.getFileBuffer(file.fd, fsStat.size);
            let buffer = new ArrayBuffer(fsStat.size);
            let readOption: ReadOptions = {
                offset: 0, // The expected position to read the file from. Optional, defaults to reading from the current position
                length: fsStat.size // The length of data expected to be read each time. Optional, default buffer length
            }
            fs.readSync(file.fd, buffer, readOption);
            Logger.info(TAG, `getPcmFileBuffer buffer length: ${buffer.byteLength}`);
            const wavView = new Uint8Array(buffer);
            let dataBuffer = wavView.slice(44).buffer;
            Logger.info(TAG, `wavBuffer length: ${buffer.byteLength}`);
            this.wavBuffer = buffer;
            this.fd = file.fd;
            return true;
        } catch (e) {
            Logger.error(TAG, `getPcmFileBuffer error: ${JSON.stringify(e)}`);
            return false;
        }
    }

    async SaveBuffer(value: ArrayBuffer) {
        try {
            let pcmBuffer = value;
            Logger.info(TAG, `SaveBuffer pcmBuffer length : ${pcmBuffer.byteLength}`);
            if(pcmBuffer.byteLength === 0) {
                Logger.info(TAG, 'SaveBuffer error');
            }
            // Picker selector, save PCM file
            let documentSaveOptions = new picker.DocumentSaveOptions();
            if(this.newAudioFileName ===  ''){
                this.newAudioFileName = getDateStringWithTimeStamp(new Date().getTime());
            } else {
                this.newAudioFileName = this.newAudioFileName + ' ' + getDateStringWithTimeStamp(new Date().getTime());   
            }
            let newFileNames = `${this.newAudioFileName}${this.audioFormate}`;
            this.newAudioFileName = '';
            Logger.info(TAG, `SaveBuffer newFileNames is: ${newFileNames}`);
            documentSaveOptions.newFileNames = [newFileNames];
            let context = getContext() as common.Context; // Please ensure that getContext(this) returns a result of UIAbilityContext
            let documentPicker = new picker.DocumentViewPicker(context);
            documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
                if(err){
                    Logger.error(TAG,
                        `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
                    return;
                }
                Logger.info(TAG, 
                    'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
                let filePath = documentSaveResult;
                // There is only one path in the cdocumentSaveResult array
                let file = fs.openSync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
                Logger.info(TAG, `SaveBuffer filePath : ${filePath}`);
                if(this.audioFormate === '.wav') {
                    // Processing WAV
                    let wavBuffer = this.writeWavFileHeader(pcmBuffer);
                    let writeLen = fs.writeSync(file.fd, wavBuffer);
                    Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
                    fs.closeSync(file);
                }
            }) 
        } catch(e) {
            Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
        }
    }

    writeString(dv: DataView, offset: number, str: string) {
        for (let i = 0; i < str.length; i++){
            dv.setUint8(offset+i, str.charCodeAt(i));
        }
    }

    concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
        const viewWavHeader = new Uint8Array(wavHeaderBuffer);
        const viewPcm = new Uint8Array(pcmBuffer);

        // Create a new ArrayBuffer with a size equal to the sum of the two original buffers
        const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
        const resultView = new Uint8Array(resultBuffer);

        // Copy the data from two views into a new view
        resultView.set(viewWavHeader);
        resultView.set(viewPcm, viewWavHeader.length);

        return resultBuffer;
    }

    // Define the method for writing WAV header file information
    writeWavFileHeader(pcmBuffer: ArrayBuffer): ArrayBuffer {
        Logger.info(TAG,
            `writeWavFileHeader sampleRate: ${this.sampleRate} --- channels: ${this.channels} --- bitsPerSample: ${this.bitsPerSample}`);
        const header = new ArrayBuffer(44);
        const dv = new DataView(header);
        // Write to RIFF block
        this.writeString(dv, 0, 'RIFF');
        let totalDataLen = pcmBuffer.byteLength + 44;
        dv.setUint32(4, totalDataLen, true);
        this.writeString(dv, 8, 'WAVE');
        // Write to fmt block
        this.writeString(dv, 12, 'fmt ');
        // fmt block size
        dv.setUint32(16, 16, true);
        // Format Category
        if (this.bitsPerSample === 32 && this.bitsPerSampleMode === BitsPerSampleMode.FLOAT) {
            dv.setUint16(20, 3, true);
        } else {
            dv.setUint16(20, 1, true);
        }
        // Number of audio channels
        dv.setUint16(22, this.channels, true);
        // sampleRate
        dv.setUint32(24, this.sampleRate, true);
        // byteRate
        let byteRate = this.bitsPerSample * this.sampleRate * this.channels / 8;
        dv.setUint32(28, byteRate, true);
        // Number of bytes per sampling point
        dv.setUint16(32, this.channels * this.bitsPerSample / 8, true);
        // Bit depth
        dv.setUint16(34, this.bitsPerSample, true);
        // writing data block
        this.writeString(dv, 36, 'data');
        // Database size 
        dv.setUint32(40, pcmBuffer.byteLength, true);

        let resultBuffer = this.concatArrayBuffer(header, pcmBuffer);
        return resultBuffer;
    }
}