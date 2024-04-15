# HW6: report

---

> 姓名：杜朋澈
>
> ID：68
>
> 学号：PB21050988

[TOC]

------

## 实现

### Blinn-Phong光照模型

fragment shader代码：

```glsl
//blinn-phong
float k_ambi = 0.4;
float k_spec = metal * 0.8;
float k_diff = 1 - k_spec;

float shinness = (1 - roughness) * 16;

for(int i = 0; i < light_count; i ++) {
    vec3 ambient = lights[i].color * k_ambi * texture(diffuseColorSampler, uv).xyz;

    vec3 lightDir = normalize(lights[i].position - pos);
    vec3 diffuse = lights[i].color * k_diff * max(dot(normal, lightDir), 0.0) * texture(diffuseColorSampler, uv).xyz;

    vec3 viewDir = normalize(camPos - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = lights[i].color * k_spec * pow(max(dot(normal, halfwayDir), 0.0), shinness) * texture(diffuseColorSampler, uv).xyz;

    vec3 result = ambient + diffuse + specular;
    Color += vec4(result, 1.0);
}
```

### 法线贴图处理

fragment shader代码:

```glsl
position = vertexPosition;
vec4 clipPos = projection * view * (vec4(position, 1.0));
depth = clipPos.z / clipPos.w;
texcoords = vTexcoord;

diffuseColor = texture2D(diffuseColorSampler, vTexcoord).xyz;
metallicRoughness = texture2D(metallicRoughnessSampler, vTexcoord).zy;

vec3 normalmap_value = texture2D(normalMapSampler, vTexcoord).xyz;
normal = normalize(vertexNormal);

// Calculate tangent and bitangent
vec3 edge1 = dFdx(vertexPosition);
vec3 edge2 = dFdy(vertexPosition);
vec2 deltaUV1 = dFdx(vTexcoord);
vec2 deltaUV2 = dFdy(vTexcoord);

vec3 tangent = edge1 * deltaUV2.y - edge2 * deltaUV1.y;

// Robust tangent and bitangent evaluation
if(length(tangent) < 1E-7) {
    vec3 bitangent = -edge1 * deltaUV2.x + edge2 * deltaUV1.x;
    tangent = normalize(cross(bitangent, normal));
}
tangent = normalize(tangent - dot(tangent, normal) * normal);
vec3 bitangent = normalize(cross(tangent,normal));

// 构造TBN矩阵，将切线空间法向变换到世界坐标系
mat3 TBN = mat3(tangent, bitangent, normal);
normal = normalize(TBN * normalize(normalmap_value * 2.0 - 1.0));
```

![image-20240415115950691](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240415115950691.png)

### Shadow Mapping

在Blinn-Phong基础上，根据光源空间深度缓冲纹理与观测到的片段在光源空间的深度的比对结果判断阴影。基础代码如下：

```glsl
    vec3 ambient = k_ambi * lights[i].color * texture(diffuseColorSampler, uv).xyz;

    vec3 lightDir = normalize(lights[i].position - pos);
    vec3 diffuse = lights[i].color * k_diff * max(dot(normal, lightDir), 0.0) * texture(diffuseColorSampler, uv).xyz;

    vec3 viewDir = normalize(camPos - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = lights[i].color * k_spec * pow(max(dot(normal, halfwayDir), 0.0), shinness) * texture(diffuseColorSampler, uv).xyz;

	// 计算片段的光源坐标
    vec4 lightPos = lights[i].light_projection * lights[i].light_view * vec4(pos, 1.0);
	// 执行透视除法
    vec3 projCoords = lightPos.xyz / lightPos.w;
	
	// 获取深度
    float currentDepth = projCoords.z;
	// 将xy分量映射到0，1空间内
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadow_maps, vec3(projCoords.xy, lights[i].shadow_map_id)).x;
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

    vec3 result = ambient + (1.0 - shadow * 0.8) * (diffuse + specular);
    Color += vec4(result, 1.0);
```

#### 抗失真

因为阴影贴图受限于分辨率，在距离光源比较远的情况下，多个片段可能从深度贴图的同一个值中去采样。图片每个斜坡代表深度贴图一个单独的纹理像素。你可以看到，多个片段从同一个深度值进行采样。

虽然很多时候没问题，但是当光源以一个角度朝向表面的时候就会出问题，这种情况下深度贴图也是从一个角度下进行渲染的。多个片段就会从同一个斜坡的深度纹理像素中采样，有些在地板上面，有些在地板下面；这样我们所得到的阴影就有了差异。因为这个，有些片段被认为是在阴影之中，有些不在，由此产生了图片中的条纹样式。

我们可以用一个叫做**阴影偏移**（shadow bias）的技巧来解决这个问题，我们对表面的深度应用一个偏移量，这样片段就不会被错误地认为在表面之下了，并且很容易注意到，当光照方向与阴影采样点法向之间的夹角越大，越容易出现这种失真，所以夹角越大，偏移量就应该越大：

```glsl
float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.005);
```

![img](https://learnopengl-cn.github.io/img/05/03/01/shadow_mapping_acne_diagram.png)

#### 过采样

注意到此时画面中有异常阴影，这是由于两方面的原因导致的：

- 光源深度纹理所能覆盖的场景范围有限，超出的纹理范围的可视片段在变换到光源空间进行采样时会受到纹理环绕方式影响。
- 点光源的frustrum大小有限，光源投影空间远截平面的深度永远为1.0，但超出该范围的可视片段会被错误的认为处于阴影中。

![0802e56779c5dc9814ec0eaca2ebd612](D:\Tencent\QQNT\Downloads\Tencent Files\1580148433\nt_qq\nt_data\Pic\2024-04\Ori\0802e56779c5dc9814ec0eaca2ebd612.png)

![image-20240414211515425](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240414211515425.png)

对于第一种情况，在CPU端对shadow map纹理环绕方式进行修改即可：

```c++
    shader->shader.setInt("shadow_maps", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_maps->texture_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//将超出纹理范围的部分深度设置为1.0，这样超出部分将永远不会处于阴影中
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
```

对于第二种情况，需要对可视片段在光源空间中的深度进行判断，若深度大于1.0，则说明该片段超出了光源投影空间的最远视距，直接将该片段的阴影系数设置为0，即永远不处于阴影中即可。

```glsl
    if(projCoords.z > 1.0)
        shadow = 0.0;
```

#### PCSS

因为深度贴图有一个固定的分辨率，多个片段对应于一个纹理像素。结果就是多个片段会从深度贴图的同一个深度值进行采样，这几个片段便得到的是同一个阴影，这就会产生锯齿边。

采用PCF（percentage-closer filtering）对锯齿进行简单处理。这是一种多个不同过滤方式的组合，它能够柔和阴影，使它们出现更少的锯齿块和硬边。核心思想是从深度贴图中多次采样，每一次采样的纹理坐标都稍有不同。每个独立的样本可能在也可能不再阴影中。所有的次生结果接着结合在一起，进行平均化，我们就得到了柔和阴影。

一个简单的PCF的实现是简单的从纹理像素四周对深度贴图采样，然后把结果平均起来：

```glsl
    vec3 ambient = k_ambi * lights[i].color * texture(diffuseColorSampler, uv).xyz;

    vec3 lightDir = normalize(lights[i].position - pos);
    vec3 diffuse = lights[i].color * k_diff * max(dot(normal, lightDir), 0.0) * texture(diffuseColorSampler, uv).xyz;

    vec3 viewDir = normalize(camPos - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = lights[i].color * k_spec * pow(max(dot(normal, halfwayDir), 0.0), shinness) * texture(diffuseColorSampler, uv).xyz;

    vec4 lightPos = lights[i].light_projection * lights[i].light_view * vec4(pos, 1.0);
    vec3 projCoords = lightPos.xyz / lightPos.w;

    float bias = max(0.2 * (1.0 - dot(normal, lightDir)), 0.005);
    float currentDepth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;
    vec2 texelSize = (1.0 / textureSize(shadow_maps, 0)).xy;
    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadow_maps, vec3(projCoords.xy + vec2(x, y) * texelSize, lights[i].shadow_map_id)).x;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;

    vec3 result = ambient + (1.0 - shadow * 0.8) * (diffuse + specular);
    Color += vec4(result, 1.0);
```

![image-20240415110308631](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240415110308631.png)

多光源下的效果较为优异。

此外，从PCF的结果可以看出，我们在减少阴影锯齿的同时，产生了一种软阴影的效果。实际上在现实生活中，由于绝大部分的光源是面光源，我们见到的更多都是软阴影。

很自然的就能想到，当我们在Shadow Map上选择查询范围的时候，范围越大，其结果过渡的越平滑，阴影也就越软。因此PCSS的关键在于如何选择合适的查询范围。

```glsl
    vec3 ambient = k_ambi * lights[i].color * texture(diffuseColorSampler, uv).xyz;

    vec3 lightDir = normalize(lights[i].position - pos);
    vec3 diffuse = lights[i].color * k_diff * max(dot(normal, lightDir), 0.0) * texture(diffuseColorSampler, uv).xyz;

    vec3 viewDir = normalize(camPos - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = lights[i].color * k_spec * pow(max(dot(normal, halfwayDir), 0.0), shinness) * texture(diffuseColorSampler, uv).xyz;

    vec4 lightPos = lights[i].light_projection * lights[i].light_view * vec4(pos, 1.0);
    vec3 projCoords = lightPos.xyz / lightPos.w;

    float bias = max(0.2 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    float currentDepth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;
    float blockerDepth = BlockerSearch(i, projCoords.xy, currentDepth);
    if (blockerDepth > 0.0) {
        float penumbraSize = (currentDepth - blockerDepth) * (5 * lights[i].radius * 5 * lights[i].radius) / blockerDepth;
        vec2 texelSize = (penumbraSize / textureSize(shadow_maps, 0)).xy;
        // 使用更大的采样区域进行PCF
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float closestDepth = texture(shadow_maps, vec3(projCoords.xy + vec2(x, y) * texelSize, lights[i].shadow_map_id)).x;
                shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }

    if(projCoords.z > 1.0)
        shadow = 0.0;

    vec3 result = ambient + (1.0 - shadow * 0.8) * (diffuse + specular);
    Color += vec4(result, 1.0);
```

其中，`float penumbraSize = (currentDepth - blockerDepth) * (5 * lights[i].radius * 5 * lights[i].radius) / blockerDepth;`用于计算伪影大小（此处的系数5是用于对放缩灯光半径从而使软阴影效果更明显，实际`(5 * lights[i].radius * 5 * lights[i].radius)`表示的即为灯光大小），计算blockerDepth的方法为：

```glsl
float BlockerSearch(int light_id, vec2 uv, float receiverDepth) {
    vec2 searchWidth = (5 * lights[light_id].radius / textureSize(shadow_maps, 0)).xy;
    float avgBlockerDepth = 0.0;
    int blockers = 0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float sampleDepth = texture(shadow_maps, vec3(uv + vec2(x, y) * searchWidth, lights[light_id].shadow_map_id)).x;
            if (sampleDepth < receiverDepth) {
                avgBlockerDepth += sampleDepth;
                blockers++;
            }
        }
    }
    if (blockers == 0) return 0.0;
    return avgBlockerDepth / float(blockers);
}
```



![image-20240415115031300](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240415115031300.png)
