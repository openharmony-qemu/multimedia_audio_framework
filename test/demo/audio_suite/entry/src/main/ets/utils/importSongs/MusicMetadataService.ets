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
    // Use the fs file system to open the sandbox address to obtain the media file address,
    // set the fdSrc property, retrieve audio metadata, and print it.
    // Obtain the audio album cover and display it on the screen using an Image control.
    async fetchMetadata(uri: string): Promise<MusicMetadata> {
        let avMetadataExtractor: media.AVMetadataExtractor | null = null;
        let pixelMap: PixelMap | undefined = undefined;
        let metadata: media.AVMetadata | undefined = undefined;
        if (canIUse('SystemCapability.Multimedia.Media.AVMetadataExtractor')) {
            try { // Create an AVMetadataExtractor object
                avMetadataExtractor = await media.createAVMetadataExtractor();
                // Set fdSrc.
                avMetadataExtractor.fdSrc = fs.openSync(uri);
                // Get Metadata (Promise Mode)
                metadata = await avMetadataExtractor.fetchMetadata();
                Logger.info(TAG, `Get metadata successfully, mimeType: ${metadata.mimeType}`);
                // Get Album Cover (Promise Mode)
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