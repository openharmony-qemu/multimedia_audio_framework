/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

export interface SongInfo {
  uri: string;
  songName?: string | Resource;
  songType?: string;
  nodes?: Array<Node>;
  nodeId?: string;
  duration?: number;
  singerName?: string | Resource;
  albumCover?: PixelMap;
  albumName?: string | Resource;
  wavBuffer?: ArrayBuffer;
  channels?: number;
  sampleRate?: number;
  bitsPerSample?: number;
  isPlay?: boolean;
  fd?: number;
}

export interface Node {
  id: string;
  type: NodeType;
  color?: BackColor;
  isByPass?: boolean;
  balanceBandGains?: number[];
  soundFiledType?: SoundFiledType;
  environmentType?: EnvironmentType;
  voiceBeautifierType?: VoiceBeautifierType;
  audioSeparationType?: AudioSeparationType;
  soundSpeed?: number;
  soundTone?: number;
  spaceRender?: SpaceRender;
  voiceChange?: VoiceChange;
}

export interface  OutSong {
  pipelineId: string,
  channels?: number;
  sampleRate?: number;
  bitsPerSample?: number;
  pipelineCascadMode: number;
  buffer?: ArrayBuffer,
  bufferLength?: number,
  isRenderer?: boolean,
  isRendererFinished?: boolean,
}

// Define Interface
export interface NavigationParams {
  mode: number;
  pipelineCascadMode: number;
  pipelineId?: string;
}

export enum BackColor {
  FIELD_COLOR = '#FFC107',
  SEPARATION_COLOR = '#4ECDC4',
  NR_COLOR = '#FF6F61',
  EQ_COLOR = '#ffc46e98',
  VB_COLOR = '#2A5C9B',
  MIXER_COLOR = '#9869E0',
  INPUT_COLOR = '#9869E0',
  OUTPUT_COLOR = '#9869E0',
  ENV_COLOR = '#D3A26F',
  SOUND_SPEED_TONE = '#4A90E2',
  SPACE_RENDER_COLOR = '#8BC34A',
  GENERAL_VOICE_CHANGE = '#005B96',
  PURE_VOICE_CHANGE = '#FFD700',
  VOICE_CHANGE = '#FFD700',
  EFFECT = '#00BFFF'
}

export interface AudioTrack {
  audioTrackId: string;
  //是否静音
  isSilent: boolean;
  //是否独奏
  isSolo: boolean;
  audioAssetArray?: Array<AudioAsset>;
}

export interface AudioAsset {
  assetId: number;
  uri: string;
  songName?: string | Resource;
  songType?: string;
  //音频资源上的效果节点
  nodes?: Array<Node>;
  singerName?: string | Resource;
  albumCover?: PixelMap;
  albumName?: string | Resource;
  //wav音频文件buffer
  wavBuffer?: ArrayBuffer;
  //跳过wav头的pcmbuffer
  pcmBuffer?: ArrayBuffer;
  //通道数
  channels: number;
  //采样率
  sampleRate: number;
  //位深
  bitsPerSample: number;
  //音频文件fd
  fd?: number;
  //音频开始时间
  startTime?: number;
  //音频时长
  duration?: number | undefined;
}

export enum NodeType {
  'VALID' = '无效',
  'EQ' = '均衡',
  'FIELD' = '声场',
  'ENV' = '环境',
  'SEPARATION' = '分离',
  'VOICE' = '人声',
  'ACCOMPANIMENT' = '伴奏',
  'NR' = '降噪',
  'INPUT' = '输入',
  'OUTPUT' = '输出',
  'MIXER' = '混音',
  'VB' = '美化',
  'GENERAL_VOICE_CHANGE' = '通用变声',
  'PURE_VOICE_CHANGE' = '传统变声',
  'VOICE_CHANGE' = '变声',
  'EFFECT' = '效果',
  'SPACE_RENDER' = '3D',
  'SOUND_SPEED_TONE' = '音速音调'
}

export const InputStringToNodeTypeMap:Map<string, NodeType> = new Map([
  ["INPUT", NodeType.INPUT],
  ["OUTPUT", NodeType.OUTPUT],
  ["AISS", NodeType.SEPARATION],
  ["NR", NodeType.NR],
  ["VB", NodeType.VB],
  ["MIXER", NodeType.MIXER],
  ["ENV", NodeType.ENV],
  ["EQ", NodeType.EQ],
  ["FIELD", NodeType.FIELD],
  ["SOUND_SPEED_TONE", NodeType.SOUND_SPEED_TONE],
  ["SPACE_RENDER", NodeType.SPACE_RENDER],
  ["GENERAL_VOICE_CHANGE", NodeType.GENERAL_VOICE_CHANGE],
  ["PURE_VOICE_CHANGE", NodeType.PURE_VOICE_CHANGE],
]);

export const ColorMap = new Map<NodeType, BackColor>([
  [NodeType.EQ, BackColor.EQ_COLOR],
  [NodeType.FIELD, BackColor.FIELD_COLOR],
  [NodeType.VOICE, BackColor.SEPARATION_COLOR],
  [NodeType.ACCOMPANIMENT, BackColor.SEPARATION_COLOR],
  [NodeType.NR, BackColor.NR_COLOR],
  [NodeType.VB, BackColor.VB_COLOR],
  [NodeType.MIXER, BackColor.MIXER_COLOR],
  [NodeType.INPUT, BackColor.INPUT_COLOR],
  [NodeType.OUTPUT, BackColor.OUTPUT_COLOR],
  [NodeType.ENV, BackColor.ENV_COLOR],
  [NodeType.SEPARATION, BackColor.SEPARATION_COLOR],
  [NodeType.SOUND_SPEED_TONE, BackColor.SOUND_SPEED_TONE],
  [NodeType.SPACE_RENDER, BackColor.SPACE_RENDER_COLOR],
  [NodeType.GENERAL_VOICE_CHANGE, BackColor.GENERAL_VOICE_CHANGE],
  [NodeType.PURE_VOICE_CHANGE, BackColor.PURE_VOICE_CHANGE],
  [NodeType.VOICE_CHANGE, BackColor.VOICE_CHANGE],
  [NodeType.EFFECT, BackColor.EFFECT]
])

export enum SoundFiledType {
  /**
   * None
   */
  SOUND_FIELD_CLOSE = 0,
  /**
   * Front
   */
  SOUND_FIELD_FRONT_FACING = 1,
  /**
   * Grandeur
   */
  SOUND_FIELD_GRAND = 2,
  /**
   * Listen
   */
  SOUND_FIELD_NEAR = 3,
  /**
   * Broad
   */
  SOUND_FIELD_WIDE = 4,
}

export enum GeneralVoiceChangeType {
  GENERAL_VOICE_CHANGE_TYPE_CLOSE = 0,
  GENERAL_VOICE_CHANGE_TYPE_CUTE = 1,
  GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK = 2,
  GENERAL_VOICE_CHANGE_TYPE_FEMALE = 3,
  GENERAL_VOICE_CHANGE_TYPE_MALE = 4,
  GENERAL_VOICE_CHANGE_TYPE_MIX = 5,
  GENERAL_VOICE_CHANGE_TYPE_MONSTER = 6,
  GENERAL_VOICE_CHANGE_TYPE_SEASONED = 7,
  GENERAL_VOICE_CHANGE_TYPE_SYNTH = 8,
  GENERAL_VOICE_CHANGE_TYPE_TRILL = 9,
  GENERAL_VOICE_CHANGE_TYPE_WAR = 10,
}
export  enum PureVoiceChangeGenderOption{
  PURE_VOICE_CHANGE_CLOSE = 0,
  PURE_VOICE_CHANGE_FEMALE = 1,
  PURE_VOICE_CHANGE_MALE = 2,
}


export  enum PureVoiceChangeVocalPartOption{
  PURE_VOICE_CHANGE_VOCAL_PART_CLOSE = 0,
  PURE_VOICE_CHANGE_VOCAL_PART_MIDDLE = 1,
}

export  enum PureVoiceChangeType{
  PURE_VOICE_CHANGE_TYPE_CLOSE = 0,
  PURE_VOICE_CHANGE_TYPE_CARTOON = 1,
  PURE_VOICE_CHANGE_TYPE_CUTE = 2,
  PURE_VOICE_CHANGE_TYPE_FEMALE = 3,
  PURE_VOICE_CHANGE_TYPE_MALE = 4,
  PURE_VOICE_CHANGE_TYPE_MONSTER = 5,
  PURE_VOICE_CHANGE_TYPE_ROBOTS = 6,
  PURE_VOICE_CHANGE_TYPE_SEASONED = 7,
}

export const SoundFiledMap = new Map<SoundFiledType | undefined, string>([
  [SoundFiledType.SOUND_FIELD_CLOSE, '无'],
  [SoundFiledType.SOUND_FIELD_FRONT_FACING, '前置'],
  [SoundFiledType.SOUND_FIELD_GRAND, '宏大'],
  [SoundFiledType.SOUND_FIELD_NEAR, '聆听'],
  [SoundFiledType.SOUND_FIELD_WIDE, '宽广']
])

export enum EnvironmentType {
  /**
   * Close environment effect type.
   *
   * @since 22
   */
  ENVIRONMENT_TYPE_CLOSE = 0,
  /**
   * Broadcast environment effect type.
   *
   * @since 22
   */
  ENVIRONMENT_TYPE_BROADCAST = 1,
  /**
   * Earpiece environment effect type.
   *
   * @since 22
   */
  ENVIRONMENT_TYPE_EARPIECE = 2,
  /**
   * Earpiece environment effect type.
   *
   * @since 22
   */
  ENVIRONMENT_TYPE_UNDERWATER = 3,
  /**
   * Gramophone environment effect type.
   *
   * @since 22
   */
  ENVIRONMENT_TYPE_GRAMOPHONE = 4,
}

export const EnvironmentMap = new Map<EnvironmentType | undefined, string>([
  [EnvironmentType.ENVIRONMENT_TYPE_CLOSE, '无'],
  [EnvironmentType.ENVIRONMENT_TYPE_BROADCAST, '广播'],
  [EnvironmentType.ENVIRONMENT_TYPE_EARPIECE, '听筒'],
  [EnvironmentType.ENVIRONMENT_TYPE_UNDERWATER, '水下'],
  [EnvironmentType.ENVIRONMENT_TYPE_GRAMOPHONE, '留声机']
])

export enum VoiceBeautifierType {
  /**
   * Normal voice beautifier type.
   *
   * @since 22
   */
  VOICE_BEAUTIFIER_TYPE_NORMAL = 0,
  /**
   * Clear voice beautifier type.
   *
   * @since 22
   */
  VOICE_BEAUTIFIER_TYPE_CLEAR,
  /**
   * Theatre voice beautifier type.
   *
   * @since 22
   */
  VOICE_BEAUTIFIER_TYPE_THEATRE,
  /**
   * CD voice beautifier type.
   *
   * @since 22
   */
  VOICE_BEAUTIFIER_TYPE_CD,
  /**
   * Studio voice beautifier type.
   *
   * @since 22
   */
  VOICE_BEAUTIFIER_TYPE_STUDIO,
}

export const VoiceBeautifierMap = new Map<VoiceBeautifierType | undefined, string>([
  [VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_NORMAL, '普通'],
  [VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CLEAR, '清澈'],
  [VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_THEATRE, '剧场'],
  [VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_CD, 'CD音效'],
  [VoiceBeautifierType.VOICE_BEAUTIFIER_TYPE_STUDIO, '录音棚'],
])

export enum AudioSeparationType {
  /**
   * @background sound type.
   *
   * @since 22
   */
  AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE = 0,
  /**
   * @human sound type.
   *
   * @since 22
   */
  AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE = 1,
}

export const AudioSeparationMap = new Map<AudioSeparationType | undefined, string>([
  [AudioSeparationType.AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE, '人声'],
  [AudioSeparationType.AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE, '伴奏']
])

// Define mapping table: numbers -> NodeType
export const NumberToNodeTypeMap = new Map<number, NodeType>([
  [201, NodeType.EQ],
  [202, NodeType.NR],
  [203, NodeType.FIELD],
  [204, NodeType.SEPARATION],
  [205, NodeType.VB],
  [206, NodeType.ENV],
  [208, NodeType.SPACE_RENDER],
  [209, NodeType.PURE_VOICE_CHANGE],
  [210, NodeType.GENERAL_VOICE_CHANGE],
  [211, NodeType.SOUND_SPEED_TONE]
]);

export interface SpaceRender {
  mode: SpaceRenderMode;
  fixedPosition?: SpacePosition;
  dynamicPosition?: SpacePosition;
  singleWeekCycleTime?: number;
  clockWise?: ClockWise;
  expansionRadius?: number;
  expansionAngle?: number;
}

export interface SpacePosition {
  x: number;
  y: number;
  z: number;
}

export enum SpaceRenderMode {
  CLOSE = -1,
  FIXED_POSITION = 0,
  DYNAMIC_RENDER = 1,
  EXPAND = 2
}

export const SpaceRenderModeMap = new Map<SpaceRenderMode, string>([
  [SpaceRenderMode.CLOSE, '无模式'],
  [SpaceRenderMode.FIXED_POSITION, '固定摆位'],
  [SpaceRenderMode.DYNAMIC_RENDER, '动态渲染'],
  [SpaceRenderMode.EXPAND, '扩展']
])

export enum ClockWise {
  CLOCK_WISE = 0,
  Counter_Clock_Wise = 1
}

export const ClockWiseMap = new Map<ClockWise, string>([
  [ClockWise.CLOCK_WISE, '顺时针'],
  [ClockWise.Counter_Clock_Wise, '逆时针']
])

export const OFFLINEPIPELINE = 1;

export const REALTIMEPIPELINE = 2;

export const PIPELINECASCADMODE_0 = 0;

export const PIPELINECASCADMODE_1 = 1;

export const PIPELINECASCADMODE_2 = 2;

export const PIPELINECASCADMODE_3 = 3;

export const outputNodeSongNameMap = new Map<number, string>([
  [PIPELINECASCADMODE_1, 'OUTPUT 1'],
  [PIPELINECASCADMODE_2, 'OUTPUT 2'],
  [PIPELINECASCADMODE_3, 'OUTPUT 3'],
]);

// Bit-depth type
export enum BitsPerSampleMode {
  INT = 0,
  FLOAT = 1,
}

// 音轨编辑
export const AudioWaveColorMap: Record<string, string> = {
  'MUSIC_1': '#B86E43',
  'MUSIC_2': '#AB8336',
  'MUSIC_3': '#638F32',
  'MUSIC_4': '#376C90',
  'MUSIC_5': '#378F88',
  'MUSIC_AUDIO_WAVES_1': '#f7d0b9',
  'MUSIC_AUDIO_WAVES_2': '#eed9ae',
  'MUSIC_AUDIO_WAVES_3': '#d1edb2',
  'MUSIC_AUDIO_WAVES_4': '#afd3ec',
  'MUSIC_AUDIO_WAVES_5': '#a4ede7',
};

export enum RecordParamDefault {
  sampleRate = 44100,
  channels = 2,
  bitsPerSample = 32
}

// 变声参数
export interface VoiceChange {
  type: NodeType.GENERAL_VOICE_CHANGE | NodeType.PURE_VOICE_CHANGE;
  generalVoiceChange?: GeneralVoiceChangeType;
  pureVoiceChangeGender?: PureVoiceChangeGenderOption;
  pureVoiceChangeVocalPart?: PureVoiceChangeVocalPartOption;
  pureVoiceChangeType?: PureVoiceChangeType;
  pureVoiceSoundTone?: number;
}

//枚举类型均衡器频带值
export const balanceBandGainsMap = new Map<number, number[]>([
  [1, [3, 5, 2, -4, 1, 2, -3, 1, 4, 5]],
  [2, [0, 0, 2, 0, 0, 4, 4, 2, 2, 5]],
  [3, [2, 3, 2, 1, 0, 0, -5, -5, -5, -6]],
  [4, [4, 3, 2, -3, 0, 0, 5, 4, 2, 0]],
  [5, [2, 0, 2, 3, 6, 5, -1, 3, 4, 4]],
  [6, [5, 2, 1, -1, -5, -5, -2, 1, 2, 4]],
  [7, [1, 4, 5, 3, -2, -2, 2, 3, 5, 5]],
  [8, [6, 4, 4, 2, 0, 1, 3, 3, 5, 4]]
]);