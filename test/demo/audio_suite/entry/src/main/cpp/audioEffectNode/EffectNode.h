/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_EFFECTNODE_H
#define AUDIOEDITTESTAPP_EFFECTNODE_H

#include <cstdint>
#include <memory>
#include "../NodeManager.h"

extern std::shared_ptr<NodeManager> nodeManager;

int32_t AddEffectNodeToNodeManager(std::string &inputNodeId, std::string &effectNodeId);

#endif //AUDIOEDITTESTAPP_EFFECTNODE_H