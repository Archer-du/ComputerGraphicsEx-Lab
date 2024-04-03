#include "Nodes/node.hpp"
#include "Nodes/node_register.h"
#include "USTC_CG.h"
#include "rich_type_buffer.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
static void reset_declaration(NodeDeclaration& declaration)
{
    std::destroy_at(&declaration);
    new (&declaration) NodeDeclaration();
}

void build_node_declaration(
    const NodeTypeInfo& typeinfo,
    NodeDeclaration& r_declaration,
    const NodeTree* ntree,
    const Node* node)
{
    reset_declaration(r_declaration);
    NodeDeclarationBuilder node_decl_builder{ r_declaration, ntree, node };
    typeinfo.declare(node_decl_builder);
    node_decl_builder.finalize();
}

static std::map<std::string, NodeTypeInfo*> geo_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_geo_node_registry()
{
    return geo_node_registry;
}

static std::map<std::string, NodeTypeInfo*> render_node_registry;

const std::map<std::string, NodeTypeInfo*>& get_render_node_registry()
{
    return render_node_registry;
}

void nodeRegisterType(NodeTypeInfo* type_info)
{
    switch (type_info->node_type_of_grpah) {
        case NodeTypeOfGrpah::Geometry: geo_node_registry[type_info->id_name] = type_info; break;
        case NodeTypeOfGrpah::Render: render_node_registry[type_info->id_name] = type_info; break;

        case NodeTypeOfGrpah::Function:
            geo_node_registry[type_info->id_name] = type_info;
            render_node_registry[type_info->id_name] = type_info;
            break;
        default: logging("Unknown graph type of node.", Error);
    }

    if (type_info->declare) {
        type_info->static_declaration = std::make_unique<NodeDeclaration>();
    }
    build_node_declaration(*type_info, *type_info->static_declaration, nullptr, nullptr);
}

NodeTypeInfo* nodeTypeFind(const char* idname)
{
    if (idname[0]) {
        auto& geo_node_registry = get_geo_node_registry();
        auto& render_node_registry = get_render_node_registry();

        NodeTypeInfo* nt;
        if (geo_node_registry.find(std::string(idname)) != geo_node_registry.end()) {
            nt = geo_node_registry.at(std::string(idname));
            if (nt) {
                return nt;
            }
        }
        if (render_node_registry.find(std::string(idname)) != render_node_registry.end()) {
            nt = render_node_registry.at(std::string(idname));
            if (nt) {
                return nt;
            }
        }
    }
    throw std::runtime_error("Id name not found.");
}

static std::map<std::string, std::unique_ptr<SocketTypeInfo>> socket_registry;

SocketTypeInfo* socketTypeFind(const char* idname)
{
    if (idname[0]) {
        auto& registry = socket_registry;
        auto nt = registry.at(std::string(idname)).get();
        if (nt) {
            return nt;
        }
    }

    return nullptr;
}

const char* get_socket_typename(SocketType socket)
{

    switch (socket) {
        case SocketType::Geometry: return "SocketGeometry";
        case SocketType::Int: return "SocketInt";
        case SocketType::Float: return "SocketFloat";
#define GetTypeName(Type, Size) \
    case SocketType::Type##Size##Buffer: return "Socket" #Type #Size "Buffer";
        case SocketType::String:
            return "SocketString";
            GetTypeName(Float, 1);
            GetTypeName(Float, 2);
            GetTypeName(Float, 3);
            GetTypeName(Float, 4);
            GetTypeName(Int, 1);
            GetTypeName(Int, 2);
            GetTypeName(Int, 3);
            GetTypeName(Int, 4);
        case SocketType::Lights: return "SocketLights";
        case SocketType::Layer: return "SocketLayer";

        default: return "";
    }
}

SocketTypeInfo* make_standard_socket_type(SocketType socket)
{
    auto type_info = new SocketTypeInfo();
    type_info->type = socket;
    strcpy(type_info->type_name, get_socket_typename(socket));
    return type_info;
}

static SocketTypeInfo* make_socket_type_int()
{
    SocketTypeInfo* socktype = make_standard_socket_type(SocketType::Int);
    socktype->cpp_type = &CPPType::get<int>();
    return socktype;
}

static SocketTypeInfo* make_socket_type_float()
{
    SocketTypeInfo* socktype = make_standard_socket_type(SocketType::Float);
    socktype->cpp_type = &CPPType::get<float>();
    return socktype;
}

static SocketTypeInfo* make_socket_type_string()
{
    SocketTypeInfo* socktype = make_standard_socket_type(SocketType::String);
    socktype->cpp_type = &CPPType::get<std::string>();
    return socktype;
}

#define MakeTypeBuffer(Type, Item, Size)                                                      \
    static SocketTypeInfo* make_socket_type_##Type##Size##_buffer()                           \
    {                                                                                         \
        SocketTypeInfo* socktype = make_standard_socket_type(SocketType::Type##Size##Buffer); \
        socktype->cpp_type = &CPPType::get<pxr::VtArray<Item>>();                             \
        return socktype;                                                                      \
    }

MakeTypeBuffer(Float, float, 1);
MakeTypeBuffer(Float, pxr::GfVec2f, 2);
MakeTypeBuffer(Float, pxr::GfVec3f, 3);
MakeTypeBuffer(Float, pxr::GfVec4f, 4);

MakeTypeBuffer(Int, int, 1);
MakeTypeBuffer(Int, pxr::GfVec2i, 2);
MakeTypeBuffer(Int, pxr::GfVec3i, 3);
MakeTypeBuffer(Int, pxr::GfVec4i, 4);

#undef MakeTypeBuffer

static SocketTypeInfo* make_socket_type_geometry()
{
    SocketTypeInfo* socktype = make_standard_socket_type(SocketType::Geometry);
    socktype->cpp_type = &CPPType::get<GOperandBase>();
    return socktype;
}

static SocketTypeInfo* make_socket_type_lights()
{
    SocketTypeInfo* socktype = make_standard_socket_type(SocketType::Lights);
    socktype->cpp_type = &CPPType::get<LightArray>();
    return socktype;
}

void register_socket(SocketTypeInfo* type_info)
{
    socket_registry[type_info->type_name] = std::unique_ptr<SocketTypeInfo>(type_info);
}

void register_sockets()
{
    register_socket(make_socket_type_int());
    register_socket(make_socket_type_float());
    register_socket(make_socket_type_Float1_buffer());
    register_socket(make_socket_type_Float2_buffer());
    register_socket(make_socket_type_Float3_buffer());
    register_socket(make_socket_type_Float4_buffer());
    register_socket(make_socket_type_Int1_buffer());
    register_socket(make_socket_type_Int2_buffer());
    register_socket(make_socket_type_Int3_buffer());
    register_socket(make_socket_type_Int4_buffer());
    register_socket(make_socket_type_string());
    register_socket(make_socket_type_geometry());
    register_socket(make_socket_type_lights());
}

void register_all()
{
    register_cpp_types();
    register_nodes();
    register_sockets();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
