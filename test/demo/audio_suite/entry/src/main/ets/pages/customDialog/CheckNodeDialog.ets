/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
import {
    Node,
    NodeType,
    ColorMap,
    SongInfo,
    SoundFiledType,
    EnvironmentType,
    VoiceBeautifierType,
    SoundFiledMap,
    EnvironmentMap,
    VoiceBeautifierMap,
    AudioSeparationType,
    AudioSeparationMap,
    NumberToNodeTypeMap,
    GeneralVoiceChangeType,
    PureVoiceChangeGenderOption,
    PureVoiceChangeType,
    ClockWise,
    SpaceRender,
    SpaceRenderMode,
    SpaceRenderModeMap,
    ClockWiseMap,
    SpacePosition
} from "../../utils/importSongs/InterfaceInfo";
import { JSON, util } from '@kit.ArkTS';
import { Logger } from '../../utils/Logger';
import { SpaceRenderPositionView } from '../common/SpaceRender';
import { AlertDialog } from '@kit.ArkUI';
import audioNapi from 'libentry.so';

const TAG: string = 'AudioEditTestApp_CheckNodeDialog';

// Balance
@Component
struct EqualizerComponent {
    // Balanced frequency band
    @Link balanceBandGains: number[];
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Column() {
            Text(`${NodeType.EQ}`)
                .margin({ bottom: $r('app.float.margin_10') })
            Flex({
                direction: FlexDirection.Row,
                justifyContent: FlexAlign.SpaceBetween,
                alignItems: ItemAlign.Center
            }) {
                ForEach(this.balanceBandGains, (item: number, index: number) => {
                    Column() {
                        Text(`${item}`).fontSize(12)
                        Slider({
                            value: item,
                            step: 1,
                            style: SliderStyle.OutSet,
                            direction: Axis.Vertical,
                            min: -10,
                            max: 10,
                            reverse: true
                        })
                            .blockSize({ width: 30, height: 30 })
                            .blockBorderWidth(5)
                            .blockColor(Color.White)
                            // .showSteps(true)
                            .onChange((value: number, mode: SliderChangeMode) => {
                                this.balanceBandGains[index] = value;
                            })
                    }.width('5%').height('200vp')
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
                    })
                Button('确定')
                    .onClick(() => {
                        if (this.onAgree) {
                            this.onAgree();
                        }
                    })
            }.margin({ top: $r('app.float.margin_10') })
        }.padding($r('app.float.padding_10'))
    }
}

// sound field
@Component
struct FieldComponent {
    @Link selectedSoundFiled: SoundFiledType;
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Column() {
            Text(`${NodeType.FIELD}`)
                .margin({ bottom: $r('app.float.margin_10') })
            Row() {
                Column() {
                    Radio({ value: 'close', group: 'fieldEffect' })
                        .checked(this.selectedSoundFiled === SoundFiledType.SOUND_FIELD_CLOSE)
                        .onChange(() => {
                            // tongjian OH_AudioSuite_Result OH_AudioSuiteEngine_BypassEffectNode(OH_AudioNode* audioNode, bool bypass);
                            this.selectedSoundFiled = SoundFiledType.SOUND_FIELD_CLOSE;
                        })
                    Text('无').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'front', group: 'fieldEffect' })
                        .checked(this.selectedSoundFiled === SoundFiledType.SOUND_FIELD_FRONT_FACING)
                        .onChange(() => {
                            this.selectedSoundFiled = SoundFiledType.SOUND_FIELD_FRONT_FACING;
                        })
                    Text('前置').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'grand', group: 'fieldEffect' })
                        .checked(this.selectedSoundFiled === SoundFiledType.SOUND_FIELD_GRAND)
                        .onChange(() => {
                            this.selectedSoundFiled = SoundFiledType.SOUND_FIELD_GRAND;
                        })
                    Text('宏大').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'near', group: 'fieldEffect' })
                        .checked(this.selectedSoundFiled === SoundFiledType.SOUND_FIELD_NEAR)
                        .onChange(() => {
                            this.selectedSoundFiled = SoundFiledType.SOUND_FIELD_NEAR;
                        })
                    Text('聆听').fontSize(20)
                }
                .width('20%')

                Column() {

                    Radio({ value: 'broad', group: 'fieldEffect' })
                        .checked(this.selectedSoundFiled === SoundFiledType.SOUND_FIELD_WIDE)
                        .onChange(() => {
                            this.selectedSoundFiled = SoundFiledType.SOUND_FIELD_WIDE;
                        })
                    Text('宽广').fontSize(20)
                }
                .width('20%')
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
                            this.onAgree();
                        }
                    })
            }.margin({ top: $r('app.float.margin_10') })
        }
        .padding($r('app.float.padding_10'))
    }
}

// environment
@Component
struct EnvironmentComponent {
    @Link selectedEnvironment: EnvironmentType;
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Column() {
            Text(`${NodeType.ENV}`)
                .margin({ bottom: $r('app.float.margin_10') })
            Row() {
                Column() {
                    Radio({ value: 'close', group: 'envEffect' })
                        .checked(this.selectedEnvironment === EnvironmentType.ENVIRONMENT_TYPE_CLOSE)
                        .onChange(() => {
                            this.selectedEnvironment = EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
                        })
                    Text('无').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'broadcast', group: 'envEffect' })
                        .checked(this.selectedEnvironment === EnvironmentType.ENVIRONMENT_TYPE_BROADCAST)
                        .onChange(() => {
                            this.selectedEnvironment = EnvironmentType.ENVIRONMENT_TYPE_BROADCAST;
                        })
                    Text('广播').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'earpiece', group: 'envEffect' })
                        .checked(this.selectedEnvironment === EnvironmentType.ENVIRONMENT_TYPE_EARPIECE)
                        .onChange(() => {
                            this.selectedEnvironment = EnvironmentType.ENVIRONMENT_TYPE_EARPIECE;
                        })
                    Text('听筒').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'underWater', group: 'envEffect' })
                        .checked(this.selectedEnvironment === EnvironmentType.ENVIRONMENT_TYPE_UNDERWATER)
                        .onChange(() => {
                            this.selectedEnvironment = EnvironmentType.ENVIRONMENT_TYPE_UNDERWATER;
                        })
                    Text('水下').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'gramophone', group: 'envEffect' })
                        .checked(this.selectedEnvironment === EnvironmentType.ENVIRONMENT_TYPE_GRAMOPHONE)
                        .onChange(() => {
                            this.selectedEnvironment = EnvironmentType.ENVIRONMENT_TYPE_GRAMOPHONE;
                        })
                    Text('留声机').fontSize(20)
                }
                .width('20%')
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
                            this.onAgree();
                        }
                    })
            }.margin({ top: $r('app.float.margin_10') })
        }
        .padding($r('app.float.padding_10'))
    }
}

// deaden
@Component
struct NoiseReductionComponent {
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

// beautify
@Component
struct VoiceBeautifierComponent {
    @Link selectedVoiceBeautifier: VoiceBeautifierType;
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Column() {
            Text(`${NodeType.VB}`)
                .margin({ bottom: $r('app.float.margin_10') })
            Row() {

                Column() {
                    Radio({
                        value: 'clear',
                        group: 'voiceBeautifier'
                    })
                        .checked(this.selectedVoiceBeautifier === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR)
                        .onChange(() => {
                            this.selectedVoiceBeautifier = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR;
                        })
                    Text('清澈').fontSize(20)
                }
                .width('20%')


                Column() {
                    Radio({ value: 'theatre', group: 'voiceBeautifier' })
                        .checked(this.selectedVoiceBeautifier === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_THEATRE)
                        .onChange(() => {
                            this.selectedVoiceBeautifier = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_THEATRE;
                        })
                    Text('剧场').fontSize(20)
                }
                .width('20%')

                Column() {
                    Radio({ value: 'cd', group: 'voiceBeautifier' })
                        .checked(this.selectedVoiceBeautifier === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CD)
                        .onChange(() => {
                            this.selectedVoiceBeautifier = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CD;
                        })
                    Text('CD音效').fontSize(20)
                }
                .width('20%')

                Column() {

                    Radio({ value: 'recordingStudio', group: 'voiceBeautifier' })
                        .checked(this.selectedVoiceBeautifier === VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_STUDIO)
                        .onChange(() => {
                            this.selectedVoiceBeautifier = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_STUDIO;
                        })
                    Text('录音棚').fontSize(20)
                }
                .width('20%')
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
                            this.onAgree();
                        }
                    })
            }.margin({ top: $r('app.float.margin_10') })
        }
        .padding($r('app.float.padding_10'))
    }
}

// Audio source separation
@Component
struct AudioSeparationComponent {
    @Link selectedAudioSeparation: AudioSeparationType;
    onCancel?: () => void;
    onAgree?: () => void;
    @State separatingShow: boolean = false;
    @State aissProgress: number = 0;

    build() {
        Column() {
            Row() {
                Column() {
                    Image($r('app.media.close_window'))
                        .width(20)
                        .height(20)
                }
                .width('20%')
                .onClick(() => {
                    if (this.onCancel) {
                        this.onCancel();
                    }
                })

                Column() {
                    Text('音源分离').fontSize(20)
                    Text('提取的音频类型将作于后续节点输入').fontSize(10).opacity(50)
                }
                .width('50%')

                Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
                    Button('提取').onClick(() => {
                        if (this.onAgree) {
                            this.onAgree();
                        }
                    })
                        .height($r('app.float.height_50'))
                }
            }
            .height($r('app.float.height_100'))
            .alignItems(VerticalAlign.Center)

            Row() {
                Column() {
                    Row() {
                        Text('人声').fontSize(20).margin({ right: 100 })
                        if (this.separatingShow && this.aissProgress < 100) {
                            Progress({ value: this.aissProgress, total: 150, type: ProgressType.Ring })
                                .width(30)
                                .height(30)
                        } else {
                            Radio({ value: 'human', group: 'separation' })
                                .checked(this.selectedAudioSeparation ===
                                AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE)
                                .onChange((isOn: boolean) => {
                                    this.selectedAudioSeparation =
                                        AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE
                                })
                        }
                    }
                    .height($r('app.float.height_50'))

                    Row() {
                        Text('伴奏 ').fontSize(20).margin({ right: 100 })
                        if (this.separatingShow && this.aissProgress <= 100) {
                            Progress({ value: this.aissProgress, total: 150, type: ProgressType.Ring })
                                .width(30)
                                .height(30)
                        } else {
                            Radio({ value: 'backGround', group: 'separation' })
                                .checked(this.selectedAudioSeparation ===
                                AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE)
                                .onChange((isOn: boolean) => {
                                    if (isOn) {
                                        this.selectedAudioSeparation =
                                            AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE
                                    }
                                })
                        }
                    }
                    .height($r('app.float.height_50'))
                }

            }
            .height($r('app.float.height_100'))

        }
    }
}

// Spatial Rendering
@Component
struct SpaceRenderComponent {
  @Prop spaceRenderPara: SpaceRender;
  onCancel?: () => void;
  onAgree?: (spaceRenderState: SpaceRender) => void;

  // Independent status per line
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
              .height(50)
              .padding({ left: 10 })
              .fontSize(16)
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

        // This line of content is displayed only when the button is turned on
        if (this.spaceRenderState.mode === 0) {
          this.RenderView()
        }

        // Second Line: Dynamic Rendering
        Row() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text("动态渲染")
              .height(50)
              .padding({ left: 10 })
              .fontSize(16)
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
                .fontSize(14)
                .margin({ left: $r('app.float.margin_10') })
              Text(`${this.spaceRenderState.singleWeekCycleTime}`)
                .fontSize(14)
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
          .borderRadius(15)

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
              .fontSize(14)
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
              .fontSize(14)
          }
          .width('100%')
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
                max: 360,
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
            })
          Button('确定')
            .margin({ right: $r('app.float.margin_10') })
            .backgroundColor(0xFFFFFF)
            .fontColor(Color.Black)
            .onClick(() => {
              if (this.onAgree) {
                this.onAgree(this.spaceRenderState);
              }
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
            .fontSize(14)
        }
        .width('50%')
        Column() {
          Text('俯视图')
            .fontSize(14)
        }
        .width('50%')
      }
      .margin({ top: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })
      Row() {
        Column() {
          Text(this.getFormattedCoords())
            .fontSize(14)
            .fontColor('#ffffff')
            .fontWeight(FontWeight.Bold)
            .background('#2C3E50')  // Dark blue gray, modern
            .borderRadius(8)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .width('auto')
            .textAlign(TextAlign.Center)
            .shadow({ color: '#00000020', radius: 4, offsetX: 0, offsetY: 2 })
            .margin({ bottom: 8 })
        }
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
      }
      .justifyContent(FlexAlign.Center)
      .alignItems(VerticalAlign.Center)
      .width('100%')

      Row() {
        SpaceRenderPositionView({
          spaceRender: this.spaceRenderState
        })
      }
      .margin({ bottom: $r('app.float.margin_10') })
    }
  }
}

// Sonic pitch
@Component
struct SoundSpeedToneComponent {
  // Sound Speed
  @Link soundSpeed: number;
  // Sound Tone
  @Link soundTone: number;
  onCancel?: () => void;
  onAgree?: () => void;
 
  build() {
    Column() {
      Row() {
        Text(`${NodeType.SOUND_SPEED_TONE}`)
          .margin({ bottom: $r('app.float.margin_10') })
      }
      .margin({ top: $r('app.float.margin_10') })
 
      Column() {
        Column() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text('音速')
            Text(`${this.soundSpeed.toFixed(1)}x`)
          }.padding('10px')
          Slider({
            value: this.soundSpeed,
            step: 0.1,
            style: SliderStyle.OutSet,
            min: 0.5,
            max: 10,
          })
            .selectedColor($r('app.color.index_audio_clip_card'))
            .onChange((value: number, mode: SliderChangeMode) => {
              this.soundSpeed = value;
            })
        }
        .height(50)
        .margin({bottom: $r('app.float.margin_10')})
        .padding('10px')
        .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
        .borderRadius(10)
        Column() {
          Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
            Text('音调')
            Text(`+${this.soundTone.toFixed(1)}`)
          }.padding('10px')
          Slider({
            value: this.soundTone,
            step: 0.1,
            style: SliderStyle.OutSet,
            min: 0.1,
            max: 5,
          })
            .selectedColor($r('app.color.index_audio_clip_card'))
            .onChange((value: number, mode: SliderChangeMode) => {
              this.soundTone = value;
            })
        }
        .height(50)
        .padding('10px')
        .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
        .borderRadius(10)
      }
      .margin({right: 20})
      .padding({left: '10%', right: '10%'})
 
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Button('取消')
          .onClick(() => {
            if (this.onCancel) {
              this.onCancel()
            }
          })
        Button('确定')
          .onClick(() => {
            if (this.onAgree) {
              this.onAgree();
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
      .padding(20)
    }
  }
}

@Component
struct GeneralVoiceChangeComponent {
    private scrollerIndex: Scroller = new Scroller();
    @Link generalVoiceChangeType: GeneralVoiceChangeType;
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Scroll(this.scrollerIndex) {
            Column() {
                Row() {
                    Text(`${NodeType.GENERAL_VOICE_CHANGE}`)
                        .margin({ bottom: $r('app.float.margin_10') })
                }
                .margin({ top: $r('app.float.margin_10') })

                Column() {
                    Grid() {
                        GridItem() {
                            Column() {
                                Radio({ value: 'close', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE
                                    })
                                Text('无').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'cute', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CUTE)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CUTE
                                    })
                                Text('可爱').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'cyberpunk', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK
                                    })
                                Text('赛博朋克').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'female', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_FEMALE)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_FEMALE
                                    })
                                Text('女声').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'male', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MALE)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MALE
                                    })
                                Text('男声').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'mix', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MIX)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MIX
                                    })
                                Text('混响').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'monster', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MONSTER)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MONSTER
                                    })
                                Text('怪物').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'seasoned', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SEASONED)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SEASONED
                                    })
                                Text('沉稳').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'synth', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SYNTH)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SYNTH
                                    })
                                Text('合成器').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'trill', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_TRILL)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_TRILL
                                    })
                                Text('颤音').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'war', group: 'GeneralVoiceChangeType' })
                                    .checked(this.generalVoiceChangeType == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_WAR)
                                    .onChange(() => {
                                        this.generalVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_WAR
                                    })
                                Text('战争').fontSize(20)
                            }.width(80)
                        }
                    }.rowsTemplate('1fr 1fr').height('80%')
                }
                .margin({ right: 20 })
                .padding({ left: '10%', right: '10%' })

                Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
                    Button('取消')
                        .onClick(() => {
                            if (this.onCancel) {
                                this.onCancel()
                            }
                        })
                    Button('确定')
                        .onClick(() => {
                            if (this.onAgree) {
                                this.onAgree();
                            }
                        })
                }.margin({ top: $r('app.float.margin_10') })
                .padding(20)
            }
        }
    }
}

@Component
struct PureVoiceChangeComponent {
    private scrollerIndex: Scroller = new Scroller();
    @Link pureVoiceGender: PureVoiceChangeGenderOption;
    @Link pureVoiceChangeType: PureVoiceChangeType;
    @Link soundTone: number;
    @State needSoundTone: boolean = false;
    onCancel?: () => void;
    onAgree?: () => void;

    build() {
        Scroll(this.scrollerIndex) {
            Column() {
                Row() {
                    Text(`${NodeType.GENERAL_VOICE_CHANGE}`)
                        .margin({ bottom: $r('app.float.margin_10') })
                }
                .margin({ top: $r('app.float.margin_10') })

                Column() {
                    Row() {
                        Text("性别")
                    }

                    Grid() {
                        GridItem() {
                            Column() {
                                Radio({ value: 'close', group: 'PureVoiceGender' })
                                    .checked(this.pureVoiceGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE)
                                    .onChange(() => {
                                        this.pureVoiceGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE
                                    })
                                Text('无').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'close', group: 'PureVoiceGender' })
                                    .checked(this.pureVoiceGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_FEMALE)
                                    .onChange(() => {
                                        this.pureVoiceGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_FEMALE
                                    })
                                Text('女性').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'close', group: 'PureVoiceGender' })
                                    .checked(this.pureVoiceGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_MALE)
                                    .onChange(() => {
                                        this.pureVoiceGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_MALE
                                    })
                                Text('男性').fontSize(20)
                            }.width(80)
                        }
                    }

                    Row() {
                        Text("变声类型")
                    }

                    Grid() {
                        GridItem() {
                            Column() {
                                Radio({ value: 'close', group: 'GeneralVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE
                                    })
                                Text('无').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'cute', group: 'GeneralVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CARTOON)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CARTOON
                                    })
                                Text('卡通').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'cute', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CUTE)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CUTE
                                    })
                                Text('可爱').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'female', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_FEMALE)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_FEMALE
                                    })
                                Text('女声').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'male', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MALE)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MALE
                                    })
                                Text('男声').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'monster', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MONSTER)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MONSTER
                                    })
                                Text('怪物').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'robot', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_ROBOTS)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_ROBOTS
                                    })
                                Text('机器人').fontSize(20)
                            }.width(80)
                        }

                        GridItem() {
                            Column() {
                                Radio({ value: 'seasoned', group: 'PureVoiceChangeType' })
                                    .checked(this.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_SEASONED)
                                    .onChange(() => {
                                        this.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_SEASONED
                                    })
                                Text('沉稳').fontSize(20)
                            }.width(80)
                        }
                    }.rowsTemplate('1fr 1fr').height('80%')
                    Column() {
                        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
                        Text('音调')
                        Checkbox({ name: 'pureSoundTone', group: 'pureSoundTone' })
                            .select(false)
                            .selectedColor('app.color.index_audio_clip_card')
                            .shape(CheckBoxShape.CIRCLE)
                            .onChange((value: boolean) => {
                            this.needSoundTone = value;
                            if (!value) {
                                this.soundTone = 0;
                            }
                            })
                        Text(`+${this.soundTone.toFixed(1)}`)
                        }.padding('10px')
                        if (this.needSoundTone) {
                        Slider({
                            value: this.soundTone,
                            step: 0.1,
                            style: SliderStyle.OutSet,
                            min: 0.3,
                            max: 3,
                        })
                            .selectedColor($r('app.color.index_audio_clip_card'))
                            .onChange((value: number) => {
                            this.soundTone = value;
                            })
                        }
                    }
                    .height(50)
                    .padding('10px')
                    .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
                    .borderRadius(10)
                }
                .margin({ right: 20 })
                .padding({ left: '10%', right: '10%' })

                Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
                    Button('取消')
                        .onClick(() => {
                            if (this.onCancel) {
                                this.onCancel()
                            }
                        })
                    Button('确定')
                        .onClick(() => {
                            if (this.onAgree) {
                                this.onAgree();
                            }
                        })
                }.margin({ top: $r('app.float.margin_10') })
                .padding(20)
            }
        }
    }
}

@CustomDialog
struct EffectNodeDialog {
    @StorageLink('isCurrentMulPipeline') isCurrentMulPipeline: boolean = false;
    @StorageLink('currentPipelineId') currentPipelineId: string = '';
    @Prop selectedNode: string = '';
    @Prop selectedEffectNode: NodeType;
    @Prop currentInputId: string = '';
    @Prop isLeft: boolean;
    @Link mixerList: Array<Node>;
    @Link inputList: SongInfo[];
    @Link nodeList: Map<string, SongInfo>;
    // Balanced frequency band
    @State balanceBandGains: number[] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    // Sound field data, default is none
    @State selectedSoundFiled: SoundFiledType = SoundFiledType.SOUND_FIELD_CLOSE;
    // Environmental data, default is none
    @State selectedEnvironment: EnvironmentType = EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
    // Beautify data, default is normal
    @State selectedVoiceBeautifier: VoiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL;
    // Noise reduction data, default is none
    @State selectedNoiseReduction: number = -1;
    // Audio source separation, default is vocals.
    @State selectedAudioSeparation: AudioSeparationType = AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE;
    // Spatial Rendering
    @State spaceRenderPara: SpaceRender = {
        mode: SpaceRenderMode.CLOSE,
        fixedPosition: {x: 0, y: 0, z: 0},
        dynamicPosition: {x: 0, y: 0, z: 0},
        singleWeekCycleTime: 2,
        clockWise: ClockWise.CLOCK_WISE,
        expansionRadius: 1,
        expansionAngle: 1
    };
    // soundSpeed
    @State soundSpeed: number = 1.0;
    // soundTone
    @State soundTone: number = 1.0;
    // Real-time playback (audio source separation)
    @Consume isSeparation: boolean = false;
    @State generalVoiceChangeType: GeneralVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE;
    @State pureVoiceGender: PureVoiceChangeGenderOption = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE;
    @State pureVoiceChangeType: PureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE;
    @State pureVoiceSoundTone: number = 0;
    controllerTwo?: CustomDialogController;

    build() {
        Column() {
            if (this.selectedEffectNode === NodeType.EQ) {
                EqualizerComponent({
                    balanceBandGains: this.balanceBandGains,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        let equalizerId = util.generateRandomUUID(true);
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result =
                            audioNapi.setEqualizerFrequencyBandGains(this.balanceBandGains, equalizerId, this.currentInputId,
                            this.selectedNode);
                        if (result !== 0) {
                            Logger.error(TAG, `setEqualizerFrequencyBandGains result: ${result}`);
                            this.getUIContext().showAlertDialog({
                                message: '添加均衡节点失败',
                                autoCancel: true
                            });
                            return;
                        }

                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: equalizerId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            balanceBandGains: this.balanceBandGains
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.FIELD) {
                FieldComponent({
                    selectedSoundFiled: this.selectedSoundFiled,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        // Underlying effect node for calling NAPI
                        const nodeId = util.generateRandomUUID(true);
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result =
                            audioNapi.startFieldEffect(this.currentInputId, this.selectedSoundFiled, nodeId, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加声场节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            soundFiledType: this.selectedSoundFiled
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.ENV) {
                EnvironmentComponent({
                    selectedEnvironment: this.selectedEnvironment,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        // Underlying effect node for calling NAPI
                        const nodeId = util.generateRandomUUID(true);
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result =
                            audioNapi.startEnvEffect(this.currentInputId, nodeId, this.selectedEnvironment, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加环境节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            environmentType: this.selectedEnvironment
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.NR) {
                NoiseReductionComponent({
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        // Underlying effect node for calling NAPI

                        // Adding effect nodes to the mixer --- Data processing
                        const nodeId = util.generateRandomUUID(true);
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result = audioNapi.addNoiseReduction(nodeId, this.currentInputId, this.selectedNode);
            
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加降噪节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode)
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.VB) {
                VoiceBeautifierComponent({
                    selectedVoiceBeautifier: this.selectedVoiceBeautifier,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        // Underlying effect node for calling NAPI
                        let voiceBeautifierId = util.generateRandomUUID(true);
                        Logger.error(TAG, `-----${this.selectedNode}`)
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result =
                            audioNapi.startVBEffect(this.currentInputId, this.selectedVoiceBeautifier, voiceBeautifierId,
                                this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加美化节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: voiceBeautifierId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            voiceBeautifierType: this.selectedVoiceBeautifier
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.SEPARATION) {
                AudioSeparationComponent({
                    selectedAudioSeparation: this.selectedAudioSeparation,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        // Underlying effect node for calling NAPI
                        const nodeId = util.generateRandomUUID(true);
                        let result = -1;
                        if (this.isCurrentMulPipeline) {
                            result = audioNapi.multiPipelineEnvPrepare(this.currentPipelineId);
                        }
                        result = audioNapi.addAudioSeparation(this.selectedAudioSeparation, nodeId, this.currentInputId,
                            this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: `添加${AudioSeparationMap.get(this.selectedAudioSeparation)}失败`,
                                autoCancel: true
                            });
                            this.isSeparation = false;
                            return;
                        }
                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            audioSeparationType: this.selectedAudioSeparation
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                        this.isSeparation = true;
                    }
                })
            } else if (this.selectedEffectNode === NodeType.SPACE_RENDER) {
                SpaceRenderComponent({
                    spaceRenderPara: this.spaceRenderPara,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: (spaceRenderState: SpaceRender) => {
                        // Underlying effect node for calling NAPI
                        const nodeId = util.generateRandomUUID(true);
                        if (spaceRenderState.mode === -1) {
                        this.getUIContext().showAlertDialog({
                            message: `请选择一种模式`,
                            autoCancel: true
                        });
                        return;
                        }
                        if (spaceRenderState.mode === 0) {
                        let result = audioNapi.startFixedPositionEffect(spaceRenderState.fixedPosition?.x,
                            spaceRenderState.fixedPosition?.y, spaceRenderState.fixedPosition?.z, nodeId,
                            this.currentInputId, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: `添加空间渲染节点失败`,
                                autoCancel: true
                            });
                            return;
                        }
                        }
                        if (spaceRenderState.mode === 1) {
                            let result = audioNapi.startDynamicRenderEffect(spaceRenderState.dynamicPosition?.x,
                                spaceRenderState.dynamicPosition?.y, spaceRenderState.dynamicPosition?.z,
                                spaceRenderState.singleWeekCycleTime, Number(spaceRenderState.clockWise), nodeId,
                                this.currentInputId, this.selectedNode);
                            if (result !== 0) {
                                this.getUIContext().showAlertDialog({
                                    message: `添加空间渲染节点失败`,
                                    autoCancel: true
                                });
                                return;
                            }
                        }
                        if (spaceRenderState.mode === 2) {
                        let result =
                            audioNapi.startExpandEffect(spaceRenderState.expansionRadius, spaceRenderState.expansionAngle,
                            nodeId, this.currentInputId, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: `添加空间渲染节点失败`,
                                autoCancel: true
                            });
                            return;
                        }
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
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            spaceRender: this.spaceRenderPara
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.SOUND_SPEED_TONE) {
                SoundSpeedToneComponent({
                    soundSpeed: this.soundSpeed,
                    soundTone: this.soundTone,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        let soundSpeedToneId = util.generateRandomUUID(true);
                        let result = audioNapi.setSoundSpeedTone(this.currentInputId, soundSpeedToneId, this.soundSpeed, this.soundTone, this.selectedNode);
                        if (result !== 0) {
                            // Sonic pitch effect node setup failed
                            this.getUIContext().showAlertDialog({
                                message: '设置音速音调失败',
                                confirm: {
                                    value: '确定',
                                    action: () => {}
                                },
                                cancel: () => {
                                    Logger.info(TAG, `cancel`);
                                }
                            });
                            return;
                        }
                        // Adding effect nodes to the mixer --- Data processing
                        let node: Node = {
                            id: soundSpeedToneId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            soundSpeed: this.soundSpeed,
                            soundTone: this.soundTone,
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);
                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.GENERAL_VOICE_CHANGE) {
                GeneralVoiceChangeComponent({
                    generalVoiceChangeType: this.generalVoiceChangeType,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        const nodeId = util.generateRandomUUID(true);
                        let result = audioNapi.startGeneralVoiceChange(this.currentInputId, this.generalVoiceChangeType, nodeId, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加通用变声节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            environmentType: this.selectedEnvironment
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            } else if (this.selectedEffectNode === NodeType.PURE_VOICE_CHANGE) {
                PureVoiceChangeComponent({
                    pureVoiceGender : this.pureVoiceGender,
                    pureVoiceChangeType : this.pureVoiceChangeType,
                    soundTone : this.pureVoiceSoundTone,
                    onCancel: () => {
                        this.controllerTwo?.close();
                    },
                    onAgree: () => {
                        const nodeId = util.generateRandomUUID(true);
                        let result = audioNapi.startPureVoiceChange(this.currentInputId, nodeId, this.pureVoiceGender, this.pureVoiceSoundTone, this.pureVoiceChangeType, this.selectedNode);
                        if (result !== 0) {
                            this.getUIContext().showAlertDialog({
                                message: '添加传统变声节点失败',
                                autoCancel: true
                            });
                            return;
                        }
                        let node: Node = {
                            id: nodeId,
                            type: this.selectedEffectNode,
                            color: ColorMap.get(this.selectedEffectNode),
                            environmentType: this.selectedEnvironment
                        }
                        this.isLeft ? this.addEffectNodeToInputList(node) : this.addEffectNodeToMixerList(node);

                        this.controllerTwo?.close();
                    }
                })
            }
        }
    }

    addEffectNodeToMixerList(node: Node) {
        const index = this.mixerList.findIndex(node => node.id === this.selectedNode);
        if (index !== -1) {
            Logger.info(TAG, `addEffectNodeToMixerList mixerList length: ${this.mixerList.length}`);
            this.mixerList.splice(index + 1, 0, node);
        } else {
            this.mixerList.unshift(node);
        }
    }

    addEffectNodeToInputList(node: Node) {
        if (this.inputList.find((item: SongInfo) => item?.nodeId === this.selectedNode)) {
            // If the node with the long-pressed "+" is an input node, then add the newly added effect node to the corresponding nodes
            let songInfo = this.nodeList.get(this.selectedNode);
            if (songInfo) {
                let index = (songInfo.nodes ?? []).findIndex(node => node.id === this.selectedNode);
                if (index !== -1) {
                    songInfo.nodes?.splice(index + 1, 0, node);
                }
                this.nodeList.set(this.selectedNode, songInfo);
                Logger.error(TAG, `${this.nodeList.get(songInfo?.nodeId ?? '')?.nodes?.length} --- `);
            }
        } else {
            // If the node that is long-pressed with the "+" sign is an effect node
            const inputId = this.currentInputId;
            if (inputId) {
                let songInfo = this.nodeList.get(inputId);
                if (songInfo) {
                    // The newly added effect node is inserted after the long-pressed button.
                    let index = (songInfo.nodes ?? []).findIndex(node => node.id === this.selectedNode);
                    songInfo.nodes?.splice(index + 1, 0, node);
                    this.nodeList.set(inputId, songInfo);
                    Logger.error(TAG, `${this.nodeList.get(songInfo?.nodeId ?? '')?.nodes?.length} --- `);
                }
            }
        }
    }
}

// "?" pop-up window
@CustomDialog
struct NodeDetailDialog {
    @Prop nodeType: NodeType;
    @Prop isLeft: boolean;
    @Prop selectedNode: string;
    @Prop currentInputId: string;
    @Link mixerList: Array<Node>;
    @Link inputList: SongInfo[];
    @Link nodeList: Map<string, SongInfo>;
    nodeDetailController?: CustomDialogController;
    private balanceBandGains: number[] = [];
    private soundFiledType = '-';
    private environmentType = '-';
    private voiceBeautifierType = '-';
    private audioSeparation = '-';
    private spaceRenderPara: SpaceRender = {
        mode: SpaceRenderMode.CLOSE,
        fixedPosition: {x: 0, y: 0, z: 0},
        dynamicPosition: {x: 0, y: 0, z: 0},
        singleWeekCycleTime: 2,
        clockWise: ClockWise.CLOCK_WISE,
        expansionRadius: 1,
        expansionAngle: 1
    };

    // Parameters of the input node
    private songName = '';
    private singerName = '';
    private channels = -1;
    private sampleRate = -1;
    private bitsPerSample = -1;

    aboutToAppear(): void {
        let mixerNode: Node | undefined = this.mixerList.find(node => node.id === this.selectedNode);
        let leftNode: SongInfo | undefined =
            this.inputList.find(node => node?.nodeId === this.selectedNode);
        if (this.isLeft) {
            // Current is input node
            if (leftNode) {
                let inputNode = this.nodeList.get(this.selectedNode);
                if (inputNode) {
                    this.songName = leftNode.songName as string;
                    this.singerName = leftNode.singerName as string;
                    this.channels = inputNode.channels ?? -1;
                    this.sampleRate = inputNode.sampleRate ?? -1;
                    this.bitsPerSample = inputNode.bitsPerSample ?? -1;
                }
            } else {
                // Current is the effect node
                let leftEffectNode: Node | undefined =
                    this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id === this.selectedNode)
                // Add the logic for getOption here, with the input parameter being only nodeId.
                if (leftEffectNode) {
                    let type = audioNapi.getOptions(leftEffectNode.id)
                    if (leftEffectNode?.type === NodeType.EQ) {
                        try {
                            this.balanceBandGains = (JSON.parse(type) as number[]) ?? [];
                        } catch (err) {
                            this.balanceBandGains = [];
                        }
                    } else if (leftEffectNode?.type === NodeType.FIELD) {
                        this.soundFiledType = SoundFiledMap.get(parseInt(type) as SoundFiledType) ?? '-';
                    } else if (leftEffectNode?.type === NodeType.ENV) {
                        this.environmentType = EnvironmentMap.get(parseInt(type) as EnvironmentType) ?? '-';
                    } else if (leftEffectNode?.type === NodeType.VB) {
                        this.voiceBeautifierType = VoiceBeautifierMap.get(parseInt(type) as VoiceBeautifierType) ?? '-';
                    } else if (leftEffectNode?.type === NodeType.SEPARATION) {
                        this.audioSeparation = AudioSeparationMap.get(leftEffectNode.audioSeparationType) ?? '-';
                    } else if (leftEffectNode?.type === NodeType.SPACE_RENDER) {
                        this.spaceRenderPara = leftEffectNode.spaceRender ?? this.spaceRenderPara;
                        if (leftEffectNode.spaceRender?.mode === SpaceRenderMode.FIXED_POSITION) {
                            let result = audioNapi.getFixedPositionParams(leftEffectNode.id);
                            Logger.info(TAG, `result is ${result.x} ${result.y} ${result.z}`);
                            this.spaceRenderPara.fixedPosition = {
                                x: result.x,
                                y: result.y,
                                z: result.z
                            }
                        } else if (leftEffectNode.spaceRender?.mode === SpaceRenderMode.DYNAMIC_RENDER) {
                            let result = audioNapi.getDynamicRenderParams(leftEffectNode.id);
                            Logger.info(TAG,
                                `result is ${result.x} ${result.y} ${result.z} ${result.surroundTime} ${result.surroundDirection}`);
                            this.spaceRenderPara.dynamicPosition = {
                                x: result.x,
                                y: result.y,
                                z: result.z
                            }
                            this.spaceRenderPara.singleWeekCycleTime = result.surroundTime;
                            this.spaceRenderPara.clockWise = result.surroundDirection;
                        } else if (leftEffectNode.spaceRender?.mode === SpaceRenderMode.EXPAND) {
                            let result = audioNapi.getExpandParams(leftEffectNode.id);
                            Logger.info(TAG, `result is ${result.extRadius} ${result.extAngle}`);
                            this.spaceRenderPara.expansionRadius = result.extRadius;
                            this.spaceRenderPara.expansionAngle = result.extAngle;
                        }
                    }
                }
            }
        } else {
            // Right Initialization Assignment
            let type = audioNapi.getOptions(mixerNode?.id)
            if (mixerNode?.type === NodeType.EQ) {
                try {
                    this.balanceBandGains = (JSON.parse(type) as number[]) ?? [];
                } catch (err) {
                    this.balanceBandGains = [];
                }
            } else if (mixerNode?.type === NodeType.FIELD) {
                this.soundFiledType = SoundFiledMap.get(parseInt(type) as SoundFiledType) ?? '-';
            } else if (mixerNode?.type === NodeType.ENV) {
                this.environmentType = EnvironmentMap.get(parseInt(type) as EnvironmentType) ?? '-';
            } else if (mixerNode?.type === NodeType.VB) {
                this.voiceBeautifierType = VoiceBeautifierMap.get(parseInt(type) as VoiceBeautifierType) ?? '-';
            } else if (mixerNode?.type === NodeType.SEPARATION) {
                this.audioSeparation = AudioSeparationMap.get(mixerNode.audioSeparationType) ?? '-';
            } else if (mixerNode?.type === NodeType.SPACE_RENDER) {
                this.spaceRenderPara = mixerNode.spaceRender ?? this.spaceRenderPara;
                if (mixerNode.spaceRender?.mode === SpaceRenderMode.FIXED_POSITION) {
                    let result = audioNapi.getFixedPositionParams(mixerNode.id);
                    Logger.info(TAG, `result is ${result.x} ${result.y} ${result.z}`);
                    this.spaceRenderPara.fixedPosition = {
                        x: result.x,
                        y: result.y,
                        z: result.z
                    }
                } else if (mixerNode.spaceRender?.mode === SpaceRenderMode.DYNAMIC_RENDER) {
                    let result = audioNapi.getDynamicRenderParams(mixerNode.id);
                    Logger.info(TAG,
                        `result is ${result.x} ${result.y} ${result.z} ${result.surroundTime} ${result.surroundDirection}`);
                    this.spaceRenderPara.dynamicPosition = {
                        x: result.x,
                        y: result.y,
                        z: result.z
                    }
                    this.spaceRenderPara.singleWeekCycleTime = result.surroundTime;
                    this.spaceRenderPara.clockWise = result.surroundDirection;
                } else if (mixerNode.spaceRender?.mode === SpaceRenderMode.EXPAND) {
                    let result = audioNapi.getExpandParams(mixerNode.id);
                    Logger.info(TAG, `result is ${result.extRadius} ${result.extAngle}`);
                    this.spaceRenderPara.expansionRadius = result.extRadius;
                    this.spaceRenderPara.expansionAngle = result.extAngle;
                }
            }
        }
    }

    build() {
        Column() {
            Text('节点信息')
                .fontSize(20)
                .fontWeight(FontWeight.Bold)
                .margin({ top: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })
            Row() {
                Column() {
                    Text(`节点类型：`)
                        .fontWeight(FontWeight.Bold)
                }

                Column() {
                    Text(`${this.nodeType}`)
                }
            }

            if (this.nodeType === NodeType.EQ) {
                Row() {
                    Text('均衡参数: ')
                        .fontWeight(FontWeight.Bold)
                    if (this.balanceBandGains.length > 0) {
                        ForEach(this.balanceBandGains, (item: number, index) => {
                            Column() {
                                Text(`${item}`)
                                    .margin(5)
                            }
                        })
                    } else {
                        Text('-')
                    }
                }
            }
            if (this.nodeType === NodeType.FIELD) {
                Row() {
                    Column() {
                        Text(`声场参数：`)
                            .fontWeight(FontWeight.Bold)
                    }

                    Column() {
                        Text(`${this.soundFiledType}`)
                    }
                }
            }
            if (this.nodeType === NodeType.ENV) {
                Row() {
                    Column() {
                        Text(`环境参数：`)
                            .fontWeight(FontWeight.Bold)
                    }

                    Column() {
                        Text(`${this.environmentType}`)
                    }
                }
            }
            if (this.nodeType === NodeType.VB) {
                Row() {
                    Column() {
                        Text(`美化参数：`)
                            .fontWeight(FontWeight.Bold)
                    }

                    Column() {
                        Text(`${this.voiceBeautifierType}`)
                    }
                }
            }
            if (this.nodeType === NodeType.SEPARATION) {
                Row() {
                    Column() {
                        Text(`音源分离类型：`)
                            .fontWeight(FontWeight.Bold)
                    }

                    Column() {
                        Text(`${this.audioSeparation}`)
                    }
                }
            }
            if (this.nodeType === NodeType.INPUT) {
                Column() {
                    Text('音频参数：')
                        .fontWeight(FontWeight.Bold)
                    Row() {
                        Text(`歌曲名称: ${this.songName}`)
                    }

                    Row() {
                        Text(`表演者: ${this.singerName ?? '-'}`)
                    }

                    Row() {
                        Text(`声道: ${this.channels}`)
                    }

                    Row() {
                        Text(`采样率: ${this.sampleRate}`)
                    }

                    Row() {
                        Text(`位深: ${this.bitsPerSample}`)
                    }
                }.padding({ top: $r('app.float.padding_20') })
            }
            if (this.nodeType === NodeType.SPACE_RENDER) {
                Column() {
                    Text('空间渲染参数：')
                        .fontWeight(FontWeight.Bold)
                    Row() {
                        Text(`模式: ${SpaceRenderModeMap.get(this.spaceRenderPara.mode)}`)
                    }

                    if (this.spaceRenderPara.mode === SpaceRenderMode.FIXED_POSITION) {
                        Row() {
                            Text(`x: ${this.spaceRenderPara.fixedPosition?.x ?? '-'}`)
                        }
                        Row() {
                            Text(`y: ${this.spaceRenderPara.fixedPosition?.y ?? '-'}`)
                        }
                        Row() {
                            Text(`z: ${this.spaceRenderPara.fixedPosition?.z ?? '-'}`)
                        }
                    } else if (this.spaceRenderPara.mode === SpaceRenderMode.DYNAMIC_RENDER) {
                        Row() {
                            Text(`x: ${this.spaceRenderPara.dynamicPosition?.x ?? '-'}`)
                        }
                        Row() {
                            Text(`y: ${this.spaceRenderPara.dynamicPosition?.y ?? '-'}`)
                        }
                        Row() {
                            Text(`z: ${this.spaceRenderPara.dynamicPosition?.z ?? '-'}`)
                        }
                        Row() {
                            Text(`单周环绕时间: ${this.spaceRenderPara.singleWeekCycleTime ?? '-'}`)
                        }
                        Row() {
                            Text(`旋转方向: ${ClockWiseMap.get(this.spaceRenderPara.clockWise) ?? '-'}`)
                        }
                    } else if (this.spaceRenderPara.mode === SpaceRenderMode.EXPAND) {
                        Row() {
                            Text(`扩展半径: ${this.spaceRenderPara.expansionRadius ?? '-'}`)
                        }
                        Row() {
                            Text(`扩展角度: ${this.spaceRenderPara.expansionAngle ?? '-'}`)
                        }
                    }
                }
            }
            Button('确定')
                .onClick(() => {
                    if (this.nodeDetailController != undefined) {
                        this.nodeDetailController.close();
                    }
                })
                .margin(20)
        }
    }
}

// "*" pop-up window
@CustomDialog
struct SetNodeDialog {
    @Prop isLeft: boolean;
    @Prop selectedNode: string;
    @Prop nodeType: NodeType;
    @Prop currentInputId: string;
    @Link mixerList: Array<Node>;
    @Link inputList: SongInfo[];
    @Link nodeList: Map<string, SongInfo>;
    // Balanced frequency band
    @State balanceBandGains: number[] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    // Sound field data, default is none.
    @State selectedSoundFiled: SoundFiledType = SoundFiledType.SOUND_FIELD_CLOSE;
    // Environmental data, default is none.
    @State selectedEnvironment: EnvironmentType = EnvironmentType.ENVIRONMENT_TYPE_CLOSE;
    // Beautify data, default is normal
    @State selectedVoiceBeautifier: VoiceBeautifierType = VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL;
    // selectedAudioSeparation
    @State selectedAudioSeparation: AudioSeparationType = AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE;
    // soundSpeed
    @State soundSpeed: number = 1.0;
    // soundTone
    @State soundTone: number = 1.0;
    // Spatial Rendering
    @State spaceRenderPara: SpaceRender = {
        mode: SpaceRenderMode.CLOSE,
        fixedPosition: {x: 0, y: 0, z: 0},
        dynamicPosition: {x: 0, y: 0, z: 0},
        singleWeekCycleTime: 2,
        clockWise: ClockWise.CLOCK_WISE,
        expansionRadius: 1,
        expansionAngle: 1
    };
    @State generalVoiceChangeType: GeneralVoiceChangeType = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE;
    @State pureVoiceGender: PureVoiceChangeGenderOption = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE;
    @State pureVoiceChangeType: PureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE;
    @State pureVoiceSoundTone: number = 0;
    setNodeController?: CustomDialogController;

    aboutToAppear(): void {
        let leftNode: SongInfo | undefined =
            this.inputList.find(node => node?.nodeId === this.selectedNode) || undefined
        if (this.isLeft) {
            let inputNode = this.nodeList.get(this.selectedNode);
            // Modification of the effect node on the left side
            if (!inputNode) {
                let leftEffectNode: Node | undefined =
                    this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id === this.selectedNode)
                Logger.info(TAG, `node env: ${leftEffectNode?.id}`)
                if (leftEffectNode) {
                    if (leftEffectNode?.balanceBandGains) {
                        this.balanceBandGains = leftEffectNode.balanceBandGains;
                    }
                    if (leftEffectNode?.soundFiledType !== undefined) {
                        this.selectedSoundFiled = leftEffectNode.soundFiledType;
                    }
                    if (leftEffectNode?.environmentType !== undefined) {
                        this.selectedEnvironment = leftEffectNode.environmentType;
                    }
                    if (leftEffectNode?.voiceBeautifierType !== undefined) {
                        this.selectedVoiceBeautifier = leftEffectNode.voiceBeautifierType;
                    }
                    if (leftEffectNode?.audioSeparationType !== undefined) {
                        this.selectedAudioSeparation = leftEffectNode.audioSeparationType
                    }
                    if (leftEffectNode?.soundSpeed !== undefined) {
                        this.soundSpeed = leftEffectNode.soundSpeed;
                    }
                    if (leftEffectNode?.soundTone !== undefined) {
                        this.soundTone = leftEffectNode.soundTone;
                    }
                    this.spaceRenderPara = leftEffectNode?.spaceRender ?? this.spaceRenderPara;
                }
            }
        } else {
            let node = this.mixerList.find(node => node.id === this.selectedNode);
            Logger.info(TAG, `node env: ${node?.id}`)
            if (node?.balanceBandGains) {
                this.balanceBandGains = node.balanceBandGains;
            }
            if (node?.soundFiledType !== undefined) {
                this.selectedSoundFiled = node.soundFiledType;
            }
            if (node?.environmentType !== undefined) {
                this.selectedEnvironment = node.environmentType;
            }
            if (node?.voiceBeautifierType !== undefined) {
                this.selectedVoiceBeautifier = node.voiceBeautifierType;
            }
            if (node?.audioSeparationType !== undefined) {
                this.selectedAudioSeparation = node.audioSeparationType;
            }
            this.spaceRenderPara = node?.spaceRender ?? this.spaceRenderPara;
        }
    }

    build() {
        Column() {
            Row() {
                if (this.nodeType === NodeType.EQ) {
                    EqualizerComponent({
                        balanceBandGains: this.balanceBandGains,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            // Modification Effect Node --- Data Processing
                            const node = this.isLeft ?
                                this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id ===
                                this.selectedNode) :
                                this.mixerList.find(node => node.id === this.selectedNode)
                            if (node) {
                                let result =
                                    audioNapi.setEqualizerFrequencyBandGains(this.balanceBandGains, this.selectedNode,
                                        this.currentInputId);
                                if (result !== 0) {
                                    Logger.error(TAG, `setEqualizerFrequencyBandGains result: ${result}`);
                                    this.getUIContext().showAlertDialog({
                                        message: '修改均衡节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                                node.balanceBandGains = this.balanceBandGains;
                            } else {
                                this.getUIContext().showAlertDialog({
                                    message: 'not find node in nodeList or mixerList',
                                    autoCancel: true
                                });
                                return;
                            }

                            this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.FIELD) {
                    FieldComponent({
                        selectedSoundFiled: this.selectedSoundFiled,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            // Modification Effect Node --- Data Processing
                            const node = this.isLeft ?
                                this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id ===
                                this.selectedNode) :
                                this.mixerList.find(node => node.id === this.selectedNode)
                            if (node) {
                                let result = audioNapi.resetFieldEffect(this.currentInputId, this.selectedSoundFiled,
                                    this.selectedNode);
                                if (result !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        message: '设置声场节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                                node.soundFiledType = this.selectedSoundFiled;
                            } else {
                                this.getUIContext().showAlertDialog({
                                    message: 'not find node in nodeList or mixerList',
                                    autoCancel: true
                                });
                                return;
                            }

                            this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.ENV) {
                    EnvironmentComponent({
                        selectedEnvironment: this.selectedEnvironment,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            // Modification Effect Node --- Data Processing
                            const node = this.isLeft ?
                                this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id ===
                                this.selectedNode) :
                                this.mixerList.find(node => node.id === this.selectedNode)
                            if (node) {
                                let result = audioNapi.resetEnvEffect(this.currentInputId, this.selectedNode,
                                    this.selectedEnvironment);
                                if (result !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        message: '设置环境节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                                node.environmentType = this.selectedEnvironment;
                            } else {
                                this.getUIContext().showAlertDialog({
                                    message: 'not find node in nodeList or mixerList',
                                    autoCancel: true
                                });
                                return;
                            }

                            this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.VB) {
                    VoiceBeautifierComponent({
                        selectedVoiceBeautifier: this.selectedVoiceBeautifier,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            // Underlying effect node for calling NAPI
                            const node = this.isLeft ?
                                this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id ===
                                this.selectedNode) :
                                this.mixerList.find(node => node.id === this.selectedNode)
                            if (node) {
                                let result =
                                    audioNapi.resetVBEffect(this.currentInputId, this.selectedVoiceBeautifier,
                                        this.selectedNode);
                                if (result !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        message: '添加美化节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                            }

                            // Modification Effect Node --- Data Processing
                            if (this.isLeft) {
                                let inputNode = this.nodeList.get(this.selectedNode);
                                // Modification of the effect node on the left side
                                if (!inputNode) {
                                    let leftEffectNode: Node | undefined =
                                        this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id ===
                                        this.selectedNode)
                                    Logger.error(TAG, `node env: ${leftEffectNode?.environmentType}`)
                                    if (leftEffectNode) {
                                        leftEffectNode.voiceBeautifierType = this.selectedVoiceBeautifier;
                                    }
                                }
                            } else {
                                const node = this.mixerList.find(node => node.id === this.selectedNode);
                                if (node?.voiceBeautifierType !== undefined) {
                                    node.voiceBeautifierType = this.selectedVoiceBeautifier;
                                }
                            }

                            this.setNodeController?.close();
                        }
                    })
                }  else if (this.nodeType === NodeType.SOUND_SPEED_TONE) {
                    SoundSpeedToneComponent({
                        soundSpeed: this.soundSpeed,
                        soundTone: this.soundTone,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                        // Modification Effect Node --- Data Processing
                        const node = this.isLeft ? this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id === this.selectedNode) :
                            this.mixerList.find(node => node.id === this.selectedNode);
                        if (node) {
                            let result =
                                audioNapi.setSoundSpeedTone(this.currentInputId, this.selectedNode, this.soundSpeed, this.soundTone);
                            if (result !== 0) {
                            // Sonic pitch effect node setup failed
                            this.getUIContext().showAlertDialog({
                                message: '修改音速音调失败',
                                autoCancel: true
                            });
                            return;
                            }
                        } else {
                            this.getUIContext().showAlertDialog({
                                message: '修改音速音调失败',
                                autoCancel: true
                            });
                        }
                        this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.SPACE_RENDER) {
                    SpaceRenderComponent({
                        spaceRenderPara: this.spaceRenderPara,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: (spaceRenderState: SpaceRender) => {
                            if (spaceRenderState.mode === -1) {
                                this.getUIContext().showAlertDialog({
                                    message: `请选择一种模式`,
                                    autoCancel: true
                                });
                                return;
                            }
                            const node = this.isLeft ?
                                this.nodeList.get(this.currentInputId)?.nodes?.find(node => node.id === this.selectedNode) :
                                this.mixerList.find(node => node.id === this.selectedNode);
                            if (node) {
                                if (spaceRenderState.mode === 0) {
                                    let result = audioNapi.resetFixedPositionEffect(spaceRenderState.fixedPosition?.x,
                                        spaceRenderState.fixedPosition?.y, spaceRenderState.fixedPosition?.z, this.selectedNode);
                                    if (result !== 0) {
                                        this.getUIContext().showAlertDialog({
                                            message: `添加空间渲染节点失败`,
                                            autoCancel: true
                                        });
                                        return;
                                    }
                                } else if (spaceRenderState.mode === 1) {
                                    let result = audioNapi.resetDynamicRenderEffect(spaceRenderState.dynamicPosition?.x,
                                        spaceRenderState.dynamicPosition?.y, spaceRenderState.dynamicPosition?.z,
                                        spaceRenderState.singleWeekCycleTime, Number(spaceRenderState.clockWise),
                                        this.selectedNode);
                                    if (result !== 0) {
                                        this.getUIContext().showAlertDialog({
                                            message: `添加空间渲染节点失败`,
                                            autoCancel: true
                                        });
                                        return;
                                    }
                                } else if (spaceRenderState.mode === 2) {
                                    let result =
                                        audioNapi.resetExpandEffect(spaceRenderState.expansionRadius,
                                        spaceRenderState.expansionAngle, this.selectedNode);
                                    if (result !== 0) {
                                        this.getUIContext().showAlertDialog({
                                            message: `添加空间渲染节点失败`,
                                            autoCancel: true
                                        });
                                        return;
                                    }
                                }
                                node.spaceRender = {
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
                            } else {
                                this.getUIContext().showAlertDialog({
                                    message: 'not find node in nodeList or mixerList',
                                    autoCancel: true
                                });
                                return;
                            }

                            this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.GENERAL_VOICE_CHANGE) {
                    GeneralVoiceChangeComponent({
                        generalVoiceChangeType: this.generalVoiceChangeType,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            const nodeId = util.generateRandomUUID(true);
                            let result = audioNapi.resetGeneralVoiceChange(this.currentInputId, this.generalVoiceChangeType, nodeId);
                            if (result !== 0) {
                                this.getUIContext().showAlertDialog({
                                    message: '设置通用变声节点失败',
                                    autoCancel: true
                                });
                                return;
                            }
                            this.setNodeController?.close();
                        }
                    })
                } else if (this.nodeType === NodeType.PURE_VOICE_CHANGE) {
                    PureVoiceChangeComponent({
                        pureVoiceGender : this.pureVoiceGender,
                        pureVoiceChangeType : this.pureVoiceChangeType,
                        soundTone : this.pureVoiceSoundTone,
                        onCancel: () => {
                            this.setNodeController?.close();
                        },
                        onAgree: () => {
                            const nodeId = util.generateRandomUUID(true);
                            let result = audioNapi.resetPureVoiceChange(this.currentInputId, nodeId, this.pureVoiceGender, this.pureVoiceSoundTone, this.pureVoiceChangeType);
                            if (result !== 0) {
                                this.getUIContext().showAlertDialog({
                                    message: '设置传统变声节点失败',
                                    autoCancel: true
                                });
                                return;
                            }
                            this.setNodeController?.close();
                        }
                    })
                }
            }
        }
    }
}

@CustomDialog
@Component
export struct CheckNodeDialog {
    // Incoming data
    // Distinguish between the left and right sections of the mixing console
    @Prop isLeft: boolean;
    // Currently selected node type
    @Prop nodeType: NodeType;
    // Currently selected node ID
    @Prop selectedNode: string;
    // Data list on the right side of the mixing console
    @Link mixerList: Array<Node>;
    // Mixing console left input node list
    @Link inputList: SongInfo[];
    // Mixing console left node list
    @Link nodeList: Map<string, SongInfo>;
    @Link isPlayRealTime: boolean;
    // Create pipeline, working mode
    @Link param: number;
    // Current audio track's inputId
    @State currentInputId: string = '';
    // Component data
    private nodes: Array<NodeType> = [];
    private select: number | number[] = 0;
    @State selectedEffectNode: NodeType = NodeType.EQ;

    aboutToAppear(): void {
        this.currentInputId = this.findInputId();
        // Get the list of effect nodes supported by the current device
        let nodeType: number[] = audioNapi.getEffectNodeList();
        this.nodes = nodeType
            .map(num => NumberToNodeTypeMap.get(num)) // Convert numbers to NodeType
            .filter(type => type !== undefined) as NodeType[]; // Filter out undefined values
    }

    effectNodeDialog: CustomDialogController | null = new CustomDialogController({
        builder: EffectNodeDialog({
            selectedNode: this.selectedNode,
            selectedEffectNode: this.selectedEffectNode,
            mixerList: this.mixerList,
            currentInputId: this.currentInputId,
            isLeft: this.isLeft,
            inputList: this.inputList,
            nodeList: this.nodeList
        }),
    })
    NodeDetailDialogController: CustomDialogController | null = new CustomDialogController({
        builder: NodeDetailDialog({
            mixerList: this.mixerList,
            isLeft: this.isLeft,
            selectedNode: this.selectedNode,
            inputList: this.inputList,
            nodeList: this.nodeList,
            nodeType: this.nodeType,
            currentInputId: this.currentInputId
        })
    })
    SetNodeDialogController: CustomDialogController | null = new CustomDialogController({
        builder: SetNodeDialog({
            mixerList: this.mixerList,
            isLeft: this.isLeft,
            selectedNode: this.selectedNode,
            inputList: this.inputList,
            nodeList: this.nodeList,
            nodeType: this.nodeType,
            currentInputId: this.currentInputId
        })
    })
    controller?: CustomDialogController;
    // If you attempt to pass multiple other controllers into the CustomDialog to open another
    // or some other CustomDialogs within the CustomDialog,
    // then you need to place the controller that points to itself at the end of all the controllers.
    cancel: () => void = () => {
    }
    confirm: () => void = () => {
    }

    private getIconResource(icon: string): Resource {
        const map: Record<string, Resource> = {
            '+': $r('app.media.plus'),
            '-': $r('app.media.minus_circle'),
            '?': $r('app.media.questionmark_circle'),
            '*': $r('app.media.star')
        };
        return map[icon];
    }

    findInputId(): string {
        for (const entry of this.nodeList.entries()) {
            const key = entry[0];
            const songInfo = entry[1];

            const foundNode = songInfo.nodes?.find((node) => node.id === this.selectedNode);
            if (foundNode) {
                return key;
            } else if (this.inputList.find((node) => node?.nodeId === this.selectedNode)) {
                Logger.error(TAG, `this selected is input node : ${this.selectedNode}, inputId is ${this.selectedNode}`)
                return this.selectedNode;
            }
        }
        return '';
    }

    // "+" pop-up window
    selectNodeHandle() {
        // Filter optional node types based on param
        let filteredNodes: NodeType[] = this.nodes;
        // Default: Show All (compatible with old logic)
        filteredNodes = this.nodes;
        this.getUIContext().showTextPickerDialog({
            range: filteredNodes,
            selected: this.select,
            value: this.selectedEffectNode,
            defaultPickerItemHeight: 40,
            onAccept: (value: TextPickerResult) => {
                Logger.info(TAG, `selectNodeHandle onAccept, value: ${JSON.stringify(value)}`);
                // Set `select` to the index of the selected item when the confirm button is pressed,
                // so that when the pop-up appears again, it displays the option that was previously confirmed.
                this.select = value.index;
                // After clicking OK, the selected text data is displayed on the page
                this.selectedEffectNode = value.value as NodeType;

                this.effectNodeDialog?.open();
            },
            onCancel: () => {
                Logger.info(TAG, 'selectNodeHandle onCancel');
            },
            onChange: () => {
                Logger.info(TAG, 'selectNodeHandle onChange');
            }
        })
    }

    // "-" pop-up window
    deleteNodeHandle() {
        this.getUIContext().showAlertDialog(
            {
                title: '确认删除该节点',
                message: '',
                autoCancel: true,
                alignment: DialogAlignment.Bottom,
                gridCount: 4,
                offset: { dx: 0, dy: -20 },
                primaryButton: {
                    value: '取消',
                    action: () => {
                        console.info('Callback when the first button is clicked');
                    }
                },
                secondaryButton: {
                    enabled: true,
                    defaultFocus: true,
                    style: DialogButtonStyle.HIGHLIGHT,
                    value: '确定',
                    action: () => {
                        if (this.isLeft) {
                            if (this.inputList.find((item: SongInfo) => item?.nodeId === this.selectedNode)) {
                                let result = audioNapi.deleteSong(this.selectedNode);
                                if (result !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        message: '删除节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                                // If you click the "-" button on the input node, both inputList and nodeList need to be processed
                                this.nodeList.delete(this.selectedNode);
                                const index = this.inputList.findIndex(item => item?.nodeId === this.selectedNode);
                                if (index !== -1) {
                                    this.inputList[index] = null;
                                }
                            } else {
                                const inputId = this.findInputId();
                                let songInfo = this.nodeList.get(inputId);
                                if (songInfo) {
                                    // Delete the corresponding node
                                    let index = songInfo.nodes?.findIndex(node => node.id === this.selectedNode);
                                    if (index !== -1) {
                                        let result = audioNapi.deleteNode(this.selectedNode);
                                        if (result !== 0) {
                                            this.getUIContext().showAlertDialog({
                                                message: '删除节点失败',
                                                autoCancel: true
                                            });
                                            return;
                                        }
                                        songInfo.nodes?.splice(index, 1);
                                        this.nodeList.set(inputId, songInfo);
                                    } else {
                                        this.getUIContext().showAlertDialog({
                                            message: 'deleteNodeHandle nodeList is not selectedNodeId',
                                            autoCancel: true
                                        });
                                    }

                                    this.controller?.close();
                                }
                            }
                        } else {
                            Logger.info(TAG, `deleteNodeHandle selectedNode: ${this.selectedNode}`);
                            const index = this.mixerList.findIndex(node => node.id === this.selectedNode);
                            if (index !== -1) {
                                let result = audioNapi.deleteNode(this.selectedNode);
                                if (result !== 0) {
                                    this.getUIContext().showAlertDialog({
                                        message: '删除节点失败',
                                        autoCancel: true
                                    });
                                    return;
                                }
                                this.mixerList.splice(index, 1);
                                this.controller?.close();
                            } else {
                                this.getUIContext().showAlertDialog({
                                    message: 'deleteNodeHandle mixerList is not has selectedNodeId',
                                    autoCancel: true
                                });
                            }
                        }
                    }
                }
            }
        )
    }

    build() {
        Column() {
            Text('').fontSize(20).margin({ top: 10, bottom: 10 })
            // Text('Whether to change a text?').fontSize(16).margin({ bottom: 10 })
            Flex({ justifyContent: FlexAlign.SpaceAround }) {
                if (this.nodeType !== NodeType.OUTPUT && !this.isPlayRealTime &&
                    this.nodeType !== NodeType.SEPARATION) {
                    Image(this.getIconResource('+'))
                        .width('50')
                        .height('50')
                        .onClick(() => {
                            this.selectNodeHandle();
                        })
                }
                if (this.nodeType !== NodeType.OUTPUT && this.nodeType !== NodeType.MIXER) {
                    Image(this.getIconResource('-'))
                        .width('50')
                        .height('50')
                        .onClick(() => {
                            this.deleteNodeHandle();
                            this.controller?.close();
                        })
                }
                Image(this.getIconResource('?'))
                    .width('50')
                    .height('50')
                    .onClick(() => {
                        if (this.NodeDetailDialogController !== null) {
                            this.NodeDetailDialogController.open();
                        }
                    })
                if (this.nodeType !== NodeType.INPUT && this.nodeType !== NodeType.MIXER &&
                    this.nodeType !== NodeType.OUTPUT &&
                    this.nodeType !== NodeType.NR) {
                    Image(this.getIconResource('*'))
                        .width('50')
                        .height('50')
                        .onClick(() => {
                            if (this.SetNodeDialogController !== null) {
                                this.SetNodeDialogController.open();
                            }
                        })
                }
            }.margin({ bottom: 10 })
        }.borderRadius(10)
        // If you need to use the border property or cornerRadius property, use them together with the borderRadius property
    }
}