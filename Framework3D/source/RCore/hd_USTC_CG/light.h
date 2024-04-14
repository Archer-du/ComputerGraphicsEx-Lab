#pragma once
#include "USTC_CG.h"
#include "color.h"
#include "pxr/imaging/hd/light.h"
#include "pxr/pxr.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;
class Hd_USTC_CG_Light : public HdLight {
   public:
    explicit Hd_USTC_CG_Light(const SdfPath& id, const TfToken& lightType)
        : HdLight(id),
          _lightType(lightType)
    {
    }

    void Sync(HdSceneDelegate* sceneDelegate, HdRenderParam* renderParam, HdDirtyBits* dirtyBits)
        override;
    HdDirtyBits GetInitialDirtyBitsMask() const override;
    virtual Color Sample(
        const GfVec3f& pos,
        GfVec3f& dir,
        float& sample_light_pdf,
        const std::function<float()>& uniform_float) = 0;
    virtual Color Intersect(const GfRay& ray, float& depth) = 0;

   protected:
    VtValue Get(TfToken const& token) const;
    // Stores the internal light type of this light. Of course, we can use polymorphism to do this.
    // But let's just keep it simple here.
    TfToken _lightType;
    // Cached states.
    TfHashMap<TfToken, VtValue, TfToken::HashFunctor> _params;
};

class Hd_USTC_CG_Sphere_Light : public Hd_USTC_CG_Light {
   public:
    Hd_USTC_CG_Sphere_Light(const SdfPath& id, const TfToken& lightType)
        : Hd_USTC_CG_Light(id, lightType)
    {
    }

    Color Sample(
        const GfVec3f& pos,
        GfVec3f& dir,
        float& sample_light_pdf,
        const std::function<float()>& uniform_float) override;
    Color Intersect(const GfRay& ray, float& depth) override;
    void Sync(
        HdSceneDelegate* sceneDelegate,
        HdRenderParam* renderParam,
        HdDirtyBits* dirtyBits) override;
    float radius;
    GfVec3f power;
    GfVec3f position;
    float area;
    GfVec3f irradiance;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
