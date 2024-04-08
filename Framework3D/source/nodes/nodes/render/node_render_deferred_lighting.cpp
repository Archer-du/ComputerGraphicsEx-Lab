#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "Nodes/socket_types/basic_socket_types.hpp"
#include "camera.h"
#include "light.h"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"
#include "rich_type_buffer.hpp"
#include "utils/draw_fullscreen.h"

namespace USTC_CG::node_deferred_lighting {

static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Lights>("Lights");

    b.add_input<decl::Texture>("Position");
    b.add_input<decl::Texture>("Normal");
    b.add_output<decl::Texture>("MetallicRoughness");
    b.add_output<decl::Texture>("diffuseColor");

    b.add_input<decl::String>("Lighting Shader").default_val("shaders/blinn_phong.fs");
    b.add_output<decl::Texture>("Color");
}

static void node_exec(ExeParams params)
{
    // Fetch all the information

    auto lights = params.get_input<LightArray>("Lights");

    auto position_texture = params.get_input<TextureHandle>("Position");
    auto normal_texture = params.get_input<TextureHandle>("Normal");
    auto metallic_roughness = params.get_input<TextureHandle>("MetallicRoughness");
    auto diffuseColor_texture = params.get_input<TextureHandle>("diffuseColor");

    // Creating output textures.
    auto size = position_texture->desc.size;
    TextureDesc color_output_desc;
    color_output_desc.format = HdFormatFloat32Vec4;
    color_output_desc.size = size;
    auto color_texture = resource_allocator.create(color_output_desc);

    unsigned int VBO, VAO;
    CreateFullScreenVAO(VAO, VBO);

    auto shaderPath = params.get_input<std::string>("Lighting Shader");

    ShaderDesc shader_desc;
    shader_desc.set_vertex_path(
        std::filesystem::path(RENDER_NODES_FILES_DIR) /
        std::filesystem::path("shaders/fullscreen.vs"));

    shader_desc.set_fragment_path(
        std::filesystem::path(RENDER_NODES_FILES_DIR) / std::filesystem::path(shaderPath));
    auto shader = resource_allocator.create(shader_desc);
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->texture_id, 0);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader->shader.use();
    shader->shader.setVec2("iResolution", size);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    DestroyFullScreenVAO(VAO, VBO);

    params.set_output("Color", color_texture);
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Deferred Lighting");
    strcpy_s(ntype.id_name, "render_deferred_lighting");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_deferred_lighting
