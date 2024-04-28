#include "path.h"

#include <random>

#include "surfaceInteraction.h"
#include "utils/sampling.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

VtValue PathIntegrator::Li(const GfRay& ray, std::default_random_engine& random)
{
    std::uniform_real_distribution<float> uniform_dist(
        0.0f, 1.0f - std::numeric_limits<float>::epsilon());
    std::function<float()> uniform_float = std::bind(uniform_dist, random);

    auto color = EstimateOutGoingRadiance(ray, uniform_float, 0);

    return VtValue(GfVec3f(color[0], color[1], color[2]));
}

GfVec3f PathIntegrator::EstimateOutGoingRadiance(
    const GfRay& ray,
    const std::function<float()>& uniform_float,
    int recursion_depth)
{
    if (recursion_depth >= 50) {
        return {};
    }

    SurfaceInteraction si;
    if (!Intersect(ray, si)) {
        if (recursion_depth == 0) {
            return IntersectDomeLight(ray);
        }

        return GfVec3f{ 0, 0, 0 };
    }

    // This can be customized : Do we want to see the lights? (Other than dome lights?)
    if (recursion_depth == 0) {
    }

    // Flip the normal if opposite
    if (GfDot(si.shadingNormal, ray.GetDirection()) > 0) {
        si.flipNormal();
        si.PrepareTransforms();
    }


    GfVec3f color{ 0 };
    GfVec3f directLight = EstimateDirectLight(si, uniform_float);

    // Estimate global lighting here.
    GfVec3f globalLight{ 0 };

    float p_RR = 0.6;
    float ksi = uniform_float();
    if (ksi > p_RR)
        return directLight;

    float pdf;
    auto sampleDir = UniformSampleHemiSphere(GfVec2f(uniform_float(), uniform_float()), pdf);
    auto worldSampleDir = si.TangentToWorld(sampleDir);

    GfRay sampleRay(si.position + 0.0001f * si.geometricNormal, worldSampleDir);
    auto brdfVal = si.Eval(-worldSampleDir);

    globalLight = GfCompMult(
            EstimateOutGoingRadiance(sampleRay, uniform_float, recursion_depth + 1), brdfVal) *
        abs(GfDot(si.shadingNormal, - worldSampleDir)) / pdf / p_RR;

    color = directLight + globalLight;

    return color;
}


USTC_CG_NAMESPACE_CLOSE_SCOPE
