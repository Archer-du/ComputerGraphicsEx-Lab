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

    // Sample the lights.
    GfVec3f wi;
    float sample_light_pdf;
    GfVec3f sampled_light_pos;
    auto sample_light_luminance =
        SampleLights(si.position, wi, sampled_light_pos, sample_light_pdf, uniform_float);
    auto brdfVal = si.Eval(wi);
    GfVec3f contribution_by_sample_lights{ 0 };

    if (this->VisibilityTest(si.position + 0.0001f * si.geometricNormal, sampled_light_pos)) {
        contribution_by_sample_lights = GfCompMult(sample_light_luminance, brdfVal) *
                                        abs(GfDot(si.shadingNormal, wi)) / sample_light_pdf;
    }

    return contribution_by_sample_lights;

    // Estimate global lighting here.
    GfVec3f globalLight{ 0 };
    //GfRay reflectedRay = si.SpawnReflectedRay();
    //globalLight += 1 * EstimateOutGoingRadiance(reflectedRay, uniform_float, recursion_depth + 1);
    GfVec3f indirectLight{ 0 };
    for (int i = 0; i < spp; ++i) {
        float pdf;
        GfVec3f sampleDir;
        si.Sample(sampleDir, pdf, uniform_float);
        GfRay sampleRay(si.position, sampleDir);
        indirectLight += EstimateOutGoingRadiance(sampleRay, uniform_float, recursion_depth + 1);
    }
    globalLight += indirectLight / static_cast<float>(spp);



    //if (si.material->IsReflective()) {
    //    GfRay reflectedRay = si.SpawnReflectedRay();
    //    globalLight += si.material->GetReflectance() *
    //                   EstimateOutGoingRadiance(reflectedRay, uniform_float, recursion_depth + 1);
    //}
    //if (si.material->IsTransmissive()) {
    //    GfRay refractedRay = si.SpawnRefractedRay();
    //    globalLight += si.material->GetTransmittance() *
    //                   EstimateOutGoingRadiance(refractedRay, uniform_float, recursion_depth + 1);
    //}

    // Add indirect lighting via hemisphere sampling for diffuse surfaces
    //if (si.material->IsDiffuse()) {
    //    GfVec3f indirectLight{ 0 };
    //    for (int i = 0; i < si.numSamples; ++i) {
    //        GfVec3f sampleDir = SampleHemisphere(si.shadingNormal, uniform_float);
    //        GfRay sampleRay(si.point, sampleDir);
    //        indirectLight +=
    //            EstimateOutGoingRadiance(sampleRay, uniform_float, recursion_depth + 1);
    //    }
    //    globalLight += indirectLight / float(si.numSamples);
    //}

    color = directLight + globalLight;

    return color;
}


USTC_CG_NAMESPACE_CLOSE_SCOPE
