# HW7: report

---

> 姓名：杜朋澈
>
> ID：68
>
> 学号：PB21050988

[TOC]

------



## 实现

### 直接光照积分器

#### 矩形光源

- `Color Hd_USTC_CG_Rect_Light::Sample`

  ```c++
  Color Hd_USTC_CG_Rect_Light::Sample(
      const GfVec3f& pos,
      GfVec3f& dir,
      GfVec3f& sampled_light_pos,
      float& sample_light_pdf,
      const std::function<float()>& uniform_float)
  {
      float u = uniform_float();
      float v = uniform_float();
  
      sampled_light_pos = corner0 + u * (corner1 - corner0) + v * (corner3 - corner0);
  
      dir = (sampled_light_pos - pos).GetNormalized();
      float distance = (sampled_light_pos - pos).GetLength();
  
      float area = width * height;
  
      GfVec3f worldSampleDir = GfCross(corner1 - corner0, corner3 - corner0);
      float cosVal = GfDot(-dir, worldSampleDir.GetNormalized());
  
      sample_light_pdf = 1 / area;
      if (cosVal < 0) {
          return Color{ 0 };
      }
      return (irradiance * cosVal) / M_PI;
  }
  ```

- `Color Hd_USTC_CG_Rect_Light::Intersect`

  ```c++
  Color Hd_USTC_CG_Rect_Light::Intersect(const GfRay& ray, float& depth)
  {
      double distance;
      if (ray.Intersect(corner0, corner1, corner2, &distance)) {
          depth = distance;
  
          return irradiance / M_PI;
      }
      if (ray.Intersect(corner2, corner3, corner0, &distance)) {
          depth = distance;
  
          return irradiance / M_PI;
      }
      depth = std::numeric_limits<float>::infinity();
      return { 0, 0, 0 };
  }
  ```

  

### 路径追踪算法

`GfVec3f PathIntegrator::EstimateOutGoingRadiance`

```c++
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

    // Russian Roullete的实现
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
```



## 演示

> 均为 path integrator 在默认 sample per pixel（=256）的结果。

- DomeLight + SphereLight

![image-20240424141635890](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240424141635890.png)

- Sphere Light

![image-20240424142013180](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240424142013180.png)

- Rect Light（单侧发光）

![image-20240424143354625](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240424143354625.png)

- Sphere Light 在启用 Russian Roullete 时的表现

![image-20240424144616499](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240424144616499.png)
