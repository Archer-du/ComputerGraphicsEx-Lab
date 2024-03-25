#pragma once
#include <vector>

#include "USTC_CG.h"
#include "util_openmesh_bind.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::vector<int> get_boundary_edges(std::shared_ptr<PolyMesh> halfedge_mesh);

USTC_CG_NAMESPACE_CLOSE_SCOPE