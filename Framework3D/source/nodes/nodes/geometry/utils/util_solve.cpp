#include "util_solve.h"

#include "util_openmesh_bind.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void solve_transform(
    const Eigen::SparseMatrix<double>& A,
    int vertex_num,
    std::shared_ptr<PolyMesh> halfedge_mesh)
{
    for (int dim = 0; dim < 3; ++dim) {
        Eigen::SparseVector<double> b(vertex_num);

        for (const auto& vertex_handle : halfedge_mesh->vertices()) {
            int idx = vertex_handle.idx();
            if (vertex_handle.is_boundary()) {
                b.coeffRef(idx) = halfedge_mesh->point(vertex_handle)[dim];
            }
        }

        Eigen::SparseLU<Eigen::SparseMatrix<double>> solver(A);
        solver.factorize(A);
        if (solver.info() != Eigen::Success) {
            throw std::runtime_error("Minimal Surface: Matrix A factorize failed.");
        }
        Eigen::VectorXd x = solver.solve(b);
        for (const auto& vertex_handle : halfedge_mesh->vertices()) {
            int idx = vertex_handle.idx();
            if (!vertex_handle.is_boundary()) {
                auto point = halfedge_mesh->point(vertex_handle);
                point[dim] = x(idx);
                halfedge_mesh->set_point(vertex_handle, point);
            }
        }
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE