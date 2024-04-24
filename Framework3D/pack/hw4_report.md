# HW7: report

---

> 姓名：杜朋澈
>
> ID：68
>
> 学号：PB21050988

[TOC]

------

## 算法

### 极小曲面计算

#### uniform weight

固定边界点坐标，取均匀权重下的 $\boldsymbol{\delta} _ i = \boldsymbol{0}$ 即

$$ \frac{1}{d _ i} \sum _ {j\in N(i)} (\boldsymbol{v} _ i - \boldsymbol{v} _ j) = \boldsymbol{0}, \quad \text{for all interior } i .$$



#### cotangent weight

$w _ j = \cot \alpha _ {ij} + \cot \beta _ {ij}$



### Tutte 参数化计算

分布边界点的坐标到平面凸区域的边界，求解同样的方程组：

$$ \boldsymbol{v _ i} - \sum _ {j \in N(i)} w _ j  \boldsymbol{v _ j} = \boldsymbol{0}, \quad \text{for all interior } i .$$​



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
      }
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

  由于后续求解与坐标填充过程相同，故此处仅给出A的填充方式

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
      }
      A.makeCompressed();
      solve_transform(A, vertex_num, halfedge_mesh);
  
    // This can be customized : Do we want to see the lights? (Other than dome lights?)
    if (recursion_depth == 0) {
          }
          else {
              double sum_weight = 0.0;
              for (const auto& out_halfedge : vertex_handle.outgoing_halfedges()) {
                  double weight = 0.0;
                  
                  //获取neighbor，以及与neighbor和self共面的两个点
                  int neighbor_idx = out_halfedge.to().idx();
                  auto vi_idx = out_halfedge.prev().from().idx();
                  auto vj_idx = out_halfedge.opp().next().to().idx();
  
                  //获取这些点的位置即可计算向量夹角，进而计算权重
                  auto pos_self = origin_mesh->point(origin_mesh->vertex_handle(idx));
                  auto pos_neighbor = origin_mesh->point(origin_mesh->vertex_handle(neighbor_idx));
                  auto i_pos = origin_mesh->point(origin_mesh->vertex_handle(vi_idx));
                  auto j_pos = origin_mesh->point(origin_mesh->vertex_handle(vj_idx));
  
                  auto vec_1_1 = pos_self - i_pos;
                  auto vec_1_2 = pos_neighbor - i_pos;
                  auto vec_2_1 = pos_self - j_pos;
                  auto vec_2_2 = pos_neighbor - j_pos;
  
                  auto cos_theta_1 = vec_1_1.dot(vec_1_2) / (vec_1_1.norm() * vec_1_2.norm());
                  auto cos_theta_2 = vec_2_1.dot(vec_2_2) / (vec_2_1.norm() * vec_2_2.norm());
  
                  auto cot_theta_1 = cos_theta_1 / (std::sqrt(1 - cos_theta_1 * cos_theta_1));
                  auto cot_theta_2 = cos_theta_2 / (std::sqrt(1 - cos_theta_2 * cos_theta_2));
  
                  weight = cot_theta_1 + cot_theta_2;
  
    // Flip the normal if opposite
    if (GfDot(si.shadingNormal, ray.GetDirection()) > 0) {
        si.flipNormal();
        si.PrepareTransforms();
      }
      A.makeCompressed();
      solve_transform(A, vertex_num, halfedge_mesh);
  
  ```

- mapping node

  由于获取边界半边索引的过程是相同的，故此处仅给出将边界映射到指定形状的代码：

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
