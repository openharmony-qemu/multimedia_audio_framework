# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(Audio)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(entry SHARED AudioEdit.cpp NodeManager.cpp callback/RegisterCallback.cpp audioSuiteError/AudioSuiteError.cpp audiosuite_dt_scene/scene_runner.cpp audiosuite_dt_scene/audiosuite_dt_main.cpp audioSuiteError/AudioSuiteError.h audioEffectNode/VoiceBeautifier.h audioEffectNode/VoiceBeautifier.cpp audioEffectNode/VoiceBeautifier.h audioEffectNode/VoiceBeautifier.cpp)
target_link_libraries(entry PUBLIC libace_napi.z.so libohaudio.so libhilog_ndk.z.so libnative_media_codecbase.so libohfileuri.so libohaudiosuite.so libnative_media_avdemuxer.so libnative_media_avsource.so libnative_media_core.so)