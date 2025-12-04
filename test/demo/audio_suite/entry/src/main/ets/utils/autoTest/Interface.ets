/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
export interface InputFile {
  inputId:string;
  path:string;
  inputPathId:string;
  sampleRate:number;
  bitDepth:number;
  channels:number;
  format?:string;
  effect:NodeInfo[];
  effectStr:string[];
};

export interface ExpectedOutput {
  md5Value:string[];
};

export interface PipelineInfo {
  pipelineId: string;
  inputIdList: string[];
  outputId: string;
  mixerId: string;
  multiRenderFrameFlag: boolean;
  sceneInfoId: string;
  sceneInfoName: string;
}

export interface OutputFile {
  nameTemplate:string;
  format:string;
  sampleRate:number;
  bitDepth:number;
  channels:number;
  outputFileList?:string[];
};

export interface ExecuteReport {
  sceneInfoId:string;
  sceneName:string;
  startTime:string;
  startTimeStamp:number;
  endTime:string;
  endTimeStamp:number;
  costTime:number;
  result:boolean;
  exeReportPath:string;
}

export interface NodeInfo{
  nodeInfoId:string;
  type:string;
  mode?:number;
  gains?:number[];
  format?:string;
  sampleRate?:number;
  bitDepth?:number;
  channels?:number;
}

export interface  SceneInfo {
  sceneInfoId:string;
  sceneName:string;
  description:string;
  inputFiles:InputFile[];
  mixerInfo:NodeInfo[];
  expectedOutput:ExpectedOutput;
  outputFile:OutputFile;
  pipelineId: string;
};

export interface  JsonFileInfo {
  jsonInfoId:string;
  filePath:string;
  summary:string;
  realTime:boolean;
  sceneInfos:string[];
};

export interface DisplayPipelineInfo {
  inputNodeIdList: string[];
  effectChain: Map<string, string[]>;
  mixerChain: string[];
}