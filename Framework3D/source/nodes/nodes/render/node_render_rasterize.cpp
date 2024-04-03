#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"

#include "camera.h"
#include "light.h"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"
#include "rich_type_buffer.hpp"

namespace USTC_CG::node_rasterize {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Lights>("Lights");
    b.add_input<decl::Camera>("Camera");
    b.add_output<decl::Texture>("Position");
    b.add_output<decl::Texture>("Normal");
    b.add_output<decl::Texture>("Depth");
}

static void node_exec(ExeParams params)
{
    // Left empty.
    auto lights = params.get_input<LightArray>("Lights");
    auto cameras = params.get_input<CameraArray>("Camera");

    TextureDesc texture_desc;
    auto position_texture = resource_allocator.create(texture_desc);

    params.set_output("Position", position_texture);
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Rasterize");
    strcpy_s(ntype.id_name, "render_rasterize");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_rasterize
