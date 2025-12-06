/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { image } from '@kit.ImageKit';
import { media } from '@kit.MediaKit';
import { fileIo as fs } from '@kit.CoreFileKit';
import { Logger } from '../Logger';

export interface MusicMetadata {
    pixelMap?: image.PixelMap;
    metadata?: media.AVMetadata;
}

const TAG: string = 'AudioEditTestApp_MusicMetadataService';

export class MusicMetadataService {
    // 使用 fs 文件系统打开沙箱地址获取媒体文件地址， 设置 fdSrc 属性， 获取音频元数据并打印，
    // 获取音频专辑封面并通过 Image 控件显示在屏幕上。
    async fetchMetadata(uri: string): Promise<MusicMetadata> {
        let avMetadataExtractor: media.AVMetadataExtractor | null = null;
        let pixelMap: PixelMap | undefined = undefined;
        let metadata: media.AVMetadata | undefined = undefined;
        if (canIUse('SystemCapability.Multimedia.Media.AVMetadataExtractor')) {
            try { // 创建 AVMetadataExtractor 对象
                avMetadataExtractor = await media.createAVMetadataExtractor();
                // 设置fdSrc
                avMetadataExtractor.fdSrc = fs.openSync(uri);
                // 获取元数据（promise 模式）
                metadata = await avMetadataExtractor.fetchMetadata();
                Logger.info(TAG, `Get metadata successfully, mimeType: ${metadata.mimeType}`);
                // 获取专辑封面（promise 模式）
                await avMetadataExtractor
                    .fetchAlbumCover()
                    .then((cover: PixelMap) => {
                        pixelMap = cover;
                    })
                    .catch((e: Error) => {
                        Logger.error(TAG, `Get Album Cover Failed`);
                    });
            } catch (e) {
                Logger.error(TAG, `createAVMetadataExtractor Failed`);
            } finally {
                //First check whether avMetadataExtractor exists, then access fdSrc.
                if (avMetadataExtractor) {
                    // destroy allocations
                    avMetadataExtractor.release();
                    // Safely close file descriptor: Attempt to close it only if fdSrc exists.
                    const fd = avMetadataExtractor.fdSrc?.fd;
                    if (fd !== undefined && fd !== null) {
                        try {
                            fs.closeSync(fd);
                        } catch (closeErr) {
                            Logger.warn(TAG, `Failed to close file descriptor ${fd}: ${closeErr.message}`);
                        }
                    }
                } else {
                    Logger.warn(TAG, 'avMetadataExtractor is null, cannot release or close fd.');
                }
            }
            return { pixelMap: pixelMap, metadata: metadata };
        } else {
            throw new Error('System does not support AVMetadataExtractor');
        }
    }
}

export const METADATA_SERVICE = new MusicMetadataService();