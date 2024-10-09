/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <common.h>

SLresult SLAPIENTRY slCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
    const SLEngineOption *pEngineOptions, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    if (pEngine == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    ClassTable *engineClass = ObjectIdToClass(SL_OBJECTID_ENGINE);
    CEngine *thiz = (CEngine *) Construct(engineClass, nullptr);
    if (thiz == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IObjectInit(&thiz->mObject);
    IEngineInit(&thiz->mEngine);
    *pEngine = &thiz->mObject.mItf;
    return SL_RESULT_SUCCESS;
}

ClassTable *ObjectIdToClass(SLuint32 objectId)
{
    ClassTable *classTable = nullptr;
    if (objectId == SL_OBJECTID_ENGINE) {
        classTable = (ClassTable *) &EngineTab;
    } else if (objectId == SL_OBJECTID_AUDIOPLAYER) {
        classTable = (ClassTable *) &AudioPlayerTab;
    } else if (objectId == SL_OBJECTID_AUDIORECORDER) {
        classTable = (ClassTable *) &AudioRecorderTab;
    } else if (objectId == SL_OBJECTID_OUTPUTMIX) {
        classTable = (ClassTable *) &OutputMixTab;
    }
    return classTable;
}

IObject *Construct(const ClassTable *classTable, SLEngineItf engine)
{
    if (classTable == nullptr) {
        return nullptr;
    }
    IObject *thiz = (IObject *) calloc(1, classTable->mSize);
    if (thiz != nullptr) {
        IEngine *thisEngine = (IEngine *) engine;
        if (thisEngine != nullptr) {
            thiz->mEngine = (CEngine *) thisEngine->mThis;
        } else {
            thiz->mEngine = (CEngine *) thiz;
        }
        thiz->mClass = classTable;
    }
    return thiz;
}
