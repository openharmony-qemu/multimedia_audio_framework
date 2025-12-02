/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

export const audioEditNodeInitMultiPipeline: (inputId: string) => number;
export const multiAudioInAndOutInit: (inputId: string, outputId: string, mixerId: string, filenames: string) => number;
export const multiPipelineEnvPrepare: (pipelineId: string) => number;
export const multiSetFormat: (channels: number, sampleRate: number, bitsPerSample: number) => number;
export const multiSaveFileBuffer: () => ArrayBuffer;
export const multiGetSecondOutputAudio:() => ArrayBuffer;
export const multiDeleteSong: (inputId: string) => number;
export const destroyMultiPipeline: () => number;
export const multiAudioRendererInit: () => number;
export const multiAudioRendererStart:() => number;
export const multiRealTimeSaveFileBuffer: () => ArrayBuffer;
export const getAutoTestProcess:() => Record<string, number>;