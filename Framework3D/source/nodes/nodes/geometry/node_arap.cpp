#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>

#include "GCore/Components/MeshOperand.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "geom_node_base.h"
#include "utils/util_openmesh_bind.h"

/*
** @brief HW5_ARAP_Parameterization
**
** This file presents the basic framework of a "node", which processes inputs
** received from the left and outputs specific variables for downstream nodes to
** use.
**
** - In the first function, node_declare, you can set up the node's input and
** output variables.
**
** - The second function, node_exec is the execution part of the node, where we
** need to implement the node's functionality.
**
** - The third function generates the node's registration information, which
** eventually allows placing this node in the GUI interface.
**
** Your task is to fill in the required logic at the specified locations
** within this template, especially in node_exec.
*/

namespace USTC_CG::node_arap {
static void node_arap_declare(NodeDeclarationBuilder& b)
{
    // Input-1: Original 3D mesh with boundary
    // Maybe you need to add another input for initialization?
    b.add_input<decl::Geometry>("Origin");
    b.add_input<decl::Geometry>("Surface");
    b.add_input<decl::Int>("Iterations").default_val(5).min(0).max(20);
    /*
    ** NOTE: You can add more inputs or outputs if necessary. For example, in
    ** some cases, additional information (e.g. other mesh geometry, other 
    ** parameters) is required to perform the computation.
    **
    ** Be sure that the input/outputs do not share the same name. You can add
    ** one geometry as
    **
    **                b.add_input<decl::Geometry>("Input");
    **
    ** Or maybe you need a value buffer like:
    **
    **                b.add_input<decl::Float1Buffer>("Weights");
    */

    // Output-1: The UV coordinate of the mesh, provided by ARAP algorithm
    b.add_output<decl::Float2Buffer>("OutputUV");
    b.add_output<decl::Geometry>("Output");
}

static void node_arap_exec(ExeParams params)
{
    // Get the input from params
    auto origin = params.get_input<GOperandBase>("Origin");
    auto surface = params.get_input<GOperandBase>("Surface");
    int max_iteration = params.get_input<int>("Iterations");
    // Avoid processing the node when there is no input
    if (!origin.get_component<MeshComponent>()) {
        throw std::runtime_error("Need Geometry Input.");
    }

    /* ----------------------------- Preprocess -------------------------------
    ** Create a halfedge structure (using OpenMesh) for the input mesh. The
    ** half-edge data structure is a widely used data structure in geometric
    ** processing, offering convenient operations for traversing and modifying
    ** mesh elements.
    */
    auto origin_mesh = operand_to_openmesh(&origin);
    auto surface_mesh = operand_to_openmesh(&surface);

   /* ------------- [HW5_TODO] ARAP Parameterization Implementation -----------
   ** Implement ARAP mesh parameterization to minimize local distortion.
   **
   ** Steps:
   ** 1. Initial Setup: Use a HW4 parameterization result as initial setup.
   **
   ** 2. Local Phase: For each triangle, compute local orthogonal approximation
   **    (Lt) by computing SVD of Jacobian(Jt) with fixed u.
   **
   ** 3. Global Phase: With Lt fixed, update parameter coordinates(u) by solving
   **    a pre-factored global sparse linear system.
   **
   ** 4. Iteration: Repeat Steps 2 and 3 to refine parameterization.
   **
   ** Note:
   **  - Fixed points' selection is crucial for ARAP and ASAP.
   **  - Encapsulate algorithms into classes for modularity.
   */

    std::cout << "node_arap: start initiating." << std::endl;
    std::vector<std::vector<Eigen::Vector2f>> original_faces2vertex(origin_mesh->n_faces());
    std::vector<std::map<int, Eigen::Vector2f>> original_faces2vertex_idx(origin_mesh->n_faces());

    for (const auto& face_handle : origin_mesh->faces()) {
        std::vector<OpenMesh::Vec3f> vertex_3d;
        std::vector<int> vertex_idx;

        for (const auto& vertex_handle : face_handle.vertices_ccw()) {
            vertex_3d.push_back(origin_mesh->point(vertex_handle));
            vertex_idx.push_back(vertex_handle.idx());
        }

        auto v_01 = vertex_3d[1] - vertex_3d[0];
        auto v_02 = vertex_3d[2] - vertex_3d[0];

        std::vector<Eigen::Vector2f> vertex_2d;
        std::map<int, Eigen::Vector2f> vertex_2d_map;

        vertex_2d.emplace_back(0, 0);
        vertex_2d.emplace_back(v_01.norm(), 0);
        vertex_2d.emplace_back(v_02.dot(v_01) / v_01.norm(), std::abs(v_01.cross(v_02).norm()) / v_01.norm());

        vertex_2d_map[vertex_idx[0]] = vertex_2d[0];
        vertex_2d_map[vertex_idx[1]] = vertex_2d[1];
        vertex_2d_map[vertex_idx[2]] = vertex_2d[2];

        original_faces2vertex[face_handle.idx()] = std::move(vertex_2d);
        original_faces2vertex_idx[face_handle.idx()] = std::move(vertex_2d_map);
    }


    std::cout << "node_arap: calculating theta." << std::endl;
    std::vector<float> theta_vec(origin_mesh->n_halfedges(), 0.0f);

    for (const auto& halfedge_handle : origin_mesh->halfedges()) {
        if (halfedge_handle.face().is_valid()) {
            auto vertex_handle_0 = halfedge_handle.from();
            auto vertex_handle_1 = halfedge_handle.to();
            auto vertex_handle_2 = halfedge_handle.next().to();

            auto pos_0 = origin_mesh->point(vertex_handle_0);
            auto pos_1 = origin_mesh->point(vertex_handle_1);
            auto pos_2 = origin_mesh->point(vertex_handle_2);

            auto v_0 = pos_1 - pos_2;
            auto v_1 = pos_0 - pos_2;

            auto cos_theta = v_0.dot(v_1) / (v_0.norm() * v_1.norm());
            auto cot_theta = cos_theta / std::sqrt(1 - cos_theta * cos_theta);
            theta_vec[halfedge_handle.idx()] = cot_theta;
        }
    }

    std::cout << "node_arap: constructing matrix." << std::endl;
    const auto num_vertex = surface_mesh->n_vertices();
    Eigen::SparseMatrix<float> A(num_vertex, num_vertex);

    for (const auto& vertex_handle : origin_mesh->vertices()) {
        int idx_center = vertex_handle.idx();
        if (idx_center == 0) {
            A.coeffRef(idx_center, idx_center) = 1.0f;
        }
        else {
            float coefficient_sum = 0.0;
            for (const auto& outgoing_halfedge_handle : vertex_handle.outgoing_halfedges_ccw()) {
                int idx_neighbor = outgoing_halfedge_handle.to().idx();

                float coefficient = theta_vec[outgoing_halfedge_handle.idx()];
                if (outgoing_halfedge_handle.opp().is_valid())
                    coefficient += theta_vec[outgoing_halfedge_handle.opp().idx()];

                coefficient_sum += coefficient;
                A.coeffRef(idx_center, idx_neighbor) = -coefficient;
            }
            A.coeffRef(idx_center, idx_center) = coefficient_sum;
        }
    }

    A.makeCompressed();
    Eigen::SparseLU<Eigen::SparseMatrix<float>> solver(A);
    solver.factorize(A);
    if (solver.info() != Eigen::Success) {
        std::cout << "node_arap: fail to factorize A" << std::endl;
        throw std::runtime_error("Not implemented");
    }


    std::cout << "node_arap: iteration begin... " << std::endl;
    int iteration_counter = 0;
    while (max_iteration--) {
        iteration_counter++;
        std::cout << "node_arap: iteration " << iteration_counter << std::endl;

        /* local phase */
        std::vector<Eigen::Matrix2f> L_vector(surface_mesh->n_faces());

        for (const auto& face_handle : surface_mesh->faces()) {
            auto face_idx = face_handle.idx();
            Eigen::Matrix2f S = Eigen::Matrix2f::Zero();
            for (const auto& halfedge_handle : face_handle.halfedges()) {
                auto u_i_idx = halfedge_handle.from().idx();
                auto u_ip1_idx = halfedge_handle.to().idx();

                auto pos_u_i = surface_mesh->point(halfedge_handle.from());
                auto pos_u_ip1 = surface_mesh->point(halfedge_handle.to());

                Eigen::Vector2f u_i(pos_u_i[0], pos_u_i[1]);
                Eigen::Vector2f u_ip1(pos_u_ip1[0], pos_u_ip1[1]);

                auto x_i = original_faces2vertex_idx[face_idx][u_i_idx];
                auto x_ip1 = original_faces2vertex_idx[face_idx][u_ip1_idx];

                auto cot_theta = theta_vec[halfedge_handle.idx()];

                S += cot_theta * (u_i - u_ip1) * (x_i - x_ip1).transpose();
            }

            Eigen::JacobiSVD<Eigen::Matrix2f> svd(S, Eigen::ComputeFullU | Eigen::ComputeFullV);
            Eigen::Matrix2f U = svd.matrixU(), V = svd.matrixV();
            if (U.determinant() <= 0)
                U.row(1) *= -1.0f;
            Eigen::Matrix2f L = svd.matrixU() * svd.matrixV().transpose();
            L_vector[face_idx] = L;
        }

        /* global phase
         * 先计算 b_x b_y */
        Eigen::VectorXf b_0 = Eigen::VectorXf::Zero(num_vertex);
        Eigen::VectorXf b_1 = Eigen::VectorXf::Zero(num_vertex);
        for (const auto& vertex_handle : surface_mesh->vertices()) {
            int idx_center = vertex_handle.idx();
            for (const auto& outgoing_halfedge_handle : vertex_handle.outgoing_halfedges_ccw()) {
                Eigen::Vector2f coefficient = Eigen::Vector2f::Zero();

                int idx_neighbor = outgoing_halfedge_handle.to().idx();

                if (outgoing_halfedge_handle.face().is_valid()) {
                    auto face_idx = outgoing_halfedge_handle.face().idx();
                    float cot_theta = theta_vec[outgoing_halfedge_handle.idx()];

                    auto x_i = original_faces2vertex_idx[face_idx][idx_center];
                    auto x_j = original_faces2vertex_idx[face_idx][idx_neighbor];

                    coefficient += cot_theta * L_vector[face_idx] * (x_i - x_j);
                }
                if (outgoing_halfedge_handle.opp().face().is_valid()) {
                    auto face_idx = outgoing_halfedge_handle.opp().face().idx();
                    float cot_theta = theta_vec[outgoing_halfedge_handle.opp().idx()];

                    auto x_i = original_faces2vertex_idx[face_idx][idx_center];
                    auto x_j = original_faces2vertex_idx[face_idx][idx_neighbor];

                    coefficient += cot_theta * L_vector[face_idx] * (x_i - x_j);
                }

                b_0(idx_center) = coefficient(0);
                b_1(idx_center) = coefficient(1);
            }
        }

        auto vertex_handle = surface_mesh->vertex_handle(0);
        auto pos = surface_mesh->point(vertex_handle);
        b_0(0) = pos[0];
        b_1(0) = pos[1];

        Eigen::VectorXf result_0 = solver.solve(b_0.cast<float>()).cast<float>();
        Eigen::VectorXf result_1 = solver.solve(b_1.cast<float>()).cast<float>();

        for (const auto& vertex_handle : surface_mesh->vertices()) {
            auto vertex_idx = vertex_handle.idx();
            auto pos = surface_mesh->point(vertex_handle);
            pos[0] = result_0(vertex_idx);
            pos[1] = result_1(vertex_idx);
            surface_mesh->set_point(vertex_handle, pos);
        }
    } // iteration end


    // The result UV coordinates
    pxr::VtArray<pxr::GfVec2f> uv_result;
    for (const auto& vertex_handle : surface_mesh->vertices()) {
        auto pos = surface_mesh->point(vertex_handle);
        pxr::GfVec2f vec(pos[0], pos[1]);
        uv_result.push_back(vec);
    }


    // Set the output of the node
    params.set_output("OutputUV", uv_result);

    /* ----------------------------- Postprocess ------------------------------ */
    auto operand_base = openmesh_to_operand(surface_mesh.get());
    params.set_output("Output", std::move(*operand_base));
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "ARAP Parameterization");
    strcpy_s(ntype.id_name, "geom_arap");

    geo_node_type_base(&ntype);
    ntype.node_execute = node_arap_exec;
    ntype.declare = node_arap_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_arap
