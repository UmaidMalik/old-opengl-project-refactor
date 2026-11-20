#version 330 core

layout (location=0) in vec4 clump;
layout (location=1) in vec2 blade;
uniform mat4 worldMatrix;
uniform mat4 viewProjection;
uniform mat4 lightSpace;
uniform vec3 terrainCamera;
uniform float time;
uniform float drawRange;
out vec4 shadowPos;
out vec3 bladeNormal;
out float bladeHeight;
out float variation;
out float distanceToCamera;

void main() {
    int stalk = gl_VertexID / 9;
    float seed = fract(clump.w * 5.13 + float(stalk) * 0.387);
    float angle = clump.w * 6.283 + float(stalk) * 2.399;
    vec2 right = vec2(cos(angle), sin(angle));
    vec2 forward = vec2(-right.y, right.x);
    float distance = length(clump.xyz - terrainCamera);
    float fade = 1.0 - smoothstep(drawRange * 0.67, drawRange, distance);
    float height = mix(0.23, 0.64, seed) * fade;
    float width = mix(0.022, 0.042, seed) * fade;
    float t = blade.y;
    float gust = sin(time * 1.8 + dot(clump.xz, vec2(0.36, 0.24)))
               + 0.42 * sin(time * 3.1 + clump.x * 0.71 - clump.z * 0.55);
    vec2 bend = forward * (0.13 + seed * 0.24) + vec2(0.83, 0.55) * (0.20 + gust * 0.13);
    vec2 position = clump.xz + forward * (0.08 * float(stalk) * fade)
                  + right * blade.x * width + bend * t * t * height;
    vec4 world = worldMatrix * vec4(position.x, clump.y + height * t, position.y, 1.0);
    gl_Position = viewProjection * world;
    shadowPos = lightSpace * world;
    bladeNormal = normalize(mat3(worldMatrix) * vec3(forward.x, 0.35, forward.y));
    bladeHeight = t;
    variation = seed;
    distanceToCamera = distance;
}
