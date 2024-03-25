#pragma once
#include "USTC_CG.h"
#include <Eigen/Sparse>
#include "util_openmesh_bind.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void solve_transform(
    const Eigen::SparseMatrix<double>& A,
    int vertex_num,
    std::shared_ptr<PolyMesh> halfedge_mesh);

USTC_CG_NAMESPACE_CLOSE_SCOPE