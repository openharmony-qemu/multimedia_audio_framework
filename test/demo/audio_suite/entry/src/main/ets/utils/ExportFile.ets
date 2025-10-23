/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import audioNapi from 'libentry.so';
import { TextModifier } from '@kit.ArkUI';

@Concurrent
export async function saveFileBuffer(): Promise<ArrayBuffer> {
    let pcmBuffer = audioNapi.saveFileBuffer();
    return pcmBuffer;
}

@Concurrent
export async function RealTimeSaveFileBuffer(): Promise<ArrayBuffer> {
    let pcmBuffer = audioNapi.realTimeSaveFileBuffer();
    return pcmBuffer;
}

export class MainTitleTextModifier extends TextModifier {
    userStyle: boolean = true;

    applyNormalAttribute(instance: TextAttribute): void {
        if (this.userStyle) {
            instance.fontColor($r('app.color.index_title_text_color'));
        }
    }
}