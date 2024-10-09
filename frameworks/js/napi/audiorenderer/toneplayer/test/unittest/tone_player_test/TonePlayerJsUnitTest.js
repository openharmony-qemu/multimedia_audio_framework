/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import audio from '@ohos.multimedia.audio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'


describe("TonePlayerJsUnitTest", function () {
    let audioRendererInfo = {
        content : audio.ContentType.CONTENT_TYPE_SONIFICATION,
        usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
        rendererFlags : 0
    }

    beforeAll(async function () {

        // input testsuit setup step，setup invoked before all testcases
        console.info('TonePlayerJsUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('TonePlayerJsUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('TonePlayerJsUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('TonePlayerJsUnitTest:afterEach called')
    })
})