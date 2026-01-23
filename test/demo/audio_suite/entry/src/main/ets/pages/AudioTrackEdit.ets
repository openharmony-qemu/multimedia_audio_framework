/*
* Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
*/
import { StyleConst } from "../utils/StyleConst";
import {
  NodeType,
  SongInfo,
  AudioAsset,
  AudioTrack,
  BitsPerSampleMode,
  RecordParamDefault,
  AudioWaveColorMap,
  SpaceRender,
  SpaceRenderMode,
  ClockWise,
  VoiceChange,
  GeneralVoiceChangeType,
  PureVoiceChangeGenderOption,
  PureVoiceChangeType,
  PureVoiceChangeVocalPartOption,
  ColorMap,
  Node,
  NumberToNodeTypeMap,
  SoundFiledType,
  EnvironmentType,
  VoiceBeautifierType,
  AudioSeparationType
} from "../utils/InterfaceInfo";
import { Logger } from '../utils/Logger';
import { MainTitleTextModifier } from '../utils/ExportFile';
import { common } from "@kit.AbilityKit";
import { AlertDialog, display, LoadingDialog, SymbolGlyphModifier, window } from '@kit.ArkUI';
import { TimeLine } from "./audioWave/TimeLine";
import { assembleAudioAsset, importSingleSong, importSongs } from '../utils/importSongs/ImportSongs';
import { util } from "@kit.ArkTS";
import { fileUri, picker } from '@kit.CoreFileKit';
import { taskpool } from '@kit.ArkTS';
import fs, { ReadOptions } from '@ohos.file.fs';
import { AudioEditWave } from "./audioWave/AudioEditWave";
import { ScaleMode } from "./audioWave/TimeLineUtils";
import { CustomDialogAudioRecord } from './customDialog/AudioRecord';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import { getDateStringWithTimeStamp, getTimesBySecond, getDate, sortAudioAssetsByStartTime } from '../utils/Util';
import audioNapi from 'libentry.so';
import { BusinessError } from '@kit.BasicServicesKit';
import { SetFormatDialog } from './customDialog/SetFormatDialog';
import { saveFileBuffer } from '../utils/ExportFile';
import { SpaceRenderDialog } from './customDialog/SpaceRenderDialog'
import { EqualizerDialog } from './customDialog/EqualizerDialog'
import { VoiceChangeDialog } from './customDialog/VoiceChangeDialog'
import { EffectDialog } from './customDialog/EffectDialog'


const TAG: string = 'AudioEditTestApp_AudioTrackEdit';
const INTERVAL_TIME = 1000;

let atManager = abilityAccessCtrl.createAtManager();
let context = getContext(this) as common.UIAbilityContext;

@Builder
export function AudioTrackEditBuilder() {
  AudioTrackEdit()
}

@Component
export struct AudioTrackEdit {
  pageInfos: NavPathStack = new NavPathStack();
  // Main title style modifier
  @State mainTitleModifier: MainTitleTextModifier = new MainTitleTextModifier();
  private symbolModifier: SymbolGlyphModifier =
    new SymbolGlyphModifier($r('sys.symbol.chevron_backward')).fontColor([$r('app.color.index_title_text_color')]);
  @State audioTrackList: AudioTrack[] = [];
  @State notRenderTrackList: string[] = [];
  @State renderTrackList: string[] = [];
  private MAX_TRACKS: number = 5; // Maximum number of audio tracks
  // Record or not
  @State g_isRecord: boolean = false;
  // Play or not
  @State isPlay: boolean = false;
  @State isPlayRealTime: boolean = false;
  @State showOnlyMusic: boolean = false; // Hide extra columns
  @State leftWidthPercent: number = 40; // Initial left width percentage
  private dragStartX: number = 0; // coordinate at the start of the drag.
  private dragStartWidth: number = 40; // Width at the start of dragging
  private minWidth: number = 10; // Minimum Width Percentage
  private maxWidth: number = 40; // Maximum Width Percentage
  private screenWidth: number = 1000; // Screen width
  private scrollerIndex: Scroller = new Scroller();
  @Provide currentFirstTick: number = 0;
  @Provide intervalWidth: number = 2;
  @Provide audioWavePositionX: number = 0;
  @Provide divisorMode: ScaleMode = ScaleMode.MODE_TWENTY_MILLISECONDS;
  @State realTimeLineWidth: number = 0;
  @State realTimeLineHeight: number = 0;
  @State realAudioWaveWidth: number = 0;
  @State realAudioWaveHeight: number = 0;
  @State isShowAudioWave: boolean[] = [false, false, false, false, false];
  @State importAudioWaves: string[][] = [];
  private isSelectIndex: number = -1;
  private wavBuffer: ArrayBuffer = new ArrayBuffer(0);
  private pcmBuffer: ArrayBuffer = new ArrayBuffer(0);
  private fd: number = -1;
  //Recording parameters
  @State recordState: string = 'init'; // [init,started,continued,paused,stoped]
  @State recordSec: number = 0;
  @State showTime: string = '00:00:00';
  @State isRecordOver: boolean = false;
  @State date: string = '';
  @State interval: number = 0;
  @State context: common.UIAbilityContext | null = null;
  @State recordInputId: string = "";
  @State recordInputIdSet: Set<string> = new Set();
  @State recordOutPutId: string = "";
  @State recordMixerId: string = "";
  @State realPlayRecord: string = "init";
  //playing parameters
  // 实时播放（音源分离）
  @Provide isSeparation: boolean = false;
  @State voiceType: string = NodeType.SEPARATION;
  // equalizer parameters for dialog
  @State balanceBandGains: number[] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
  // spaceRender parameters for dialog
  @State spaceRenderPara: SpaceRender = {
    mode: SpaceRenderMode.CLOSE,
    fixedPosition: { x: 0, y: 0, z: 0 },
    dynamicPosition: { x: 0, y: 0, z: 0 },
    singleWeekCycleTime: 2,
    clockWise: ClockWise.CLOCK_WISE,
    expansionRadius: 1,
    expansionAngle: 1
  };
  // voiceChange parameters for dialog
  @State voiceChangePara: VoiceChange = {
    type: NodeType.GENERAL_VOICE_CHANGE,
    generalVoiceChange: GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE,
    pureVoiceChangeGender: PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE,
    pureVoiceChangeVocalPart: PureVoiceChangeVocalPartOption.PURE_VOICE_CHANGE_VOCAL_PART_CLOSE,
    pureVoiceChangeType: PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE,
    pureVoiceSoundTone: 0
  }
  @State hasEffectNodes: NodeType[] = [];
  @State soundFiledPara: SoundFiledType = SoundFiledType.SOUND_FIELD_CLOSE;
  @State environmentPara: EnvironmentType = EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
  @State voiceBeautifierPara: VoiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL;
  @State audioSeparationPara: AudioSeparationType = AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE;
  @State soundSpeedPara: number = 1.0;
  @State soundTonePara: number = 1.0;
  //import audio param
  private importAudioAsset: AudioAsset = {
    assetId: 0,
    uri: '',
    channels: 0,
    sampleRate: 0,
    bitsPerSample: 0,
    startTime: 0,
    nodes: []
  };
  // output file params
  @State newAudioFileName: string = '';
  @State audioFormat: string = '.wav';
  @State sampleRate: number = RecordParamDefault.sampleRate;
  @State channels: number = RecordParamDefault.channels;
  @State bitsPerSample: number = RecordParamDefault.bitsPerSample;
  @State bitsPerSampleMode: number = BitsPerSampleMode.FLOAT;
  @State _currentTime: number = 0;
  // 添加音频后，音频是否还在缓存中
  @State isAudioCache: boolean = false;
  // current selected audioTrackID(currentInputId)
  @State selectedAudioTrackId: string = '';
  // current selected audioAsset startTime
  @State selectedAudioAssetStartTime: number = -1;
  // force render effect node
  @State forceRenderEffectNodeState: boolean = false;
  // effect node list
  @State nodeTypes: NodeType[] = [];
  // selected effectNode ID
  @State selectedEffectNodeId: string = '';
  // force render music icon
  @State forceRenderMusicIconState: boolean = false;
  // force render SilentAndSolo
  @State forceRenderSilentAndSoloState: boolean = false;

  aboutToAppear(): void {
    Logger.info(TAG, 'aboutToAppear');

    let displayClass: display.Display | null = null;
    try {
      displayClass = display.getDefaultDisplaySync();
      this.screenWidth = displayClass.width;
    } catch (exception) {
      Logger.error(TAG, 'get screenWidth fail')
    }
    this.isPlayRealTime = false;
    this.recordState = 'init';
    this.date = getDate(1);
    let context = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
    window.getLastWindow(context).then((windowClass) => {
      // Set screen to landscape mode
      windowClass.setPreferredOrientation(window.Orientation.AUTO_ROTATION_LANDSCAPE);
    });

    //初始化音轨、音频数据
    audioNapi.clearTimeline();

    audioNapi.registerStringCallback((result: string) => {
      // Logger.info(TAG, `registerStringCallback result: ${result}`);
      this.importAudioWaves[this.isSelectIndex] = result.split(' ');
      Logger.info(TAG,
        `registerStringCallback this.importAudioWaves size: ${this.importAudioWaves[this.isSelectIndex].length}, isSelectIndex: ${this.isSelectIndex}`);
    })

    audioNapi.registerAudioFormatCallback((result: string[]) => {
      Logger.info(TAG, `registerAudioFormatCallback string size: ${JSON.stringify(result.length)}, result: ${result}`);
      this.importAudioAsset.sampleRate = Number(result[0]);
      this.importAudioAsset.channels = Number(result[1]);
      this.sampleRate = Number(result[0]);
      this.channels = Number(result[1]);
      Logger.info(TAG,
        `registerAudioFormatCallback this.importAudioAsset.sampleRate:  ${this.importAudioAsset.sampleRate}`);
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
      this.importAudioAsset.bitsPerSample = this.bitsPerSample;
      if (this.recordState === 'started' || this.recordState === 'continued') {
        Logger.warn(TAG, `this.recordState is : return`);
        return;
      }
      const newAudioAsset: AudioAsset = {
        assetId: this.importAudioAsset.assetId,
        uri: this.importAudioAsset.uri,
        songName: this.importAudioAsset.songName,
        songType: this.importAudioAsset.songType,
        nodes: this.importAudioAsset.nodes,
        singerName: this.importAudioAsset.singerName,
        albumCover: this.importAudioAsset.albumCover,
        albumName: this.importAudioAsset.albumName,
        wavBuffer: this.importAudioAsset.wavBuffer,
        channels: this.importAudioAsset.channels,
        sampleRate: this.importAudioAsset.sampleRate,
        bitsPerSample: this.importAudioAsset.bitsPerSample,
        fd: this.importAudioAsset.fd,
        startTime: this.importAudioAsset.startTime,
        duration: this.importAudioAsset.duration
      };

      if (this.audioTrackList && this.isSelectIndex !== undefined) {
        if (this.audioTrackList[this.isSelectIndex].audioAssetArray !== undefined) {
          let existingAssetIndex =
            this.audioTrackList[this.isSelectIndex].audioAssetArray!.findIndex(asset => asset.assetId ===
            newAudioAsset.assetId);
          if (existingAssetIndex !== -1) {
            this.audioTrackList[this.isSelectIndex].audioAssetArray![existingAssetIndex] = newAudioAsset;
          } else {
            this.audioTrackList[this.isSelectIndex].audioAssetArray!.push(newAudioAsset);
          }
          existingAssetIndex =
            this.audioTrackList[this.isSelectIndex].audioAssetArray!.findIndex(asset => asset.assetId ===
            newAudioAsset.assetId);
          Logger.error(TAG,
            `existingAssetIndex: ${JSON.stringify(this.audioTrackList[this.isSelectIndex].audioAssetArray![existingAssetIndex])}`);
          sortAudioAssetsByStartTime([this.audioTrackList[this.isSelectIndex]]); // 自动排序
        }
      }
      //在对应音轨添加音频
      let newAssetPcmBufferLength: number = 0;
      if (newAudioAsset.duration != undefined) {
        newAssetPcmBufferLength =
          newAudioAsset.duration * newAudioAsset.sampleRate * (newAudioAsset.bitsPerSample / 8) *
          newAudioAsset.channels;
      }
      this.InitAddAllEffectNode(this.isSelectIndex, newAudioAsset.startTime,
        this.audioTrackList[this.isSelectIndex].audioTrackId);
      //reset audioasset
      this.importAudioAsset = {
        assetId: 0,
        uri: '',
        channels: 0,
        sampleRate: 0,
        bitsPerSample: 0,
        nodes: []
      };
      this.forceRenderMusicIconState = !this.forceRenderMusicIconState;
    })

    // 注册添加音频后，缓存是否完成回调
    audioNapi.registerAudioCacheCallback((result: boolean) => {
      Logger.info(TAG, `registerAudioCacheCallback result: ${result}`);
      if (!result) {
        this.importAudioProgress.close();
        this.isAudioCache = result;
        fs.closeSync(this.fd);
      }
    });

    // Initialize an empty list (reserve 5 empty slots)
    this.audioTrackList = [
      {
        audioTrackId: '1',
        isSilent: false,
        isSolo: false,
        audioAssetArray: []
      }, {
      audioTrackId: '2',
      isSilent: false,
      isSolo: false,
      audioAssetArray: []
    }, {
      audioTrackId: '3',
      isSilent: false,
      isSolo: false,
      audioAssetArray: []
    }, {
      audioTrackId: '4',
      isSilent: false,
      isSolo: false,
      audioAssetArray: []
    }, {
      audioTrackId: '5',
      isSilent: false,
      isSolo: false,
      audioAssetArray: []
    }
    ]

    // get effect node list
    let nodeType: number[] = audioNapi.getEffectNodeList();
    this.nodeTypes = nodeType
      .map(num => NumberToNodeTypeMap.get(num)) // Convert numbers to NodeType
      .filter(type => {
        if (type === undefined || type === NodeType.EQ || type === NodeType.SPACE_RENDER ||
          type === NodeType.GENERAL_VOICE_CHANGE || type === NodeType.PURE_VOICE_CHANGE) {
          return false;
        } else {
          return true;
        }
      }) as NodeType[];

    audioNapi.registerFinishedCallback((result: boolean) => {
      Logger.info(TAG, `registerFinishedCallback finished: ${result}`);
      if (result) {
        this.isPlay = false;
        // 暂停
        audioNapi.audioRendererStop();
        // 将buffer的map中的 totalWriteAudioDataSize 置为 0
        audioNapi.resetTotalWriteAudioDataSize();
      }
    })
  }

  aboutToDisappear(): void {
    Logger.info(TAG, 'aboutToDisappear');
    this.isPlay = false;
    // 暂停
    audioNapi.audioRendererStop();

    audioNapi.clear();
    // 销毁管线， 销毁引擎
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
  }

  importAudioProgress: CustomDialogController = new CustomDialogController({
    builder: LoadingDialog({
      content: '添加音频中...',
    }),
    autoCancel: false,
  })
  // SpaceRender Dialog
  spaceRenderDialogController: CustomDialogController | null = new CustomDialogController({
    builder: SpaceRenderDialog({
      spaceRenderPara: this.spaceRenderPara,
      onAgree: (spaceRenderState: SpaceRender) => {
        // 调用napi的底层效果节点
        if (spaceRenderState.mode === -1) {
          this.getUIContext().showAlertDialog({
            message: `请选择一种模式`,
            autoCancel: true
          });
          return;
        }
        if (spaceRenderState.mode === 0) {
          let result = audioNapi.ResetFixedPositionEffect(spaceRenderState.fixedPosition?.x,
            spaceRenderState.fixedPosition?.y, spaceRenderState.fixedPosition?.z, this.selectedEffectNodeId);
          if (result !== 0) {
            this.getUIContext().showAlertDialog({
              message: `设置空间渲染节点失败`,
              autoCancel: true
            });
            return;
          }
        }
        if (spaceRenderState.mode === 1) {
          let result = audioNapi.ResetDynamicRenderEffect(spaceRenderState.dynamicPosition?.x,
            spaceRenderState.dynamicPosition?.y, spaceRenderState.dynamicPosition?.z,
            spaceRenderState.singleWeekCycleTime, Number(spaceRenderState.clockWise), this.selectedEffectNodeId);
          if (result !== 0) {
            this.getUIContext().showAlertDialog({
              message: `设置空间渲染节点失败`,
              autoCancel: true
            });
            return;
          }
        }
        if (spaceRenderState.mode === 2) {
          let result =
            audioNapi.ResetExpandEffect(spaceRenderState.expansionRadius, spaceRenderState.expansionAngle,
              this.selectedEffectNodeId);
          if (result !== 0) {
            this.getUIContext().showAlertDialog({
              message: `设置空间渲染节点失败`,
              autoCancel: true
            });
            return;
          }
        }

        // Bypass control node via the addAudioAssetEffectNode interface
        if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.SPACE_RENDER)) {
          return;
        }

        this.spaceRenderPara = {
          mode: spaceRenderState.mode ?? SpaceRenderMode.CLOSE,
          fixedPosition: {
            x: spaceRenderState.fixedPosition?.x ?? 0,
            y: spaceRenderState.fixedPosition?.y ?? 0,
            z: spaceRenderState.fixedPosition?.z ?? 0
          },
          dynamicPosition: {
            x: spaceRenderState.dynamicPosition?.x ?? 0,
            y: spaceRenderState.dynamicPosition?.y ?? 0,
            z: spaceRenderState.dynamicPosition?.z ?? 0
          },
          singleWeekCycleTime: spaceRenderState.singleWeekCycleTime ?? 2,
          clockWise: spaceRenderState.clockWise ?? ClockWise.CLOCK_WISE,
          expansionRadius: spaceRenderState.expansionRadius ?? 1,
          expansionAngle: spaceRenderState.expansionAngle ?? 1
        };

        let node: Node = {
          id: this.selectedEffectNodeId,
          type: NodeType.SPACE_RENDER,
          isByPass: true,
          color: ColorMap.get(NodeType.SPACE_RENDER),
          spaceRender: this.spaceRenderPara
        }
        this.addEffectNodeToTrackList(node);
        this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
      }
    }),
  });
  // Equalizer Dialog
  equalizerDialogController: CustomDialogController | null = new CustomDialogController({
    builder: EqualizerDialog({
      balanceBandGains: this.balanceBandGains,
      onAgree: (balanceBandGains: number[]) => {
        // 调用napi的底层效果节点
        let result =
          audioNapi.setEqualizerFrequencyBandGains(this.balanceBandGains, this.selectedEffectNodeId,
            this.selectedAudioTrackId);
        if (result !== 0) {
          Logger.error(TAG, `setEqualizerFrequencyBandGains result: ${result}`);
          this.getUIContext().showAlertDialog({
            message: '修改均衡节点失败',
            autoCancel: true
          });
          return;
        }
        // Bypass control node via the addAudioAssetEffectNode interface
        if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.EQ)) {
          return;
        }

        this.balanceBandGains = [...balanceBandGains];

        let node: Node = {
          id: this.selectedEffectNodeId,
          type: NodeType.EQ,
          isByPass: true,
          color: ColorMap.get(NodeType.EQ),
          balanceBandGains: this.balanceBandGains
        }
        this.addEffectNodeToTrackList(node);
        this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
      }
    }),
  });
  // VoiceChange Dialog
  voiceChangeDialogController: CustomDialogController | null = new CustomDialogController({
    builder: VoiceChangeDialog({
      voiceChangePara: this.voiceChangePara,
      onAgree: (voiceChangePara: VoiceChange) => {
        // 调用napi的底层效果节点
        let audioTrack = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId === this.selectedAudioTrackId);
        let audioAsset =
          audioTrack?.audioAssetArray?.find(audioAsset => audioAsset.startTime === this.selectedAudioAssetStartTime);
        if (voiceChangePara.type === NodeType.GENERAL_VOICE_CHANGE) {
          let node = audioAsset?.nodes?.find(node => node.type === NodeType.GENERAL_VOICE_CHANGE);
          if (voiceChangePara.generalVoiceChange === GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE) {
            let res = audioNapi.setEffectNodeBypass(this.selectedAudioTrackId, node?.id, true);
            if (!res) {
              this.getUIContext().showAlertDialog({
                message: `设置节点Bypass失败`,
                autoCancel: true
              });
              return;
            }
            this.voiceChangePara = {
              type: voiceChangePara.type,
              generalVoiceChange: voiceChangePara.generalVoiceChange,
              pureVoiceChangeGender: voiceChangePara.pureVoiceChangeGender,
              pureVoiceChangeVocalPart: voiceChangePara.pureVoiceChangeVocalPart,
              pureVoiceChangeType: voiceChangePara.pureVoiceChangeType,
              pureVoiceSoundTone: voiceChangePara.pureVoiceSoundTone
            }

            let newNode: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.VOICE_CHANGE,
              isByPass: false,
              color: ColorMap.get(NodeType.VOICE_CHANGE),
              voiceChange: this.voiceChangePara
            }
            this.addEffectNodeToTrackList(newNode);
            this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
            return;
          }
          let result = audioNapi.resetGeneralVoiceChange(this.selectedAudioTrackId, voiceChangePara.generalVoiceChange,
            node?.id);
          if (result !== 0) {
            this.getUIContext().showAlertDialog({
              message: '设置通用变声节点失败',
              autoCancel: true
            });
            return;
          }
          // Bypass control node via the addAudioAssetEffectNode interface
          this.selectedEffectNodeId = node?.id ?? '';
          if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.GENERAL_VOICE_CHANGE)) {
            return;
          }
        } else {
          let node = audioAsset?.nodes?.find(node => node.type === NodeType.PURE_VOICE_CHANGE);
          //如果是无选项，则passby需要改为true
          if (voiceChangePara.pureVoiceChangeGender === PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE ||
            voiceChangePara.pureVoiceChangeType === PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE) {
            let res = audioNapi.setEffectNodeBypass(this.selectedAudioTrackId, node?.id, true);
            if (!res) {
              this.getUIContext().showAlertDialog({
                message: `设置节点Bypass失败`,
                autoCancel: true
              });
              return;
            }
            this.voiceChangePara = {
              type: voiceChangePara.type,
              generalVoiceChange: voiceChangePara.generalVoiceChange,
              pureVoiceChangeGender: voiceChangePara.pureVoiceChangeGender,
              pureVoiceChangeVocalPart: voiceChangePara.pureVoiceChangeVocalPart,
              pureVoiceChangeType: voiceChangePara.pureVoiceChangeType,
              pureVoiceSoundTone: voiceChangePara.pureVoiceSoundTone
            }

            let newNode: Node = {
              id: this.selectedEffectNodeId,
              type: NodeType.VOICE_CHANGE,
              isByPass: false,
              color: ColorMap.get(NodeType.VOICE_CHANGE),
              voiceChange: this.voiceChangePara
            }
            this.addEffectNodeToTrackList(newNode);
            this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
            return;
          } else {
            let result = audioNapi.resetPureVoiceChange(this.selectedAudioTrackId, node?.id,
              voiceChangePara.pureVoiceChangeGender, voiceChangePara.pureVoiceSoundTone,
              voiceChangePara.pureVoiceChangeType);
            if (result !== 0) {
              this.getUIContext().showAlertDialog({
                message: '设置传统变声节点失败',
                autoCancel: true
              });
              return;
            }
            // Bypass control node via the addAudioAssetEffectNode interface
            this.selectedEffectNodeId = node?.id ?? '';
            if (!this.isSuccessAddEffectToAudioAssetNapi(NodeType.PURE_VOICE_CHANGE)) {
              return;
            }
          }
        }

        this.voiceChangePara = {
          type: voiceChangePara.type,
          generalVoiceChange: voiceChangePara.generalVoiceChange,
          pureVoiceChangeGender: voiceChangePara.pureVoiceChangeGender,
          pureVoiceChangeVocalPart: voiceChangePara.pureVoiceChangeVocalPart,
          pureVoiceChangeType: voiceChangePara.pureVoiceChangeType,
          pureVoiceSoundTone: voiceChangePara.pureVoiceSoundTone
        }

        let node: Node = {
          id: this.selectedEffectNodeId,
          type: NodeType.VOICE_CHANGE,
          isByPass: true,
          color: ColorMap.get(NodeType.VOICE_CHANGE),
          voiceChange: this.voiceChangePara
        }
        this.addEffectNodeToTrackList(node);
        this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
      }
    }),
  });
  // EffectNode Dialog
  effectDialogController: CustomDialogController | null = new CustomDialogController({
    builder: EffectDialog({
      nodeTypes: this.nodeTypes,
      hasEffectNodes: this.hasEffectNodes,
      selectedSoundFiled: this.soundFiledPara,
      selectedEnvironment: this.environmentPara,
      selectedVoiceBeautifier: this.voiceBeautifierPara,
      selectedAudioSeparation: this.audioSeparationPara,
      selectedSoundSpeed: this.soundSpeedPara,
      selectedSoundTone: this.soundTonePara,
      audioTrackList: this.audioTrackList,
      selectedAudioTrackId: this.selectedAudioTrackId,
      selectedAudioAssetStartTime: this.selectedAudioAssetStartTime,
      selectedEffectNodeId: this.selectedEffectNodeId,
      onAgree: (node: Node) => {
        // 调用napi的底层效果节点

        this.hasEffectNodes.push(node.type);
        this.addEffectNodeToTrackList(node);
        this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
      }
    }),
  });

  addEffectNodeToTrackList(node: Node) {
    let audioTrack = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId === this.selectedAudioTrackId);
    let audioAsset =
      audioTrack?.audioAssetArray?.find(audioAsset => audioAsset.startTime === this.selectedAudioAssetStartTime);
    let index = audioAsset?.nodes?.findIndex(nodeItem => nodeItem.type === node.type) ?? -1;
    if (audioAsset?.nodes) {
      if (index >= 0) {
        audioAsset.nodes[index] = node;
      } else {
        audioAsset.nodes.push(node);
      }
    }
  }

  isSuccessAddEffectToAudioAssetNapi(type: NodeType) {
    Logger.info(TAG, `enter isSuccessAddEffectToAudioAssetNapi, nodeType:${type}`);
    if (this.isNodeTypeInAudioAsset(type)) {
      let addAssetResult =
        audioNapi.addAudioAssetEffectNode(this.selectedAudioTrackId, this.selectedAudioAssetStartTime,
          this.selectedEffectNodeId);
      if (!addAssetResult) {
        this.getUIContext().showAlertDialog({
          message: `设置节点失败`,
          autoCancel: true
        });
        return false;
      }
      Logger.info(TAG, `enter bypass set`);
      let ret = audioNapi.setEffectNodeBypass(this.selectedAudioTrackId, this.selectedEffectNodeId, false);
      if (!ret) {
        this.getUIContext().showAlertDialog({
          message: `设置节点Bypass失败`,
          autoCancel: true
        });
        return false;
      }
    }
    return true;
  }

  isNodeTypeInAudioAsset(type: NodeType) {
    let nodes = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId ===
    this.selectedAudioTrackId)?.audioAssetArray?.find(audioAsset => audioAsset.startTime ===
    this.selectedAudioAssetStartTime)?.nodes;
    return nodes?.find(node => node.type === type) ? true : false;
  }

  //start record
  capturerStart() {
    try {
      audioNapi.clearRecordBuffer();
      audioNapi.audioCapturerStart();
      this.showTime = '00:00:00';
      this.recordSec = 0;
      this.recordState = 'started';
      clearInterval(this.interval);
      this.interval = setInterval(async () => {
        this.recordSec++;
        this.showTime = getTimesBySecond(this.recordSec);
      }, INTERVAL_TIME);
    } catch (err) {
      let error = err as BusinessError;
      Logger.info(TAG, `AudioRecording:audioCapturer start err=${JSON.stringify(error)}`);
    }
  }

  audioRecordDialog: CustomDialogController | null = new CustomDialogController({
    builder: CustomDialogAudioRecord({
      recordSec: this.recordSec,
      recordState: this.recordState,
      showTime: this.showTime,
      interval: this.interval,
      isRecordOver: this.isRecordOver,
      audioTrackList: this.audioTrackList,
      sampleRate: this.sampleRate,
      channels: this.channels,
      bitsPerSample: this.bitsPerSample,
      bitsPerSampleMode: this.bitsPerSampleMode,
      startTime: this._currentTime,
      isPlay: this.isPlay,
      inputId: this.recordInputId,
      mixerId: this.recordMixerId,
      outPutId: this.recordOutPutId,
      realPlayRecord: this.realPlayRecord
    }),
    autoCancel: false
  })

  //init record
  @Builder
  InitRecord() {
    Image($r('app.media.ic_record_circle'))
      .width('30')
      .height('30')
      .margin({ right: $r('app.float.margin_5') })
      .onClick(() => {
        atManager.requestPermissionsFromUser(context, ['ohos.permission.MICROPHONE']).then((data) => {
          Logger.info(TAG, 'data:' + JSON.stringify(data));
          this.recordState = 'started';
          if (data.authResults[0] !== 0) {
            return;
          }
          this.recordInputId = util.generateRandomUUID(true);
          this.recordMixerId = util.generateRandomUUID(true);
          this.recordOutPutId = util.generateRandomUUID(true);
          let isPure = !this.audioTrackList.some(track => track.audioAssetArray?.length !== 0);
          audioNapi.audioCapturerInit(this.isPlay, this.recordInputId, this.recordMixerId, this.recordOutPutId, 0,
            isPure);
          if (this.isPlay) {
            this.recordInputIdSet.add(this.recordInputId);
            audioNapi.mixPlayInitBuffer(this.recordInputId, this.recordMixerId, this.recordOutPutId, 0);
          }
          // audioNapi.audioCapturerInit(this.isPlay, this.recordInputId, this.recordOutPutId, this.recordMixerId, this._currentTime);
          this.capturerStart();
          this.audioRecordDialog?.open();
        }).catch((err: BusinessError) => {
          this.recordState = 'init';
          Logger.error(TAG, 'data:' + JSON.stringify(err));
        });
      })
  }

  //finish record
  @Builder
  FinishedRecord() {
    Column() {
      Image($r('app.media.ic_record_circle')).width(56).height(56)
    }
    .width('100%')
    .height(56)
    .position({ y: 60 })
    .opacity(0.4)
    .id('disalbe_btn')
  }

  handleDrag(event: TouchEvent): void {
    const screenWidth = this.screenWidth;
    const sensitivity = 0.5; // Sensitivity coefficient

    switch (event.type) {
      case TouchType.Down:
        this.dragStartX = event.touches[0].x;
        this.dragStartWidth = this.leftWidthPercent;
        break;

      case TouchType.Move:
        if (this.dragStartX === 0) {
          return;
        }

        const deltaX = this.dragStartX - event.touches[0].x;
        const deltaPercent = (deltaX / screenWidth) * 100 * sensitivity;

        let newWidth = this.dragStartWidth - deltaPercent;

        // Restrict between minimum and maximum widths
        if (newWidth < this.minWidth) {
          newWidth = this.minWidth;
        }
        if (newWidth > this.maxWidth) {
          newWidth = this.maxWidth;
        }

        // Smooth transition
        animateToImmediately({
          duration: 20,
          curve: Curve.Linear
        }, () => {
          this.leftWidthPercent = newWidth;
        });
        break;

      case TouchType.Up:
      case TouchType.Cancel:
        this.dragStartX = 0;

        // Drag left to set to the minimum value, drag right to set to the maximum value.
        if (this.leftWidthPercent < this.maxWidth && this.dragStartWidth === this.maxWidth) {
          animateToImmediately({
            duration: 150,
            curve: Curve.EaseOut
          }, () => {
            this.leftWidthPercent = this.minWidth;
            this.showOnlyMusic = true;
          });
        } else if (this.leftWidthPercent > this.minWidth && this.dragStartWidth === this.minWidth) {
          animateToImmediately({
            duration: 150,
            curve: Curve.EaseOut
          }, () => {
            this.leftWidthPercent = this.maxWidth;
            this.showOnlyMusic = false;
          });
        }
        break;
    }
  }

  handleSilent(audioTrack: AudioTrack, index: number) {
    let isSilentIndex = this.audioTrackList.findIndex(item => item.isSolo);
    if (audioTrack.isSilent) {
      if (isSilentIndex === -1) {
        this.audioTrackList[index].isSilent = !this.audioTrackList[index].isSilent;
      } else if (isSilentIndex === index) {
        this.audioTrackList[index].isSilent = !this.audioTrackList[index].isSilent;
        this.audioTrackList[index].isSolo = !this.audioTrackList[index].isSolo;
      }
    } else {
      if (audioTrack.isSolo) {
        this.audioTrackList[index].isSilent = !this.audioTrackList[index].isSilent;
        this.audioTrackList[index].isSolo = !this.audioTrackList[index].isSolo;
        for (let i = 0; i < this.audioTrackList.length; i++) {
          if (i !== index) {
            this.audioTrackList[i].isSilent = false;
          }
        }
      } else {
        this.audioTrackList[index].isSilent = !this.audioTrackList[index].isSilent;
      }
    }
    this.updateTrackRenderLists(this.notRenderTrackList, this.renderTrackList, this.audioTrackList);
    this.forceRenderSilentAndSoloState = !this.forceRenderSilentAndSoloState;
  }

  handleSolo(audioTrack: AudioTrack, index: number) {
    if (audioTrack.isSolo) {
      this.audioTrackList.forEach((item, idx) => {
        this.audioTrackList[idx].isSilent = false;
        this.audioTrackList[idx].isSolo = false;
      })
    } else {
      this.audioTrackList.forEach((item, idx) => {
        if (idx === index) {
          this.audioTrackList[idx].isSilent = false;
          this.audioTrackList[idx].isSolo = true;
        } else {
          this.audioTrackList[idx].isSilent = true;
          this.audioTrackList[idx].isSolo = false;
        }
      })
    }
    this.updateTrackRenderLists(this.notRenderTrackList, this.renderTrackList, this.audioTrackList);
    this.forceRenderSilentAndSoloState = !this.forceRenderSilentAndSoloState;
  }

  handleEffectNode(type: NodeType, index: number) {
    if (this.selectedAudioAssetStartTime !== -1 &&
      this.audioTrackList[index].audioTrackId === this.selectedAudioTrackId) {
      let audioTrack = this.audioTrackList.find(audioTrack => audioTrack.audioTrackId === this.selectedAudioTrackId);
      let audioAsset =
        audioTrack?.audioAssetArray?.find(audioAsset => audioAsset.startTime === this.selectedAudioAssetStartTime);
      if (type === NodeType.SPACE_RENDER) {
        let node = audioAsset?.nodes?.find(node => node.type === NodeType.SPACE_RENDER);
        this.selectedEffectNodeId = node?.id ?? '';
        this.spaceRenderPara = node?.spaceRender ?? this.spaceRenderPara;
        this.spaceRenderDialogController?.open();
      } else if (type === NodeType.EQ) {
        let node = audioAsset?.nodes?.find(node => node.type === NodeType.EQ);
        this.selectedEffectNodeId = node?.id ?? '';
        this.balanceBandGains = node?.balanceBandGains ?? this.balanceBandGains;
        this.equalizerDialogController?.open();
      } else if (type === NodeType.VOICE_CHANGE) {
        let node = audioAsset?.nodes?.find(node => node.type === NodeType.VOICE_CHANGE);
        this.voiceChangePara = node?.voiceChange ?? this.voiceChangePara;
        this.voiceChangeDialogController?.open();
      } else if (type === NodeType.EFFECT) {
        this.hasEffectNodes = [];
        audioAsset?.nodes?.forEach(node => {
          if (this.nodeTypes.includes(node.type) && node.isByPass) {
            this.hasEffectNodes.push(node.type);
            switch (node.type) {
              case NodeType.FIELD:
                this.soundFiledPara = node.soundFiledType ?? SoundFiledType.SOUND_FIELD_CLOSE;
                break;
              case NodeType.ENV:
                this.environmentPara = node.environmentType ?? EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
                break;
              case NodeType.VB:
                this.voiceBeautifierPara = node.voiceBeautifierType ?? VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR;
                break;
              case NodeType.SEPARATION:
                this.audioSeparationPara =
                  node.audioSeparationType ?? AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE;
                break;
              case NodeType.SOUND_SPEED_TONE:
                this.soundSpeedPara = node.soundSpeed ?? 1.0;
                this.soundTonePara = node.soundTone ?? 1.0;
                break;
              default:
                break;
            }
          }
        })
        this.effectDialogController?.open();
      }
    } else {
      this.getUIContext().showAlertDialog({
        message: '请选择对应音轨音频',
        autoCancel: true
      });
    }
  }

  InitAddAllEffectNode(audioTrackSelectIndex: number, assetId: number | undefined, inputId: string) {
    // get audioTrack
    if (!this.audioTrackList || audioTrackSelectIndex === undefined) {
      this.getUIContext().showAlertDialog({
        title: '错误',
        message: `error get audio track`
      });
      return;
    }
    const audioTrack = this.audioTrackList[audioTrackSelectIndex];
    if (!audioTrack || !audioTrack.audioAssetArray) {
      this.getUIContext().showAlertDialog({
        title: '错误',
        message: `error audio track, index: ${audioTrackSelectIndex}`
      });
      return;
    }
    // get audioAsset
    let audioAsset: AudioAsset | undefined;
    if (assetId !== undefined) {
      audioAsset = audioTrack.audioAssetArray.find(asset => asset.assetId === assetId);
    }
    if (!audioAsset) {
      this.getUIContext().showAlertDialog({
        title: '错误',
        message: 'audio asset not exist'
      });
      return;
    }
    // make sure nodes elements exist
    if (!audioAsset.nodes) {
      audioAsset.nodes = [];
    }
    // 3D
    const fixedPositionEffectId: string = util.generateRandomUUID(true);
    let result = audioNapi.StartFixedPositionEffect(0, 0, 0, fixedPositionEffectId, inputId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startFixedPositionEffect: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: fixedPositionEffectId,
        type: NodeType['SPACE_RENDER'],
        spaceRender: { mode: SpaceRenderMode.FIXED_POSITION, fixedPosition: { x: 0, y: 0, z: 0 } }
      });
    }
    let ret = audioNapi.setEffectNodeBypass(inputId, fixedPositionEffectId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set fixedPositionEffect bypass`
      })
      return;
    }
    // const dynamicRenderEffectId: string = util.generateRandomUUID(true);
    // result = audioNapi.startDynamicRenderEffect(0, 0, 0, 2.0, 0, dynamicRenderEffectId, inputId);
    // if (result !== 0) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error startDynamicRenderEffect: ${result}`
    //   });
    // } else {
    //   audioAsset.nodes.push({
    //     id: dynamicRenderEffectId,
    //     type: NodeType['SPACE_RENDER'],
    //     spaceRender: { mode: SpaceRenderMode.DYNAMIC_RENDER, dynamicPosition: {x: 0, y: 0, z: 0},
    //       singleWeekCycleTime: 2.0, clockWise: ClockWise.CLOCK_WISE }
    //   });
    // }
    // ret = audioNapi.setEffectNodeBypass(inputId, dynamicRenderEffectId, true);
    // if (!ret) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error set dynamicRenderEffect bypass`
    //   })
    //   return;
    // }
    // const expandEffectId: string = util.generateRandomUUID(true);
    // result = audioNapi.startExpandEffect(1.0, 1, expandEffectId, inputId);
    // if (result !== 0) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error startExpandEffect: ${result}`
    //   });
    // } else {
    //   audioAsset.nodes.push({
    //     id: expandEffectId,
    //     type: NodeType['SPACE_RENDER'],
    //     spaceRender: { mode: SpaceRenderMode.EXPAND, expansionRadius: 1.0, expansionAngle: 1 }
    //   });
    // }
    // ret = audioNapi.setEffectNodeBypass(inputId, expandEffectId, true);
    // if (!ret) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error set expandEffect bypass`
    //   })
    //   return;
    // }
    // // equalizer
    const equalizerModeId: string = util.generateRandomUUID(true);
    Logger.warn(TAG, `equalizerModeId is ${equalizerModeId}`)
    result = audioNapi.setEqualizerMode(0, equalizerModeId, inputId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error setEqualizerMode: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: equalizerModeId,
        type: NodeType['EQ'],
        balanceBandGains: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, equalizerModeId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set equalizerMode bypass`
      })
      return;
    }
    // pure voice change
    const pureVoiceChangeId: string = util.generateRandomUUID(true);
    result = audioNapi.startPureVoiceChange(inputId, pureVoiceChangeId, 1, 0, 1);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startPureVoiceChange: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: pureVoiceChangeId,
        type: NodeType['PURE_VOICE_CHANGE'],
        voiceChange: {
          type: NodeType.PURE_VOICE_CHANGE,
          pureVoiceChangeGender: PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_FEMALE,
          pureVoiceChangeVocalPart: PureVoiceChangeVocalPartOption.PURE_VOICE_CHANGE_VOCAL_PART_CLOSE,
          pureVoiceChangeType: PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CARTOON
        }
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, pureVoiceChangeId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set pureVoiceChange bypass`
      })
      return;
    }
    // // general voice change
    const generalVoiceChangeId: string = util.generateRandomUUID(true);
    result = audioNapi.startGeneralVoiceChange(inputId, 1, generalVoiceChangeId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startGeneralVoiceChange: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: generalVoiceChangeId,
        type: NodeType['GENERAL_VOICE_CHANGE'],
        voiceChange: {
          type: NodeType.GENERAL_VOICE_CHANGE,
          generalVoiceChange: GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE
        }
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, generalVoiceChangeId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set generalVoiceChange bypass`
      })
      return;
    }
    // noise reduction
    const noiseReductionId: string = util.generateRandomUUID(true);
    result = audioNapi.addNoiseReduction(noiseReductionId, inputId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error addNoiseReduction: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: noiseReductionId,
        type: NodeType['NR']
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, noiseReductionId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set noiseReduction bypass`
      })
      return;
    }
    // sound field
    const soundFieldEffectId: string = util.generateRandomUUID(true);
    result = audioNapi.startFieldEffect(inputId, 0, soundFieldEffectId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startFieldEffect: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: soundFieldEffectId,
        type: NodeType['FIELD'],
        soundFiledType: SoundFiledType.SOUND_FIELD_CLOSE
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, soundFieldEffectId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set soundFieldEffect bypass`
      })
      return;
    }
    // env
    const envEffectId: string = util.generateRandomUUID(true);
    result = audioNapi.startEnvEffect(inputId, envEffectId, 0);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startEnvEffect: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: envEffectId,
        type: NodeType['ENV'],
        environmentType: EnvironmentType.ENVIRONMENT_TYPE_CLOSE
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, envEffectId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set envEffect bypass`
      })
      return;
    }
    //beauty
    const beautEffectId: string = util.generateRandomUUID(true);
    result = audioNapi.startVBEffect(inputId, 0, beautEffectId);
    if (result !== 0) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error startVBEffect: ${result}`
      });
    } else {
      audioAsset.nodes.push({
        id: beautEffectId,
        type: NodeType['VB'],
        voiceBeautifierType: VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL
      });
    }
    ret = audioNapi.setEffectNodeBypass(inputId, beautEffectId, true);
    if (!ret) {
      this.getUIContext().showAlertDialog({
        title: '提示',
        message: `error set beautEffect bypass`
      })
      return;
    }
    // aiss
    // const aissEffectId: string = util.generateRandomUUID(true);
    // result = audioNapi.addAudioSeparation(0, aissEffectId, inputId);
    // if (result !== 0) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error addAudioSeparation: ${result}`
    //   });
    // } else {
    //   audioAsset.nodes.push({
    //     id: aissEffectId,
    //     type: NodeType['SEPARATION'],
    //     audioSeparationType: AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE
    //   });
    // }
    // ret = audioNapi.setEffectNodeBypass(inputId, aissEffectId, true);
    // if (!ret) {
    //   this.getUIContext().showAlertDialog({
    //     title: '提示',
    //     message: `error set aissEffect bypass`
    //   })
    //   return;
    // }
  }

  build() {
    NavDestination() {
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween }) {
        Scroll(this.scrollerIndex) {
          // Left
          Row() {
            // Audio effect module
            Row() {
              Column() {
                Row()
                  .height($r('app.float.height_10'))
                  .width(StyleConst.FULL_WIDTH)
                ForEach(this.audioTrackList, (audioTrack: AudioTrack, index: number) => {
                  Row() {
                    // silent and solo UI
                    if (!this.showOnlyMusic) {
                      Column() {
                        if (this.forceRenderSilentAndSoloState) {
                          this.silentAndSoloBuilder(audioTrack, index)
                        } else {
                          this.silentAndSoloBuilder(audioTrack, index)
                        }
                      }
                      .justifyContent(FlexAlign.Center)
                      .alignItems(HorizontalAlign.Center)
                    }
                    // effect Node UI
                    if (!this.showOnlyMusic) {
                      Column() {
                        if (this.forceRenderEffectNodeState) {
                          this.EffectNode(audioTrack, index)
                        } else {
                          this.EffectNode(audioTrack, index)
                        }
                      }
                      .width(StyleConst.SIXTY_PERCENT_WIDTH)
                      .margin({ left: $r('app.float.margin_10') })
                    }
                    // music icons UI
                    if (this.forceRenderMusicIconState) {
                      this.MusicIcon(audioTrack, index)
                    } else {
                      this.MusicIcon(audioTrack, index)
                    }
                  }
                  .height(StyleConst.NINETEEN_PERCENT_HEIGHT)
                  .width(StyleConst.FULL_WIDTH)
                  .border({
                    width: { top: '1vp', right: index === 2 ? '2vp' : '0vp' },
                    color: { top: Color.Black, right: Color.Gray }
                  })
                })
              }
              .width(StyleConst.FULL_WIDTH)
            }
            .backgroundColor($r('app.color.mixing_console_backgroundColor'))
            .width(`${this.leftWidthPercent}%`)
            .padding({ left: $r('app.float.padding_10') })
            .onTouch((event: TouchEvent) => {
              this.handleDrag(event);
            })

            // Audio Wave Module
            Row() {
              Column() {
                // TimeLine
                Row() {
                  TimeLine({
                    viewWidth: this.realTimeLineWidth,
                    viewHeight: this.realTimeLineHeight,
                    isPlay: this.isPlay,
                    _currentTime: this._currentTime
                  })
                }
                .height(StyleConst.FIVE_PERCENT_HEIGHT)
                .width(StyleConst.FULL_WIDTH)
                .margin({ left: $r('app.float.margin_10') })
                .onAreaChange((oldArea, newArea) => {
                  Logger.info(TAG,
                    `onAreaChange realTimeLineWidth: ${newArea.width}, realTimeLineHeight: ${newArea.height}`)
                  this.realTimeLineWidth = newArea.width as number;
                  this.realTimeLineHeight = newArea.height as number;
                  this.realAudioWaveWidth = newArea.width as number;
                })

                ForEach(this.audioTrackList, (info: AudioTrack, index: number) => {
                  Row() {
                    AudioEditWave({
                      viewWidth: this.realAudioWaveWidth,
                      viewHeight: this.realAudioWaveHeight,
                      canvasHeight: 40,
                      isShowAudioWave: this.isShowAudioWave[index],
                      importAudioWaves: this.importAudioWaves[index],
                      lastCurrentFirstTick: -Math.ceil(this.realAudioWaveWidth / 2 / this.intervalWidth),
                      audioTrackList: this.audioTrackList,
                      index: index,
                      audioTrackId: info.audioTrackId,
                      selectedAudioTrackId: this.selectedAudioTrackId,
                      selectedAudioAssetStartTime: this.selectedAudioAssetStartTime,
                      forceRenderMusicIconState: this.forceRenderMusicIconState,
                      _currentTime: this._currentTime
                    })
                  }
                  .height(StyleConst.NINETEEN_PERCENT_HEIGHT)
                  .width(StyleConst.FULL_WIDTH)
                  .clip(true)
                  .margin({ left: $r('app.float.margin_10') })
                  .onAreaChange((oldArea, newArea) => {
                    Logger.info(TAG,
                      `onAreaChange realAudioWaveWidth: ${newArea.width}, realAudioWaveHeight: ${newArea.height}`)
                    this.realAudioWaveWidth = newArea.width as number;
                    this.realAudioWaveHeight = newArea.height as number;
                  })
                }, (info: AudioTrack) => info.audioTrackId)
              }
            }
            .width(`${100 - this.leftWidthPercent}%`)
            .backgroundColor($r('app.color.main_backgroundColor'))
          }
        }
      }
      .height(StyleConst.ONE_HUNDRED_AND_EIGHT_HEIGHT)
    }
    .onReady((context: NavDestinationContext) => {
      try {
        // 实时模式
        let result = audioNapi.audioEditNodeInit(2);
        Logger.info(TAG, `audioEditNodeInit result: ${result}`);
      } catch (e) {
        Logger.error(TAG, `audioEditNodeInit error: ${JSON.stringify(e)}`);
      }
    })
    .title($r('app.string.audio_track_edit'), { mainTitleModifier: this.mainTitleModifier })
    .menus(this.NavigationMenus())
    .height(StyleConst.FULL_HEIGHT)
    .width(StyleConst.FULL_WIDTH)
    .backgroundColor($r('app.color.mixing_console_backgroundColor'))
    .backButtonIcon(this.symbolModifier)
  }

  // setting dialog
  dialogControllerSet: CustomDialogController | null = new CustomDialogController({
    builder: SetFormatDialog({
      newAudioFileName: this.newAudioFileName,
      sampleRate: this.sampleRate,
      channels: this.channels,
      bitsPerSample: this.bitsPerSample,
      bitsPerSampleMode: this.bitsPerSampleMode,
      audioTrackList: this.audioTrackList,
      cancel: () => {
      },
      confirm: () => {
      }
    }),
    maskColor: $r('app.color.mixing_console_mask_color'),
  });

  @Builder
  silentAndSoloBuilder(audioTrack: AudioTrack, index: number) {
    Row() {
      Column() {
        Image(audioTrack.isSilent ? $r('app.media.ic_audio_sound_off') : $r('app.media.ic_audio_sound_off_black'))
          .width($r('app.float.height_20'))
          .height($r('app.float.height_20'))
          .onClick(() => {
            this.handleSilent(audioTrack, index);
          });
      }
      .padding({
        top: 2,
        bottom: 2,
        left: 5,
        right: 5
      })
      .backgroundColor(audioTrack.isSilent ? $r('app.color.silent_color') : Color.Gray)
      .borderRadius({ topLeft: 15, bottomLeft: 15 });

      Column()
        .width($r('app.float.width_1'))
        .height($r('app.float.height_20'))
        .backgroundColor(Color.Black);
      Column() {
        Image(audioTrack.isSolo ? $r('app.media.ic_audio_sound1') : $r('app.media.ic_audio_sound'))
          .width($r('app.float.height_20'))
          .height($r('app.float.height_20'))
          .onClick(() => {
            this.handleSolo(audioTrack, index);
          });
      }
      .padding({
        top: 2,
        bottom: 2,
        left: 5,
        right: 5
      })
      .backgroundColor(audioTrack.isSolo ? $r('app.color.solo_color') : Color.Gray)
      .borderRadius({ topRight: 15, bottomRight: 15 });
    }
  }

  @Builder
  NavigationMenus() {
    Row() {
      // Save
      Image($r('app.media.ic_save'))
        .width($r('app.float.height_30'))
        .height($r('app.float.height_30'))
        .margin({ right: $r('app.float.margin_10') })
        .onClick(() => {
          if (this.sampleRate === -1) {
            this.getUIContext().showAlertDialog({
              title: '提示',
              message: '请在设置页面输入采样率'
            });
            return;
          }
          if (this.channels === -1) {
            this.getUIContext().showAlertDialog({
              title: '提示',
              message: '请在设置页面输入声道'
            });
            return;
          }
          if (this.bitsPerSample === -1) {
            this.getUIContext().showAlertDialog({
              title: '提示',
              message: '请在设置页面输入位深'
            });
            return;
          }
          audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, this.bitsPerSampleMode);
          taskpool.execute(saveFileBuffer).then(async (value: object) => {
            Logger.info(TAG,
              `taskpool onClick save mixingConsole start, length: ${(value as ArrayBuffer).byteLength}`);
            this.SaveBuffer(value as ArrayBuffer);
            audioNapi.resetTotalWriteAudioDataSize();
          });
        })

      // Record
      Column() {
        if (this.recordState === 'init') {
          this.InitRecord();
        } else if (this.recordState === 'stopped') {
          this.FinishedRecord();
        }
      }

      // clear record
      Column() {
        Image($r('app.media.clean'))
          .width($r('app.float.height_30'))
          .height($r('app.float.height_30'))
          .margin({ right: $r('app.float.margin_10') })
          .onClick(() => {
            this.getUIContext().showAlertDialog({
              title: '清空录音',
              message: '是否清空录音',
              primaryButton: {
                value: '删除',
                action: () => {
                  // 用户确认后执行的逻辑
                  for (let id of this.recordInputIdSet) {
                    audioNapi.clearByInputId(id, 0); // 销毁录制的音频
                    Logger.warn(TAG, `recordInputID is ${id}`)
                  }
                  this.recordInputIdSet.clear();
                }
              },
              secondaryButton: {
                value: '取消',
                action: () => {
                }
              }
            });
          })
      }

      // Broadcast
      Image(!this.isPlay ? $r('app.media.ic_record_playing') : $r('app.media.ic_record_paused'))
        .width($r('app.float.height_30'))
        .height($r('app.float.height_30'))
        .margin({ right: $r('app.float.margin_10') })
        .onClick(() => {
          if (!this.audioTrackList.some(track => track.audioAssetArray?.length !== 0)) {
            this.getUIContext().showAlertDialog({
              title: '提示',
              message: '至少添加一个音频才可以播放'
            });
            return;
          }
          this.isPlayRealTime = true;
          this.isPlay = !this.isPlay;
          audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, this.bitsPerSampleMode);
          // 播放
          if (this.isPlay) {
            audioNapi.setCurrentTime(0);
            // 初始化 Renderer
            audioNapi.audioRendererInit();
            // 点击播放，才开始 AudioRenderer
            audioNapi.audioRendererStart();
          } else {
            // 暂停
            audioNapi.audioRendererStop();
            // 将buffer的map中的 totalWriteAudioDataSize 置为 0
            audioNapi.resetTotalWriteAudioDataSize();
          }
        })

      // Setting
      Image($r('app.media.ic_set'))
        .width($r('app.float.height_30'))
        .height($r('app.float.height_30'))
        .onClick(() => {
          // open setting dialog
          this.dialogControllerSet?.open();
        })
    }
    .margin({ top: $r('app.float.margin_10'), right: $r('app.float.margin_35') })
  }

  @Builder
  EffectNode(audioTrack: AudioTrack, index: number) {
    Row() {
      Column() {
        if (audioTrack.audioTrackId === this.selectedAudioTrackId) {
          this.Node(NodeType.SPACE_RENDER,
            audioTrack.audioAssetArray?.find(asset => asset.startTime ===
            this.selectedAudioAssetStartTime)?.nodes?.find(node => node.type ===
            NodeType.SPACE_RENDER && node.isByPass) ? true : false)
        } else {
          this.Node(NodeType.SPACE_RENDER, false)
        }
      }
      .onClick(() => {
        this.handleEffectNode(NodeType.SPACE_RENDER, index);
      })

      Column() {
        if (audioTrack.audioTrackId === this.selectedAudioTrackId) {
          this.Node(NodeType.EQ, audioTrack.audioAssetArray?.find(asset => asset.startTime ===
          this.selectedAudioAssetStartTime)?.nodes?.find(node => node.type ===
          NodeType.EQ && node.isByPass) ? true : false)
        } else {
          this.Node(NodeType.EQ, false)
        }
      }
      .onClick(() => {
        this.handleEffectNode(NodeType.EQ, index);
      })

      Column() {
        if (audioTrack.audioTrackId === this.selectedAudioTrackId) {
          this.Node(NodeType.VOICE_CHANGE,
            audioTrack.audioAssetArray?.find(asset => asset.startTime ===
            this.selectedAudioAssetStartTime)?.nodes?.find(node => node.type ===
            NodeType.VOICE_CHANGE && node.isByPass) ? true : false)
        } else {
          this.Node(NodeType.VOICE_CHANGE, false)
        }
      }
      .onClick(() => {
        this.handleEffectNode(NodeType.VOICE_CHANGE, index);
      })

      Column() {
        if (audioTrack.audioTrackId === this.selectedAudioTrackId) {
          this.Node(NodeType.EFFECT, audioTrack.audioAssetArray?.find(asset => asset.startTime ===
          this.selectedAudioAssetStartTime)?.nodes?.find(node => this.nodeTypes
            .find(type => type === node.type) && node.isByPass) ? true : false)
        } else {
          this.Node(NodeType.EFFECT, false)
        }
      }
      .onClick(() => {
        this.handleEffectNode(NodeType.EFFECT, index);
      })
    }
  }

  @Builder
  Node(type: NodeType | undefined, isShow?: boolean) {
    Text(type ?? '-')
      .height($r('app.float.height_30'))
      .width(StyleConst.TWENTY_TWO_PERCENT_WIDTH)
      .margin({ right: $r('app.float.margin_5') })
      .textAlign(TextAlign.Center)
      .backgroundColor(isShow ? ColorMap.get(type) : Color.Gray)
      .borderRadius(5)
      .fontColor(Color.White)
      .maxLines(2)
      .textOverflow({ overflow: TextOverflow.None })
  }

  @Builder
  MusicIcon(audioTrack: AudioTrack, index: number) {
    Column() {
      Image(audioTrack.audioAssetArray?.length ? $r('app.media.music6') :
        $r(`app.media.music${index % 5 + 1}`))
        .width($r('app.float.height_30'))
        .height($r('app.float.height_30'))
        .onClick(() => {
          if (!this.checkPreconditions() || audioTrack.audioAssetArray?.length) {
            return;
          }
          let playList: Map<string, SongInfo> = new Map();
          importSingleSong(playList).then((songMap: Map<string, SongInfo>) => {
            const inputId: string = util.generateRandomUUID(true);
            const outPutId: string = util.generateRandomUUID(true);
            const mixerId: string = util.generateRandomUUID(true);
            songMap.forEach((value: SongInfo, key) => {
              this.getPcmFileBuffer(value as SongInfo);
              //assemble audioAsset data
              this.audioTrackList[index].audioTrackId = inputId;
              this.importAudioAsset.assetId = this._currentTime;
              this.importAudioAsset.wavBuffer = this.wavBuffer;
              this.importAudioAsset.startTime = this._currentTime;
              this.importAudioAsset.pcmBuffer = this.pcmBuffer;
              assembleAudioAsset(this.importAudioAsset, value);
              try {
                this.isShowAudioWave[index] = true;
                this.isSelectIndex = index;
                this.isAudioCache = true;
                this.importAudioProgress.open();
                let nodeIds: string[] = [inputId, outPutId, mixerId];
                let result =
                  audioNapi.audioInAndOutInit(nodeIds, this.fd, this.wavBuffer.byteLength, this._currentTime);
                if (result !== 0) {
                  this.getUIContext().showAlertDialog({
                    title: '提示',
                    message: `error: ${result}`
                  });
                  return;
                }
              } catch (e) {
                Logger.info(TAG, `err: ${e}`);
              }
            })
          });
          this.forceRenderEffectNodeState = !this.forceRenderEffectNodeState;
        })
    }
    .margin({ left: $r('app.float.margin_5') })
  }

  getPcmFileBuffer(info: SongInfo): boolean {
    try {
      let path: string = new fileUri.FileUri(info.uri).path;
      Logger.info(TAG, `getPcmFileBuffer path: ${JSON.stringify(path)}`);
      let file = fs.openSync(path, fs.OpenMode.READ_ONLY | fs.OpenMode.CREATE);
      let fsStat = fs.statSync(path);
      Logger.info(TAG, `failSize : ${fsStat.size}`);
      // 不要删 --- let buffer = audioNapi.getFileBuffer(file.fd, fsStat.size);
      let buffer = new ArrayBuffer(fsStat.size);
      let readOption: ReadOptions = {
        offset: 0, // 期望读取文件的位置。可选，默认从当前位置开始读
        length: fsStat.size // 每次期望读取数据的长度。可选，默认缓冲区长度
      }
      fs.readSync(file.fd, buffer, readOption);
      Logger.info(TAG, `getPcmFileBuffer buffer length: ${buffer.byteLength}`);
      const wavView = new Uint8Array(buffer);
      let dataBuffer = wavView.slice(44).buffer;
      Logger.info(TAG, `wavBuffer length : ${buffer.byteLength}`);
      this.wavBuffer = buffer;
      if (dataBuffer instanceof ArrayBuffer) {
        this.pcmBuffer = dataBuffer;
        Logger.info(TAG, `this.pcmBuffer length : ${this.pcmBuffer.byteLength}`);
      }
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
      if (pcmBuffer.byteLength === 0) {
        Logger.info(TAG, 'SaveBuffer error');
      }
      let documentSaveOptions = new picker.DocumentSaveOptions();
      if (this.newAudioFileName === '') {
        this.newAudioFileName = getDateStringWithTimeStamp(new Date().getTime());
      } else {
        this.newAudioFileName = this.newAudioFileName + ' ' + getDateStringWithTimeStamp(new Date().getTime());
      }
      let newFileNames = `${this.newAudioFileName}${this.audioFormat}`;
      Logger.info(TAG, `SaveBuffer newFileNames is: ${newFileNames}`);
      documentSaveOptions.newFileNames = [newFileNames];
      let context = getContext() as common.Context; // 请确保 getContext(this) 返回结果为UIAbilityContext
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
        if (err) {
          Logger.error(TAG,
            `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
          return;
        }
        Logger.info(TAG,
          'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
        let filePath = documentSaveResult;
        let file = fs.openSync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        Logger.info(TAG, `SaveBuffer filePath : ${filePath}`);
        if (this.audioFormat === '.wav') {
          let wavBuffer = this.writeWavFileHeader(pcmBuffer);
          let writeLen = fs.writeSync(file.fd, wavBuffer);
          Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
          fs.closeSync(file);
        }
      })
    } catch (e) {
      Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
    }
  }

  writeWavFileHeader(pcmBuffer: ArrayBuffer): ArrayBuffer {
    Logger.info(TAG,
      `writeWavFileHeader sampleRate: ${this.sampleRate} --- channels: ${this.channels} --- bitsPerSample: ${this.bitsPerSample}`);
    const header = new ArrayBuffer(44);
    const dv = new DataView(header);
    this.writeString(dv, 0, 'RIFF');
    let totalDataLen = pcmBuffer.byteLength + 44;
    dv.setUint32(4, totalDataLen, true);
    this.writeString(dv, 8, 'WAVE');
    this.writeString(dv, 12, 'fmt ');
    dv.setUint32(16, 16, true);
    if (this.bitsPerSample === 32 && this.bitsPerSampleMode === BitsPerSampleMode.FLOAT) {
      dv.setUint16(20, 3, true);
    } else {
      dv.setUint16(20, 1, true);
    }
    dv.setUint16(22, this.channels, true);
    dv.setUint32(24, this.sampleRate, true);
    let byteRate = this.bitsPerSample * this.sampleRate * this.channels / 8;
    dv.setUint32(28, byteRate, true);
    dv.setUint16(32, this.channels * this.bitsPerSample / 8, true);
    dv.setUint16(34, this.bitsPerSample, true);
    this.writeString(dv, 36, 'data');
    dv.setUint32(40, pcmBuffer.byteLength, true);
    let resultBuffer = this.concatArrayBuffer(header, pcmBuffer);
    return resultBuffer;
  }

  writeString(dv: DataView, offset: number, str: string) {
    for (let i = 0; i < str.length; i++) {
      dv.setUint8(offset + i, str.charCodeAt(i));
    }
  }

  concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
    const viewWavHeader = new Uint8Array(wavHeaderBuffer);
    const viewPcm = new Uint8Array(pcmBuffer);
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);
    return resultBuffer;
  }

  checkPreconditions(): boolean {
    if (this.isAudioCache) {
      this.getUIContext().showAlertDialog({
        title: '音频添加中',
        message: '添加音频文件过大，请稍等'
      });
      return false;
    }
    return true;
  }

  /**
   * 根据 AudioTrack 的 isSilent 状态，更新 renderTrackList 和 notRenderTrackList
   *
   * @param notRenderTrackList - 初始为空的字符串数组，用于存放不渲染的 trackId
   * @param renderTrackList - 初始为空的字符串数组，用于存放渲染的 trackId
   * @param audioTrackList - 所有音轨的列表
   */
  updateTrackRenderLists(
    notRenderTrackList: string[],
    renderTrackList: string[],
    audioTrackList: AudioTrack[]
  ): void {
    //清空输入数组（每次进来前已初始化，但确保干净）
    notRenderTrackList.length = 0;
    renderTrackList.length = 0;

    //遍历所有音轨，根据 isSilent 分类
    audioTrackList.forEach(track => {
      if (track.isSilent) {
        notRenderTrackList.push(track.audioTrackId);
      } else {
        renderTrackList.push(track.audioTrackId);
      }
    });
    audioNapi.ModifyRender([...notRenderTrackList], [...renderTrackList]);
  }
}