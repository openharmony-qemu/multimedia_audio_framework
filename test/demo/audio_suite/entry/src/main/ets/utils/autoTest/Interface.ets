/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
export interface InputFile {
    path: string;
    sampleRate: number;
    bitDepth: number;
    channels: number;
    format?: string;
    effect: NodeInfo[];
}

export interface ExpectedOutput {
    targetFile: string[];
    format?: string;
    sampleRate?: number;
    bit_depth?: number;
    channels?: number;
    
}

export interface OutputFile {
    nameTemplate: string[];
    format: string;
    sampleRate: number;
    bitDepth: number;
    channels: number;
}

export interface ExecuteReport {
    caseName: string;
    startTime: string;
    startTimeStamp: number;
    endTime: string;
    endTimeStamp: number;
    inputFileReport: string;
    effectProcess: string;
    outFileReport: string;
    costTime: number;
    result: boolean;

}

export interface NodeInfo {
    type: string;
    mode?: number;
    gains: number[];
}

export interface CaseInfo {
    filePath: string;
    sceneName: string;
    description: string;
    inputFiles: InputFile[];
    mixerInfo: object[];
    expectedOutput: ExpectedOutput;
    outputFile: OutputFile;
}

export enum BackColor {
    FIELD_COLOR = '#ffee0b80',
    SEPARATION_COLOR = '#ffae05f6',
    NR_COLOR = '#ff0b71ee',
    EQ_COLOR = '#ffeea20b',
    VB_COLOR = '#ff0f740b',
    MIXER_COLOR = '#ff0d27cf',
    INPUT_COLOR = '#ff1eac35',
    OUTPUT_COLOR = '#ff20c61a',
    ENV_COLOR = '#ffe2ad79'
}

export enum NodeType {
    'VALID' = '无效',
    'EQ' = '均衡',
    'FIELD' = '声场',
    'ENV' = '环境',
    'AISS' = '分离',
    'NR' = '降噪',
    'INPUT' = '输入',
    'OUTPUT' = '输出',
    'MIXER' = '混音',
    'VB' = '美化'
}

export const ColorMap = new Map<NodeType, BackColor>([
    [NodeType.EQ, BackColor.EQ_COLOR],
    [NodeType.FIELD, BackColor.FIELD_COLOR],
    [NodeType.AISS, BackColor.SEPARATION_COLOR],
    [NodeType.NR, BackColor.NR_COLOR],
    [NodeType.VB, BackColor.VB_COLOR],
    [NodeType.MIXER, BackColor.MIXER_COLOR],
    [NodeType.INPUT, BackColor.INPUT_COLOR],
    [NodeType.OUTPUT, BackColor.OUTPUT_COLOR],
    [NodeType.ENV, BackColor.ENV_COLOR]
])