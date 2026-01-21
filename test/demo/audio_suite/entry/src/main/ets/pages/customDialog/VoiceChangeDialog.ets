/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import {
  NodeType,
  VoiceChange,
  GeneralVoiceChangeType,
  PureVoiceChangeGenderOption,
  PureVoiceChangeType
} from "../../utils/InterfaceInfo";
import { StyleConst } from '../../utils/StyleConst'

const TAG: string = 'AudioEditTestApp_VoiceChangeDialog';

@CustomDialog
struct GeneralVoiceChangeDialog {
  private scrollerIndex: Scroller = new Scroller();
  @Link voiceChange: VoiceChange;
  @State voiceChangeComponent: VoiceChange = {
    type: NodeType.GENERAL_VOICE_CHANGE,
    generalVoiceChange: this.voiceChange.generalVoiceChange,
    pureVoiceChangeGender: this.voiceChange.pureVoiceChangeGender,
    pureVoiceChangeVocalPart: this.voiceChange.pureVoiceChangeVocalPart,
    pureVoiceChangeType: this.voiceChange.pureVoiceChangeType,
    pureVoiceSoundTone: this.voiceChange.pureVoiceSoundTone
  };
  onAgree?: (voiceChangePara: VoiceChange, controllerTwo: CustomDialogController) => void;

  controllerTwo: CustomDialogController;

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
                  .checked(this.voiceChangeComponent.generalVoiceChange === GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CLOSE
                  })
                Text('无').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'cute', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CUTE)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CUTE
                  })
                Text('可爱').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'cyberpunk', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK
                  })
                Text('赛博朋克').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'female', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_FEMALE)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_FEMALE
                  })
                Text('女声').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'male', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MALE)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MALE
                  })
                Text('男声').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'mix', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MIX)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MIX
                  })
                Text('混响').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'monster', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MONSTER)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_MONSTER
                  })
                Text('怪物').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'seasoned', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SEASONED)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SEASONED
                  })
                Text('沉稳').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'synth', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SYNTH)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_SYNTH
                  })
                Text('合成器').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'trill', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_TRILL)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_TRILL
                  })
                Text('颤音').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'war', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.generalVoiceChange == GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_WAR)
                  .onChange(() => {
                    this.voiceChangeComponent.generalVoiceChange = GeneralVoiceChangeType.GENERAL_VOICE_CHANGE_TYPE_WAR
                  })
                Text('战争').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }
          }.rowsTemplate('1fr 1fr').height('80%')
        }
        .margin({ right: $r('app.float.margin_20') })
        .padding({ left: '10%', right: '10%' })

        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
          Button('取消')
            .onClick(() => {
              this.controllerTwo?.close();
            })
          Button('确定')
            .onClick(() => {
              if (this.onAgree) {
                this.onAgree(this.voiceChangeComponent, this.controllerTwo);
              }
            })
        }.margin({ top: $r('app.float.margin_10') })
        .padding($r('app.float.padding_20'))
      }
    }
  }
}

@CustomDialog
struct PureVoiceChangeDialog {
  private scrollerIndex: Scroller = new Scroller();
  @Link voiceChange: VoiceChange;
  @State voiceChangeComponent: VoiceChange = {
    type: NodeType.PURE_VOICE_CHANGE,
    generalVoiceChange: this.voiceChange.generalVoiceChange,
    pureVoiceChangeGender: this.voiceChange.pureVoiceChangeGender,
    pureVoiceChangeVocalPart: this.voiceChange.pureVoiceChangeVocalPart,
    pureVoiceChangeType: this.voiceChange.pureVoiceChangeType,
    pureVoiceSoundTone: this.voiceChange.pureVoiceSoundTone
  };
  onAgree?: (voiceChangePara: VoiceChange, controllerTwo: CustomDialogController) => void;
  @State needSoundTone: boolean = false;

  controllerTwo: CustomDialogController;

  build() {
    Scroll(this.scrollerIndex) {
      Column() {
        Row() {
          Text(`${NodeType.PURE_VOICE_CHANGE}`)
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
                  .checked(this.voiceChangeComponent.pureVoiceChangeGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_CLOSE
                  })
                Text('无').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'close', group: 'PureVoiceGender' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_FEMALE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_FEMALE
                  })
                Text('女性').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'close', group: 'PureVoiceGender' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeGender == PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_MALE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeGender = PureVoiceChangeGenderOption.PURE_VOICE_CHANGE_MALE
                  })
                Text('男性').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }
          }

          Row() {
            Text("变声类型")
          }

          Grid() {
            GridItem() {
              Column() {
                Radio({ value: 'close', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CLOSE
                  })
                Text('无').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'cute', group: 'GeneralVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CARTOON)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CARTOON
                  })
                Text('卡通').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'cute', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CUTE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_CUTE
                  })
                Text('可爱').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'female', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_FEMALE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_FEMALE
                  })
                Text('女声').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'male', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MALE)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MALE
                  })
                Text('男声').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'monster', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MONSTER)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_MONSTER
                  })
                Text('怪物').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'robot', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_ROBOTS)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_ROBOTS
                  })
                Text('机器人').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }

            GridItem() {
              Column() {
                Radio({ value: 'seasoned', group: 'PureVoiceChangeType' })
                  .checked(this.voiceChangeComponent.pureVoiceChangeType == PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_SEASONED)
                  .onChange(() => {
                    this.voiceChangeComponent.pureVoiceChangeType = PureVoiceChangeType.PURE_VOICE_CHANGE_TYPE_SEASONED
                  })
                Text('沉稳').fontSize($r('app.float.font_size_20'))
              }.width($r('app.float.width_80'))
            }
          }.rowsTemplate('1fr 1fr').height(StyleConst.EIGHTY_PERCENT_HEIGHT)
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
                    this.voiceChangeComponent.pureVoiceSoundTone = 0;
                  }
                })
              Text(`+${this.voiceChangeComponent.pureVoiceSoundTone?.toFixed(1)}`)
            }.padding($r('app.float.padding_10'))
            if (this.needSoundTone) {
              Slider({
                value: this.voiceChangeComponent.pureVoiceSoundTone,
                step: 0.1,
                style: SliderStyle.OutSet,
                min: 0.3,
                max: 3,
              })
                .selectedColor($r('app.color.index_audio_clip_card'))
                .onChange((value: number) => {
                  this.voiceChangeComponent.pureVoiceSoundTone = value;
                })
            }
          }
          .height($r('app.float.height_50'))
          .padding($r('app.float.padding_10'))
          .backgroundColor($r('app.color.sound_speed_tone_backgroundColor'))
          .borderRadius($r('app.float.borderRadius_10'))
        }
        .margin({ right: $r('app.float.margin_20') })
        .padding({ left: '10%', right: '10%' })

        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
          Button('取消')
            .onClick(() => {
              this.controllerTwo?.close();
            })
          Button('确定')
            .onClick(() => {
              if (this.onAgree) {
                this.onAgree(this.voiceChangeComponent, this.controllerTwo);
              }
            })
        }.margin({ top: $r('app.float.margin_10') })
        .padding($r('app.float.padding_20'))
      }
    }
  }
}

@CustomDialog
@Component
export struct VoiceChangeDialog {
  @Prop voiceChangePara: VoiceChange;

  onAgree?: (voiceChangePara: VoiceChange) => void;

  controller?: CustomDialogController;

  // Universal Voice Change Pop-up Window
  generalVoiceChangeDialogController: CustomDialogController | null = new CustomDialogController({
    builder: GeneralVoiceChangeDialog({
      voiceChange: this.voiceChangePara,
      onAgree: (voiceChangePara: VoiceChange, controllerTwo: CustomDialogController) => {
        if (this.onAgree) {
          this.onAgree(voiceChangePara);
        }
        controllerTwo.close();
        this.controller?.close();
      }
    }),
  });

  // Traditional Voice Change Pop-up Window
  pureVoiceChangeDialogController: CustomDialogController | null = new CustomDialogController({
    builder: PureVoiceChangeDialog({
      voiceChange: this.voiceChangePara,
      onAgree: (voiceChangePara: VoiceChange, controllerTwo: CustomDialogController) => {
        if (this.onAgree) {
          this.onAgree(voiceChangePara);
        }
        controllerTwo.close();
        this.controller?.close();
      }
    }),
  });

  build() {
    Column() {
      Text(`${NodeType.VOICE_CHANGE}`)
        .margin({ bottom: $r('app.float.margin_10') })
      Row() {
        Column() {
          Radio({ value: 'front', group: 'fieldEffect' })
            .checked(this.voiceChangePara.type === NodeType.GENERAL_VOICE_CHANGE)
            .onChange(() => {
              this.voiceChangePara.type = NodeType.GENERAL_VOICE_CHANGE;
            })
          Text(NodeType.GENERAL_VOICE_CHANGE).fontSize(20)
        }
        .width(StyleConst.FIFTY_PERCENT_WIDTH)

        Column() {
          Radio({ value: 'grand', group: 'fieldEffect' })
            .checked(this.voiceChangePara.type === NodeType.PURE_VOICE_CHANGE)
            .onChange(() => {
              this.voiceChangePara.type = NodeType.PURE_VOICE_CHANGE;
            })
          Text(NodeType.PURE_VOICE_CHANGE).fontSize(20)
        }
        .width(StyleConst.FIFTY_PERCENT_WIDTH)
      }

      Flex({
        direction: FlexDirection.Row,
        justifyContent: FlexAlign.SpaceBetween,
        alignItems: ItemAlign.Center
      }) {
        Button('取消')
          .onClick(() => {
            this.controller?.close();
          })
        Button('确定')
          .onClick(() => {
            if (this.voiceChangePara.type === NodeType.GENERAL_VOICE_CHANGE) {
              this.generalVoiceChangeDialogController?.open();
            } else {
              this.pureVoiceChangeDialogController?.open();
            }
          })
      }.margin({ top: $r('app.float.margin_10') })
    }
    .padding($r('app.float.padding_10'))
  }
}