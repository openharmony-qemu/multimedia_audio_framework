/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. All rights reserved.
 */

import { Logger } from './Logger';
import { common } from '@kit.AbilityKit';
import { fileUri, picker } from '@kit.CoreFileKit';
import fs from '@ohos.file.fs';
import { BusinessError } from '@kit.BasicServicesKit';

const TAG: string = 'DirInit';

// root path of Auto Test APP
export const rootPath = `/storage/Users/currentUser/Download/src.main.audiodemo`;

// Directories to be checked and created.
const dirCheckList: string[] = ["expected_output", "output_files", "input_files", "case_json"];

/**
 * Directory initialization utility class
 * Provides public methods such as directory creation and inspection.
 */
export class DirInitHelper {
  static async checkAndCreateDefaultDir(): Promise<boolean> {
    try {
      // Check if rootPath exists, create it if not
      if (!DirInitHelper.checkDirExists(rootPath)) {
        Logger.info(TAG, `rootPath not existed, creating: ${rootPath}`);
        if (!DirInitHelper.createDir(rootPath)) {
          Logger.error(TAG, `Failed to create rootPath: ${rootPath}`);
          return false;
        }
      }

      // Check each directory in dirCheckList
      dirCheckList.forEach((check: string) => {
        let dirPath = `${rootPath}/${check}`;
        if (!DirInitHelper.isDirectory(dirPath)) {
          Logger.info(TAG, `dir:${check} not existed`);
          fs.mkdirSync(dirPath);
        }
      })
      Logger.info(TAG, 'Default directories check completed');
      return true;
    } catch (e) {
      Logger.error(TAG, `rootPath check failed: ${e}, need init rootPath`);
      return false;
    }
  }

  /**
   * Initialize the directory using DocumentViewPicker.
   * Let the user select a directory location, then create the required subdirectories.
   */
  static async initDirWithPicker(): Promise<boolean> {
    return new Promise<boolean>((resolve) => {
      let documentSaveOptions = new picker.DocumentSaveOptions();
      documentSaveOptions.pickerMode = 1;
      let context = getContext() as common.Context;
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
        if (err) {
          Logger.error(TAG,
            `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
          resolve(false);
          return;
        }
        Logger.info(TAG, 'documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
        let uri = documentSaveResult[0];
        let filePath = new fileUri.FileUri(uri + `/init.txt`).path;
        try {
          let file = fs.openSync(filePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
          fs.writeSync(file.fd, new ArrayBuffer(0));
          Logger.info(TAG, `audiodemo dir init success`);
          fs.closeSync(file);
          dirCheckList.forEach((dir: string) => {
            let dirPath = new fileUri.FileUri(`${uri}/${dir}`).path;
            fs.mkdirSync(dirPath);
          })
          resolve(true);
        } catch (err) {
          Logger.error(TAG, `initAutoTestDir ERROR: ${err}`);
          resolve(false);
        }
      })
    });
  }

  /**
   * Ensure that the directory exists (intelligent mode)
   * Check the default directory first.
   * If the directory does not exist, use Picker for initialization.
   */
  static async ensureDirExists(): Promise<boolean> {
    let exists = await DirInitHelper.checkAndCreateDefaultDir();
    if (!exists) {
      Logger.info(TAG, 'Default dir not exists, initializing with picker...');
      exists = await DirInitHelper.initDirWithPicker();
    }
    return exists;
  }

  /**
   * Checking whether the specified directory exists
   */
  static checkDirExists(dirPath: string): boolean {
    try {
      fs.accessSync(dirPath);
      return true;
    } catch (e) {
      return false;
    }
  }

  /**
   * Check if the path exists and is a directory
   */
  static isDirectory(dirPath: string): boolean {
    try {
      let stat = fs.lstatSync(dirPath);
      return stat.isDirectory();
    } catch (e) {
      return false;
    }
  }

  /**
   * Create a directory (recursive creation supported)
   */
  static createDir(dirPath: string): boolean {
    try {
      fs.mkdirSync(dirPath, true);
      Logger.info(TAG, `Directory created: ${dirPath}`);
      return true;
    } catch (err) {
      Logger.error(TAG, `Failed to create directory ${dirPath}: ${err}`);
      return false;
    }
  }
}