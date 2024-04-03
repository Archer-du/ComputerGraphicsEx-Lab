#pragma once

#include <memory>

#include "USTC_CG.h"
#include "pxr/usd/usd/stage.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTree;
struct UsdviewEngineImpl;

class USTC_CG_API UsdviewEngine {
public:
    explicit UsdviewEngine(pxr::UsdStageRefPtr root_stage);
    ~UsdviewEngine();
    void render(NodeTree* render_nodetree = nullptr);

protected:
    std::unique_ptr<UsdviewEngineImpl> impl_;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
