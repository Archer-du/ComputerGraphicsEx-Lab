#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <iostream>

#include "GCore/Components/MeshOperand.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "geom_node_base.h"
#include "utils/util_openmesh_bind.h"

/*
** @brief HW4_TutteParameterization
**
** This file presents the basic framework of a "node", which processes inputs
** received from the left and outputs specific variables for downstream nodes to
** use.
** - In the first function, node_declare, you can set up the node's input and
** output variables.
** - The second function, node_exec is the execution part of the node, where we
** need to implement the node's functionality.
** - The third function generates the node's registration information, which
** eventually allows placing this node in the GUI interface.
**
** Your task is to fill in the required logic at the specified locations
** within this template, especially in node_exec.
*/

namespace USTC_CG::node_min_surf {
static void node_min_surf_declare(NodeDeclarationBuilder& b)
{
    // Input-1: Original 3D mesh with boundary
    b.add_input<decl::Geometry>("Input");

    /*
    ** NOTE: You can add more inputs or outputs if necessary. For example, in some cases,
    ** additional information (e.g. other mesh geometry, other parameters) is required to perform
    ** the computation.
    **
    ** Be sure that the input/outputs do not share the same name. You can add one geometry as
    **
    **                b.add_input<decl::Geometry>("Input");
    **
    ** Or maybe you need a value buffer like:
    **
    **                b.add_input<decl::Float1Buffer>("Weights");
    */

    // Output-1: Minimal surface with fixed boundary
    b.add_output<decl::Geometry>("Output");
}

static void node_min_surf_exec(ExeParams params)
{
    // Get the input from params
    auto input = params.get_input<GOperandBase>("Input");

    // (TO BE UPDATED) Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Minimal Surface: Need Geometry Input.");
    }

    /* ----------------------------- Preprocess -------------------------------
    ** Create a halfedge structure (using OpenMesh) for the input mesh. The
    ** half-edge data structure is a widely used data structure in geometric
    ** processing, offering convenient operations for traversing and modifying
    ** mesh elements.
    */
    auto halfedge_mesh = operand_to_openmesh(&input);

    /* ---------------- [HW4_TODO] TASK 1: Minimal Surface --------------------
    ** In this task, you are required to generate a 'minimal surface' mesh with
    ** the boundary of the input mesh as its boundary.
    **
    ** Specifically, the positions of the boundary vertices of the input mesh
    ** should be fixed. By solving a global Laplace equation on the mesh,
    ** recalculate the coordinates of the vertices inside the mesh to achieve
    ** the minimal surface configuration.
    **
    ** (Recall the Poisson equation with Dirichlet Boundary Condition in HW3)
    */

    /*
    ** Algorithm Pseudocode for Minimal Surface Calculation
    ** ------------------------------------------------------------------------
    ** 1. Initialize mesh with input boundary conditions.
    **    - For each boundary vertex, fix its position.
    **    - For internal vertices, initialize with initial guess if necessary.
    **
    ** 2. Construct Laplacian matrix for the mesh.
    **    - Compute weights for each edge based on the chosen weighting scheme
    **      (e.g., uniform weights for simplicity).
    **    - Assemble the global Laplacian matrix.
    **
    ** 3. Solve the Laplace equation for interior vertices.
    **    - Apply Dirichlet boundary conditions for boundary vertices.
    **    - Solve the linear system (Laplacian * X = 0) to find new positions
    **      for internal vertices.
    **
    ** 4. Update mesh geometry with new vertex positions.
    **    - Ensure the mesh respects the minimal surface configuration.
    **
    ** Note: This pseudocode outlines the general steps for calculating a
    ** minimal surface mesh given fixed boundary conditions using the Laplace
    ** equation. The specific implementation details may vary based on the mesh
    ** representation and numerical methods used.
    **
    */

    int vertex_num = halfedge_mesh->n_vertices();
    Eigen::SparseMatrix<double> A(vertex_num, vertex_num);
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = vertex_handle.idx();
        if (vertex_handle.is_boundary()) {
            A.coeffRef(idx, idx) = 1;
        }
        else {
            int neighbor_num = 0;
            for (const auto& out_halfedge : vertex_handle.outgoing_halfedges()) {
                ++neighbor_num;
                int neighbor_idx = out_halfedge.to().idx();
                A.coeffRef(idx, neighbor_idx) = -1;
            }
            A.coeffRef(idx, idx) = neighbor_num;
        }
    }
    A.makeCompressed();

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

    /* ----------------------------- Postprocess ------------------------------
    ** Convert the minimal surface mesh from the halfedge structure back to
    ** GOperandBase format as the node's output.
    */
    auto operand_base = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*operand_base));

    //for (const auto& vertex_handle : halfedge_mesh->vertices()) {
    //    pxr::GfVec3f val;
    //    auto point = halfedge_mesh->point(vertex_handle);
    //    for (int i = 0; i < point.size() && i < 3; ++i)
    //        val[i] = point[i];
    //    buffer.push_back(val);
    //}
    //params.set_output("Buffer", buffer);
}

static void node_min_surf_cot_declare(NodeDeclarationBuilder& b)
{
    // Input-1: Original 3D mesh with boundary
    b.add_input<decl::Geometry>("Origin");
    b.add_input<decl::Geometry>("Input");
    // Output-1: Minimal surface with fixed boundary
    b.add_output<decl::Geometry>("Output");
}

static void node_min_surf_cot_exec(ExeParams params)
{
    // Get the input from params
    auto input = params.get_input<GOperandBase>("Input");
    auto input_o = params.get_input<GOperandBase>("Origin");

    // (TO BE UPDATED) Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>() || !input_o.get_component<MeshComponent>()) {
        throw std::runtime_error("Minimal Surface: Need Geometry Input.");
    }
    /* ----------------------------- Preprocess ------------------------------- */
    auto halfedge_mesh = operand_to_openmesh(&input);
    auto origin_mesh = operand_to_openmesh(&input_o);

    int vertex_num = halfedge_mesh->n_vertices();
    Eigen::SparseMatrix<double> A(vertex_num, vertex_num);
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = vertex_handle.idx();
        if (vertex_handle.is_boundary()) {
            A.coeffRef(idx, idx) = 1;
        }
        else {
            double sum_weight = 0.0;
            for (const auto& out_halfedge : vertex_handle.outgoing_halfedges()) {
                double weight = 0.0;
                int neighbor_idx = out_halfedge.to().idx();
                auto v1_index = out_halfedge.prev().from().idx();
                auto v2_index = out_halfedge.opp().next().to().idx();

                auto pos_self = origin_mesh->point(origin_mesh->vertex_handle(idx));
                auto pos_neighbor = origin_mesh->point(origin_mesh->vertex_handle(neighbor_idx));
                auto pos_1 = origin_mesh->point(origin_mesh->vertex_handle(v1_index));
                auto pos_2 = origin_mesh->point(origin_mesh->vertex_handle(v2_index));

                auto vec_1_1 = pos_self - pos_1;
                auto vec_1_2 = pos_neighbor - pos_1;
                auto vec_2_1 = pos_self - pos_2;
                auto vec_2_2 = pos_neighbor - pos_2;

                auto cos_theta_1 = vec_1_1.dot(vec_1_2) / (vec_1_1.norm() * vec_1_2.norm());
                auto cos_theta_2 = vec_2_1.dot(vec_2_2) / (vec_2_1.norm() * vec_2_2.norm());

                auto cot_theta_1 = cos_theta_1 / (std::sqrt(1 - cos_theta_1 * cos_theta_1));
                auto cot_theta_2 = cos_theta_2 / (std::sqrt(1 - cos_theta_2 * cos_theta_2));

                weight = cot_theta_1 + cot_theta_2;

                A.coeffRef(idx, neighbor_idx) = -weight;
                sum_weight += weight;
            }
            A.coeffRef(idx, idx) = sum_weight;
        }
    }
    A.makeCompressed();
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
            std::cout << "fail to factorize A" << std::endl;
        }
        else {
            Eigen::VectorXd x = solver.solve(b);
            for (const auto& vertex_handle : halfedge_mesh->vertices()) {
                int index_self = vertex_handle.idx();
                if (!vertex_handle.is_boundary()) {
                    auto point = halfedge_mesh->point(vertex_handle);
                    point[dim] = x(index_self);
                    halfedge_mesh->set_point(vertex_handle, point);
                }
            }
        }
    }

    /* ----------------------------- Postprocess ------------------------------ */
    auto operand_base = openmesh_to_operand(halfedge_mesh.get());
    params.set_output("Output", std::move(*operand_base));
}

static void node_register()
{
    static NodeTypeInfo ntype, ntype_cot;

    strcpy(ntype.ui_name, "Minimal Surface Uniform");
    strcpy_s(ntype.id_name, "geom_min_surf");

    geo_node_type_base(&ntype);
    ntype.node_execute = node_min_surf_exec;
    ntype.declare = node_min_surf_declare;
    nodeRegisterType(&ntype);

    strcpy(ntype_cot.ui_name, "Minimal Surface Cotangent");
    strcpy_s(ntype_cot.id_name, "geom_min_surf_cot");

    geo_node_type_base(&ntype_cot);
    ntype_cot.node_execute = node_min_surf_cot_exec;
    ntype_cot.declare = node_min_surf_cot_declare;
    nodeRegisterType(&ntype_cot);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_min_surf
