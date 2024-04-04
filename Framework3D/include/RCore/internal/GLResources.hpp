#pragma once

#include "USTC_CG.h"
#include "pxr/base/gf/vec2i.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/hd/types.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

#define RESOURCE_LIST Texture

struct TextureDesc {
    pxr::GfVec2i size;
    pxr::HdFormat format;

    friend bool operator==(const TextureDesc& lhs, const TextureDesc& rhs)
    {
        return lhs.size == rhs.size && lhs.format == rhs.format;
    }

    friend bool operator!=(const TextureDesc& lhs, const TextureDesc& rhs)
    {
        return !(lhs == rhs);
    }
};

struct TextureResource {
    TextureDesc desc;
    GLuint texture_id;

    ~TextureResource()
    {
        glDeleteTextures(1, &texture_id);
    }
};

using TextureHandle = std::shared_ptr<TextureResource>;

TextureHandle createTexture(const TextureDesc& desc);

#define DESC_HANDLE_TRAIT(RESOURCE)        \
    template<>                             \
    struct ResouceDesc<RESOURCE##Handle> { \
        using Desc = RESOURCE##Desc;       \
    };

#define HANDLE_DESC_TRAIT(RESOURCE)        \
    template<>                             \
    struct DescResouce<RESOURCE##Desc> {   \
        using Resource = RESOURCE##Handle; \
    };

template<typename RESOURCE>
struct ResouceDesc {
    using Desc = void;
};

template<typename DESC>
struct DescResouce {
    using Resource = void;
};

GLenum GetGLFormat(pxr::HdFormat hd_format);
GLenum GetGLType(pxr::HdFormat hd_format);
USTC_CG_NAMESPACE_CLOSE_SCOPE