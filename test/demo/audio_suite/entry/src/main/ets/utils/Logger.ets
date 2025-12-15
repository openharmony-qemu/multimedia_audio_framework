/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import hilog from '@ohos.hilog';

/**
 * Logger
 */
export class Logger {
    private static readonly PARENT_CONTROL_DOMAIN: number = 0x5555;
    private static readonly hpcString: string = 'HwParentControl';

    /**
     * debug log
     *
     * @param tag tag
     * @param msg message
     */
    public static debug(tag: string, msg: string, ...args: (Object | undefined | null)[]): void {
        const msgArg = Logger.formatLogArg(msg, ...args);
        hilog.debug(Logger.PARENT_CONTROL_DOMAIN, Logger.hpcString, `${tag} - ${msgArg}`);
    }

    /**
     * info log
     *
     * @param tag tag
     * @param msg message
     */
    public static info(tag: string, msg: string, ...args: (Object | undefined | null)[]): void {
        const msgArg = Logger.formatLogArgSafely(msg, ...args);
        hilog.info(Logger.PARENT_CONTROL_DOMAIN, Logger.hpcString, `${tag} - ${msgArg}`);
    }

    /**
     * warn log
     *
     * @param tag tag
     * @param msg message
     */
    public static warn(tag: string, msg: string, ...args: (Object | undefined | null)[]): void {
        const msgArg = Logger.formatLogArgSafely(msg, ...args);
        hilog.warn(Logger.PARENT_CONTROL_DOMAIN, Logger.hpcString, `${tag} - ${msgArg}`);
    }

    /**
     * error log
     *
     * @param tag tag
     * @param msg message
     */
    public static error(tag: string, msg: string, ...args: (Object | undefined | null)[]): void {
        const msgArg = Logger.formatLogArgSafely(msg, ...args);
        hilog.error(Logger.PARENT_CONTROL_DOMAIN, Logger.hpcString, `${tag} - ${msgArg}`);
    }

    private static formatLogArg(msg: string, ...args: (Object | undefined | null)[]): string {
        return msg.replace(/{\d+}/g, (match) => {
            const index = match.match(/\d+/)?.[0];
            if (args.length > 0 && index !== undefined) {
                return Logger.getPrintArg(args[index]);
            } else {
                return '';
            }
        })
    }

    private static formatLogArgSafely(msg: string, ...args: (Object | undefined | null)[]): string {
        return msg.replace(/{\d+}/g, (match) => {   
            const index = match.match(/\d+/)?.[0];
            if (args.length > 0 && index !== undefined) {
                return Logger.getSafetyPrintArg(args[index]);
            } else {
                return '';
            }
        })
    }

    /**
     * Debug-level formatted logs: Strings, numbers, and booleans are printed in their original form;
     * Error types only print the name and description; others are converted to strings using JSON.stringify before being printed.
     * @param logs
     */
    private static getPrintArg(log: Object | undefined): string {
        try {
            if (typeof log !== 'object' && log!== null && log !== undefined) {
                return log;
            }
            if (log instanceof Error) {
                return `errorName: ${log.name}. errorMessage: ${log.message}`;
            }
            return JSON.stringify(log);
        } catch {
            return '{print.err}';
        }
    }

    /**
     * Formatted logs for info, warn, and error levels: Error type only prints the name and description;
     * printing objects and JSON objects is prohibited.
     * @param logs
     */
    private static getSafetyPrintArg(log: Object | undefined | null): string {
        try {
            if (log === null || log === undefined) {
                return 'undefined';
            }
            // Rule 1: Only print the name and description for Error types
            if (log instanceof Error) {
                return `errorName: ${log.name}. errorMessage: ${log.message}`;
            }
            // Rule 2: Prohibition of Printing Objects
            if (typeof log === 'object') {
                return '{print.obj}';
            }
            // Rule 3: Printing JSON objects is prohibited
            if (typeof log === 'string') {
                return Logger.replaceBracesWithLength(log);
            }
            return log;
        } catch {
            return '{print.err}';
        }
    }

    /**
     * Replace {} with length to prevent printing JSON objects
     * @param input Input string
     * @returns Character string after replacement
     */
    private static replaceBracesWithLength(input: string): string {
        return input.replace(/\{.*\}/, (match) => {
            const length = match.length;
            return '{print.json.len: ' + length.toString() + '}';
        });
    }
}