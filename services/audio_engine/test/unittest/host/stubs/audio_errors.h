/*
 * Stub header for x86 host build.
 * Provides error code constants.
 */
#ifndef AUDIO_ERRORS_H
#define AUDIO_ERRORS_H

constexpr int32_t SUCCESS = 0;
constexpr int32_t ERROR = -1;
constexpr int32_t ERR_INVALID_OPERATION = -2;
constexpr int32_t ERR_ILLEGAL_STATE = -3;
constexpr int32_t ERR_NOT_SUPPORTED = -4;
constexpr int32_t ERR_INVALID_PARAM = -5;

#endif  // AUDIO_ERRORS_H
