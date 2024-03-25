#pragma once
#include "USTC_CG.h"
#include "util_getboundary_edges.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

std::vector<int> get_boundary_edges(std::shared_ptr<PolyMesh> halfedge_mesh)
{
    std::vector<int> boundary_halfedges;
    for (const auto& halfedge_handle : halfedge_mesh->halfedges()) {
        if (halfedge_handle.is_boundary()) {
            boundary_halfedges.push_back(halfedge_handle.idx());
            break;
        }
    }
    if (boundary_halfedges.empty()) {
        throw std::runtime_error("No boundary edges.");
    }

    int index = boundary_halfedges[0];
    do {
        auto this_handle = halfedge_mesh->halfedge_handle(index);
        int next_index = halfedge_mesh->next_halfedge_handle(this_handle).idx();
        boundary_halfedges.push_back(next_index);
        index = next_index;
    } while (index != boundary_halfedges[0]);

    boundary_halfedges.pop_back();
    return boundary_halfedges;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE