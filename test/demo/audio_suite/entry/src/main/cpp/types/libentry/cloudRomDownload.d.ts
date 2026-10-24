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

export const cloudRomGetDownloadManager: () => { code: number };

export const cloudRomIsFeatureInstalled: (featureNames: string[]) => {
  code: number;
  installStatus: boolean[];
};

export const cloudRomGetFeatureStatus: (featureNames: string[]) => {
  code: number;
  statusInfos: Array<{
    featureName: string;
    downloadStatus: number;
    size: number;
    installPath: string;
    progress: number;
    errorCode: number;
  }>;
};

export const cloudRomGetDownloadStatus: (featureNames: string[]) => {
  results: Array<{
    featureName: string;
    downloadStatus: number;
    size: number;
    installPath: string;
    progress: number;
    errorCode: number;
    code: number;
  }>;
};

export const cloudRomGetNewVersionInfo: (featureNames: string[]) => {
  code: number;
  versionInfos: Array<{
    featureName: string;
    downloadStatus: number;
    version: string;
    size: number;
    errorCode: number;
  }>;
};

export const cloudRomStartDownload: (featureNames: string[]) => {
  results: Array<{ featureName: string; code: number }>;
};

export const cloudRomPauseDownload: (featureNames: string[]) => {
  results: Array<{ featureName: string; code: number }>;
};

export const cloudRomCancelDownload: (featureNames: string[]) => {
  results: Array<{ featureName: string; code: number }>;
};

export const cloudRomStartBackgroundDownload: (featureNames: string[]) => {
  results: Array<{ featureName: string; code: number }>;
};

export const cloudRomUninstallFeature: (featureNames: string[]) => {
  results: Array<{ featureName: string; code: number }>;
};

export const cloudRomRegisterDownloadCallback: (callback: (infoArray: Object[]) => void) => { code: number };

export const cloudRomUnregisterDownloadCallback: () => { code: number };
