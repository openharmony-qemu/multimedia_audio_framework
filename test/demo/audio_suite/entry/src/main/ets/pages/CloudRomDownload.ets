/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { audio } from '@kit.AudioKit';
import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import { BusinessError } from '@kit.BasicServicesKit';
import audioNapi from 'libentry.so';

const TAG: string = 'AudioEditTestApp_CloudRomDownload';

// 5 feature types supported by cloud ROM download
const FEATURE_LIST: audio.AudioSuiteFeatureType[] = [
  audio.AudioSuiteFeatureType.AUDIO_MUSIC_ENHANCEMENT,
  audio.AudioSuiteFeatureType.AUDIO_VOICE_ENHANCEMENT,
  audio.AudioSuiteFeatureType.AUDIO_SOURCE_SEPARATION,
  audio.AudioSuiteFeatureType.AUDIO_DIALOG_ENHANCEMENT,
  audio.AudioSuiteFeatureType.AUDIO_CENTER_VOICE_ENHANCEMENT
];

// Display names for UI (aligned with FEATURE_LIST order)
const FEATURE_DISPLAY_NAMES: string[] = [
  'MusicMasterAlgoRes',
  'RecordDefinition',
  'AudioSourceSeparationAlgoRes',
  'RecordDialogAlgoRes',
  'RecordPicAlgoRes'
];

// Feature type names indexed by enum value (for log and result mapping)
const FEATURE_TYPE_NAMES: string[] = [
  'RecordDialogAlgoRes',         // AUDIO_DIALOG_ENHANCEMENT = 0
  'RecordPicAlgoRes',            // AUDIO_CENTER_VOICE_ENHANCEMENT = 1
  'RecordDefinition',            // AUDIO_VOICE_ENHANCEMENT = 2
  'MusicMasterAlgoRes',          // AUDIO_MUSIC_ENHANCEMENT = 3
  'AudioSourceSeparationAlgoRes' // AUDIO_SOURCE_SEPARATION = 4
];

interface LogItem {
  time: string;
  content: string;
}

@Builder
export function CloudRomDownloadBuilder() {
  CloudRomDownload()
}

@Component
export struct CloudRomDownload {
  // Selected feature indexes
  @State selectedFeatures: boolean[] = [false, false, false, false, false];
  // Network type for startDownload: 6=WIFI, 7=CELLULAR_AND_WIFI
  @State networkTypeIndex: number = 0;
  // Whether download status callback is registered
  @State isListening: boolean = false;
  // Log list (newest at bottom)
  @State logList: LogItem[] = [];
  // API mode: false = ArkTS API, true = C API
  @State useCAPI: boolean = false;
  // Scroll controller for log area
  private logScroller: Scroller = new Scroller();
  // Download manager instance (ArkTS API)
  private downloadManager: audio.AudioSuiteDownloadManager | null = null;
  // Callback reference for on/off (ArkTS API)
  private statusCallback: ((info: audio.AudioSuiteFeatureStatusInfoArray) => void) | null = null;
  // C API callback reference (for unregister)
  private capiCallback: ((info: object[]) => void) | null = null;
  // Guard flag to prevent Toggle onChange from firing during programmatic state changes
  private isTogglingApiMode: boolean = false;

  aboutToAppear(): void {
    try {
      this.downloadManager = audio.getAudioManager().getSuiteDownloadManager();
      this.appendLog('getSuiteDownloadManager succeeded (ArkTS).');
    } catch (e) {
      const err = e as BusinessError;
      Logger.error(TAG, `getSuiteDownloadManager failed, code: ${err.code}, msg: ${err.message}`);
      this.appendLog(`getSuiteDownloadManager failed: ${err.code} ${err.message}`);
    }
  }

  aboutToDisappear(): void {
    if (this.isListening) {
      this.toggleListen();
    }
  }

  // Compose featureNames from selection
  private getSelectedFeatureNames(): audio.AudioSuiteFeatureType[] {
    const featureTypes: audio.AudioSuiteFeatureType[] = [];
    for (let i = 0; i < FEATURE_LIST.length; i++) {
      if (this.selectedFeatures[i]) {
        featureTypes.push(FEATURE_LIST[i]);
      }
    }
    // featureTypes.push(6);
    return featureTypes;
  }

  // Compose feature display name strings from selection (for C API)
  private getSelectedFeatureNameStrings(): string[] {
    const names: string[] = [];
    for (let i = 0; i < FEATURE_LIST.length; i++) {
      if (this.selectedFeatures[i]) {
        names.push(FEATURE_DISPLAY_NAMES[i]);
      }
    }
    return names;
  }

  private appendLog(content: string): void {
    const now = new Date();
    const time =
      `${now.getHours().toString().padStart(2, '0')}:` +
      `${now.getMinutes().toString().padStart(2, '0')}:` +
      `${now.getSeconds().toString().padStart(2, '0')}`;
    this.logList.push({ time, content });
    // Keep last 200 entries
    if (this.logList.length > 200) {
      this.logList.splice(0, this.logList.length - 200);
    }
  }

  private appendLogLines(lines: string): void {
    const arr = lines.split('\n');
    for (const line of arr) {
      if (line.length > 0) {
        this.appendLog(line);
      }
    }
  }

  // Common error handler
  private handleError(prefix: string, e: Object): void {
    const err = e as BusinessError;
    const msg = `${prefix} failed: code=${err.code}, msg=${err.message}`;
    Logger.error(TAG, msg);
    this.appendLog(msg);
  }

  // ---- API mode toggle ----

  private toggleApiMode(): void {
    // Set guard to prevent Toggle onChange from firing during programmatic state change
    this.isTogglingApiMode = true;
    // Force-stop listening with the current mode before switching (directly, not via toggleListen)
    if (this.isListening) {
      if (this.useCAPI && this.capiCallback) {
        // C API: unregister callback directly
        const result = audioNapi.cloudRomUnregisterDownloadCallback() as Record<string, number>;
        this.appendLog(`[CAPI] UnregisterDownloadCallback: code=${result.code}`);
      } else if (!this.useCAPI && this.statusCallback) {
        // ArkTS API: unregister callback directly
        try {
          this.downloadManager?.offDownloadStatusChange(this.statusCallback);
          this.appendLog('offDownloadStatusChange success.');
        } catch (e) {
          this.handleError('offDownloadStatusChange', e);
        }
      }
    }
    // Reset state — do not reuse listening state across modes
    this.isListening = false;
    this.statusCallback = null;
    this.capiCallback = null;
    this.useCAPI = !this.useCAPI;
    const modeName = this.useCAPI ? 'C API' : 'ArkTS API';
    this.appendLog(`切换到 ${modeName} 模式`);
    if (this.useCAPI) {
      const result = audioNapi.cloudRomGetDownloadManager() as Record<string, number>;
      this.appendLog(`[CAPI] OH_AudioSuite_GetDownloadManager: code=${result.code}`);
    }
    // Release guard after state settles (next frame)
    setTimeout(() => {
      this.isTogglingApiMode = false;
    }, 0);
  }

  // ---- Interface call wrappers ----

  private callGetInstallStatus(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('isFeatureInstalled: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] isFeatureInstalled start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomIsFeatureInstalled(featureNames) as Record<string, Object>;
      const code = result.code as number;
      if (code === 0) {
        const statuses = result.installStatus as boolean[];
        this.appendLog(`[CAPI] isFeatureInstalled success: ${JSON.stringify(statuses)}`);
      } else {
        this.appendLog(`[CAPI] isFeatureInstalled failed: code=${code}`);
      }
      return;
    }

    this.appendLog(`isFeatureInstalled start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.isFeatureInstalled(featureTypes)
      .then((result: Array<boolean>) => {
        this.appendLog(`isFeatureInstalled success: ${JSON.stringify(result)}`);
      })
      .catch((e: BusinessError) => this.handleError('isFeatureInstalled', e));
  }

  private callGetFeatureStatus(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('getFeatureStatus: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] getFeatureStatus start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomGetFeatureStatus(featureNames) as Record<string, Object>;
      const code = result.code as number;
      if (code === 0) {
        const infos = result.statusInfos as Array<Record<string, Object>>;
        const lines = infos.map((item) =>
          `  [CAPI] ${item.featureName}: downloadStatus=${item.downloadStatus}, progress=${item.progress}, ` +
          `installPath=${item.installPath}, errorCode=${item.errorCode}`);
        this.appendLog('[CAPI] getFeatureStatus success:');
        this.appendLogLines(lines.join('\n'));
      } else {
        this.appendLog(`[CAPI] getFeatureStatus failed: code=${code}`);
      }
      return;
    }

    this.appendLog(`getFeatureStatus start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.getFeatureStatus(featureTypes)
      .then((result: audio.AudioSuiteFeatureStatusInfoArray) => {
        const lines = result.map((item) =>
          `  ${FEATURE_TYPE_NAMES[item.featureType]}: status=${item.status}, progress=${item.progress}, ` +
          `installPath=${item.installPath}, errorCode=${item.errorCode}`);
        this.appendLog('getFeatureStatus success:');
        this.appendLogLines(lines.join('\n'));
      })
      .catch((e: BusinessError) => this.handleError('getFeatureStatus', e));
  }

  private callGetDownloadStatus(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('getDownloadStatus: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] getDownloadStatus start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomGetDownloadStatus(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) =>
        `  [CAPI] ${item.featureName}: downloadStatus=${item.downloadStatus}, progress=${item.progress}, ` +
        `installPath=${item.installPath}, errorCode=${item.errorCode}, code=${item.code}`);
      this.appendLog('[CAPI] getDownloadStatus result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    // ArkTS API 无直接对应的 getDownloadStatus 接口，使用 getFeatureStatus 替代
    this.appendLog('getDownloadStatus (ArkTS: using getFeatureStatus as fallback)');
    this.downloadManager?.getFeatureStatus(featureTypes)
      .then((result: audio.AudioSuiteFeatureStatusInfoArray) => {
        const lines = result.map((item) =>
          `  ${FEATURE_TYPE_NAMES[item.featureType]}: status=${item.status}, progress=${item.progress}, ` +
          `installPath=${item.installPath}, errorCode=${item.errorCode}`);
        this.appendLog('getDownloadStatus success:');
        this.appendLogLines(lines.join('\n'));
      })
      .catch((e: BusinessError) => this.handleError('getDownloadStatus', e));
  }

  private callGetNewVersionInfo(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('getNewVersionInfo: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] getNewVersionInfo start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomGetNewVersionInfo(featureNames) as Record<string, Object>;
      const code = result.code as number;
      if (code === 0) {
        const infos = result.versionInfos as Array<Record<string, Object>>;
        const lines = infos.map((item) =>
          `  [CAPI] ${item.featureName}: version=${item.version}, size=${item.size}, ` +
          `downloadStatus=${item.downloadStatus}, errorCode=${item.errorCode}`);
        this.appendLog('[CAPI] getNewVersionInfo success:');
        this.appendLogLines(lines.join('\n'));
      } else {
        this.appendLog(`[CAPI] getNewVersionInfo failed: code=${code}`);
      }
      return;
    }

    this.appendLog(`getNewVersionInfo start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.getNewVersionInfo(featureTypes)
      .then((result: Array<audio.AudioSuiteFeatureVersionInfo>) => {
        const lines = result.map((item) =>
          `  ${FEATURE_TYPE_NAMES[item.featureType]}: version=${item.version}, size=${item.size}, ` +
          `status=${item.status}, errorCode=${item.errorCode}`);
        this.appendLog('getNewVersionInfo success:');
        this.appendLogLines(lines.join('\n'));
      })
      .catch((e: BusinessError) => this.handleError('getNewVersionInfo', e));
  }

  private callStartDownload(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('startDownload: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      // C API StartDownload has no networkType parameter
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] startDownload start, featureNames=${JSON.stringify(featureNames)} (no networkType param)`);
      const result = audioNapi.cloudRomStartDownload(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) =>
        `  [CAPI] ${item.featureName}: code=${item.code}`);
      this.appendLog('[CAPI] startDownload result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    const networkType: audio.NetworkType =
      this.networkTypeIndex === 0 ? audio.NetworkType.WIFI : audio.NetworkType.CELLULAR_AND_WIFI;
    this.appendLog(`startDownload start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}, networkType=${networkType}`);
    this.downloadManager?.startDownload(featureTypes, networkType)
      .then(() => {
        this.appendLog('startDownload success.');
      })
      .catch((e: BusinessError) => this.handleError('startDownload', e));
  }

  private callPauseDownload(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('pauseDownload: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] pauseDownload start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomPauseDownload(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) => `  [CAPI] ${item.featureName}: code=${item.code}`);
      this.appendLog('[CAPI] pauseDownload result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    this.appendLog(`pauseDownload start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.pauseDownload(featureTypes)
      .then(() => {
        this.appendLog('pauseDownload success.');
      })
      .catch((e: BusinessError) => this.handleError('pauseDownload', e));
  }

  private callCancelDownload(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('cancelDownload: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] cancelDownload start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomCancelDownload(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) => `  [CAPI] ${item.featureName}: code=${item.code}`);
      this.appendLog('[CAPI] cancelDownload result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    this.appendLog(`cancelDownload start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.cancelDownload(featureTypes)
      .then(() => {
        this.appendLog('cancelDownload success.');
      })
      .catch((e: BusinessError) => this.handleError('cancelDownload', e));
  }

  private callStartBackgroundDownload(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('startBackgroundDownload: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] startBackgroundDownload start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomStartBackgroundDownload(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) => `  [CAPI] ${item.featureName}: code=${item.code}`);
      this.appendLog('[CAPI] startBackgroundDownload result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    this.appendLog(`startBackgroundDownload start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.startBackgroundDownload(featureTypes)
      .then(() => {
        this.appendLog('startBackgroundDownload success.');
      })
      .catch((e: BusinessError) => this.handleError('startBackgroundDownload', e));
  }

  private callUninstallFeature(): void {
    const featureTypes = this.getSelectedFeatureNames();
    if (featureTypes.length === 0) {
      this.appendLog('uninstallFeature: please select at least one feature.');
      return;
    }

    if (this.useCAPI) {
      const featureNames = this.getSelectedFeatureNameStrings();
      this.appendLog(`[CAPI] uninstallFeature start, featureNames=${JSON.stringify(featureNames)}`);
      const result = audioNapi.cloudRomUninstallFeature(featureNames) as Record<string, Object>;
      const results = result.results as Array<Record<string, Object>>;
      const lines = results.map((item) => `  [CAPI] ${item.featureName}: code=${item.code}`);
      this.appendLog('[CAPI] uninstallFeature result:');
      this.appendLogLines(lines.join('\n'));
      return;
    }

    this.appendLog(`uninstallFeature start, featureTypes=${JSON.stringify(featureTypes.map(v => FEATURE_TYPE_NAMES[v]))}`);
    this.downloadManager?.uninstallFeature(featureTypes)
      .then(() => {
        this.appendLog('uninstallFeature success.');
      })
      .catch((e: BusinessError) => this.handleError('uninstallFeature', e));
  }

  private toggleListen(): void {
    if (this.useCAPI) {
      this.toggleListenCAPI();
      return;
    }
    this.toggleListenArkTS();
  }

  private toggleListenArkTS(): void {
    if (this.isListening) {
      // off
      if (this.statusCallback) {
        try {
          this.downloadManager?.offDownloadStatusChange(this.statusCallback);
          this.appendLog('offDownloadStatusChange success.');
        } catch (e) {
          this.handleError('offDownloadStatusChange', e);
        }
      }
      this.isListening = false;
      this.statusCallback = null;
    } else {
      // on
      const cb = (info: audio.AudioSuiteFeatureStatusInfoArray) => {
        const lines = info.map((item) =>
          `  [cb] ${FEATURE_TYPE_NAMES[item.featureType]}: status=${item.status}, progress=${item.progress}, ` +
          `errorCode=${item.errorCode}`);
        this.appendLog('onDownloadStatusChange:');
        this.appendLogLines(lines.join('\n'));
      };
      try {
        this.downloadManager?.onDownloadStatusChange(cb);
        this.statusCallback = cb;
        this.isListening = true;
        this.appendLog('onDownloadStatusChange registered.');
      } catch (e) {
        this.handleError('onDownloadStatusChange', e);
      }
    }
  }

  private toggleListenCAPI(): void {
    if (this.isListening) {
      // off
      const result = audioNapi.cloudRomUnregisterDownloadCallback() as Record<string, number>;
      this.appendLog(`[CAPI] UnregisterDownloadCallback: code=${result.code}`);
      this.isListening = false;
      this.capiCallback = null;
    } else {
      // on
      const cb = (infoArray: object[]) => {
        const infos = infoArray as Array<Record<string, Object>>;
        const lines = infos.map((item) =>
          `  [cb] ${item.featureName}: downloadStatus=${item.downloadStatus}, progress=${item.progress}, ` +
          `errorCode=${item.errorCode}`);
        this.appendLog('[CAPI] DownloadStatusChange:');
        this.appendLogLines(lines.join('\n'));
      };
      this.capiCallback = cb;
      const result = audioNapi.cloudRomRegisterDownloadCallback(cb) as Record<string, number>;
      if (result.code === 0) {
        this.isListening = true;
        this.appendLog('[CAPI] RegisterDownloadCallback success.');
      } else {
        this.capiCallback = null;
        this.appendLog(`[CAPI] RegisterDownloadCallback failed: code=${result.code}`);
      }
    }
  }

  private clearLog(): void {
    this.logList = [];
  }

  @Builder
  FeatureCheckboxBuilder(index: number) {
    Row() {
      Checkbox({ name: FEATURE_DISPLAY_NAMES[index], group: 'features' })
        .select(this.selectedFeatures[index])
        .selectedColor('#007DFF')
        .onChange((checked: boolean) => {
          this.selectedFeatures[index] = checked;
        })
      Text(FEATURE_DISPLAY_NAMES[index])
        .fontSize(16)
        .fontColor('#333333')
        .margin({ left: 8 })
    }
    .width('100%')
    .height(44)
    .padding({ left: 12, right: 12 })
    .backgroundColor(this.selectedFeatures[index] ? '#e8e8f0' : Color.Transparent)
    .borderRadius(10)
  }

  @Builder
  ActionButtonBuilder(label: string, action: () => void) {
    Button(label)
      .width('100%')
      .height(44)
      .fontSize(16)
      .fontColor(Color.White)
      .backgroundColor('#6a4fb5')
      .borderRadius(10)
      .margin({ top: 8 })
      .onClick(() => action())
  }

  build() {
    NavDestination() {
      Column() {
        // Title bar with API mode toggle
        Row() {
          Text('云 ROM 下载管理')
            .fontSize(22)
            .fontColor(Color.White)
            .fontWeight(FontWeight.Bold)
            .textAlign(TextAlign.Center)
            .layoutWeight(1)

          // API mode toggle button (top-right)
          Button(this.useCAPI ? 'C API' : 'ArkTS API')
            .fontSize(14)
            .height(36)
            .width(100)
            .borderRadius(8)
            .fontColor(Color.White)
            .backgroundColor(this.useCAPI ? '#e87d3e' : '#6a4fb5')
            .onClick(() => this.toggleApiMode())
            .margin({ right: 8 })
        }
        .width('100%')
        .height(56)
        .backgroundColor('#5a5a6e')

        Scroll() {
          Column() {
            // Section 1: Feature multi-select
            Text('选择特性 (featureNames)')
              .fontSize(18)
              .fontColor('#333333')
              .fontWeight(FontWeight.Medium)
              .margin({ top: 12, bottom: 8 })

            ForEach(FEATURE_LIST, (_: audio.AudioSuiteFeatureType, index: number) => {
              this.FeatureCheckboxBuilder(index)
            }, (item: string) => item)

            // Section 2: Network type (for startDownload - ArkTS only)
            if (!this.useCAPI) {
              Text('网络类型 (startDownload 使用)')
                .fontSize(18)
                .fontColor('#333333')
                .fontWeight(FontWeight.Medium)
                .margin({ top: 16, bottom: 8 })
              Row() {
                Radio({ value: 'WIFI', group: 'networkType' })
                  .checked(this.networkTypeIndex === 0)
                  .onChange((isCheck: boolean) => {
                    if (isCheck) {
                      this.networkTypeIndex = 0;
                    }
                  })
                Text(`WIFI (${audio.NetworkType.WIFI})`)
                  .fontSize(16)
                  .fontColor('#333333')
                  .margin({ left: 4, right: 20 })

                Radio({ value: 'CELLULAR_AND_WIFI', group: 'networkType' })
                  .checked(this.networkTypeIndex === 1)
                  .onChange((isCheck: boolean) => {
                    if (isCheck) {
                      this.networkTypeIndex = 1;
                    }
                  })
                Text(`CELLULAR_AND_WIFI (${audio.NetworkType.CELLULAR_AND_WIFI})`)
                  .fontSize(16)
                  .fontColor('#333333')
                  .margin({ left: 4 })
              }
              .width('100%')
              .height(48)
              .padding({ left: 12, right: 12 })
            }

            // Section 3: Interface buttons
            Text('调用接口')
              .fontSize(18)
              .fontColor('#333333')
              .fontWeight(FontWeight.Medium)
              .margin({ top: 16, bottom: 4 })

            // ArkTS API 支持的接口
            if (!this.useCAPI) {
              this.ActionButtonBuilder('isFeatureInstalled', () => this.callGetInstallStatus())
              this.ActionButtonBuilder('getFeatureStatus', () => this.callGetFeatureStatus())
              this.ActionButtonBuilder('getNewVersionInfo', () => this.callGetNewVersionInfo())
              this.ActionButtonBuilder('startDownload', () => this.callStartDownload())
              this.ActionButtonBuilder('pauseDownload', () => this.callPauseDownload())
              this.ActionButtonBuilder('cancelDownload', () => this.callCancelDownload())
              this.ActionButtonBuilder('startBackgroundDownload', () => this.callStartBackgroundDownload())
              this.ActionButtonBuilder('uninstallFeature', () => this.callUninstallFeature())
            }

            // C API 支持的接口（比 ArkTS 多出 getDownloadStatus）
            if (this.useCAPI) {
              this.ActionButtonBuilder('isFeatureInstalled', () => this.callGetInstallStatus())
              this.ActionButtonBuilder('getFeatureStatus', () => this.callGetFeatureStatus())
              this.ActionButtonBuilder('getDownloadStatus', () => this.callGetDownloadStatus())
              this.ActionButtonBuilder('getNewVersionInfo', () => this.callGetNewVersionInfo())
              this.ActionButtonBuilder('startDownload', () => this.callStartDownload())
              this.ActionButtonBuilder('pauseDownload', () => this.callPauseDownload())
              this.ActionButtonBuilder('cancelDownload', () => this.callCancelDownload())
              this.ActionButtonBuilder('startBackgroundDownload', () => this.callStartBackgroundDownload())
              this.ActionButtonBuilder('uninstallFeature', () => this.callUninstallFeature())
            }

            // on/off toggle
            Row() {
              Text(this.isListening ? '监听中 (点击关闭)' : '未监听 (点击开启 onDownloadStatusChange)')
                .fontSize(14)
                .fontColor('#333333')
                .layoutWeight(1)
              Toggle({ type: ToggleType.Switch, isOn: this.isListening })
                .selectedColor('#8161c4')
                .onChange((isOn: boolean) => {
                  if (this.isTogglingApiMode) {
                    return;
                  }
                  this.toggleListen();
                })
            }
            .width('100%')
            .height(48)
            .margin({ top: 12 })
            .padding({ left: 12, right: 12 })
            .borderRadius(10)
            .backgroundColor('#e8e8f0')

            // Section 4: Log area
            Row() {
              Text('执行日志')
                .fontSize(18)
                .fontColor('#333333')
                .fontWeight(FontWeight.Medium)
                .layoutWeight(1)
              Button('清空')
                .height(32)
                .fontSize(14)
                .fontColor(Color.White)
                .backgroundColor('#6a4fb5')
                .borderRadius(8)
                .onClick(() => this.clearLog())
            }
            .width('100%')
            .margin({ top: 16, bottom: 8 })

            Scroll(this.logScroller) {
              Column() {
                ForEach(this.logList, (item: LogItem, index: number) => {
                  Text(`[${item.time}] ${item.content}`)
                    .fontSize(13)
                    .fontColor(Color.White)
                    .width('100%')
                    .padding({ left: 8, right: 8, top: 4, bottom: 4 })
                    .textAlign(TextAlign.Start)
                }, (item: LogItem, index: number) => `${index}_${item.time}_${item.content}`)
              }
              .width('100%')
            }
            .width('100%')
            .height(320)
            .padding(8)
            .borderRadius(10)
            .backgroundColor('#2a2040')
            .scrollBar(BarState.Auto)
            .onAreaChange(() => {
              this.logScroller.scrollEdge(Edge.Bottom);
            })
          }
          .width(StyleConst.FULL_WIDTH)
          .padding({ left: 16, right: 16, bottom: 24 })
        }
        .layoutWeight(1)
        .scrollBar(BarState.Auto)
      }
      .width(StyleConst.FULL_WIDTH)
      .height(StyleConst.FULL_HEIGHT)
      .backgroundColor('#e0e4e8')
    }
    .title('云 ROM 下载管理')
    .hideBackButton(false)
  }
}
