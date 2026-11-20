#version 330 core

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec3 color;
layout (location=3) in float waterTag;

uniform mat4 worldMatrix;
uniform mat4 viewProjection;
uniform mat4 lightSpace;
out vec3 surfacePos;
out vec3 worldPos;
out vec3 surfaceNormal;
out vec3 bakedColor;
out vec4 shadowPos;
out float water;

void main() {
    vec4 p = worldMatrix * vec4(position, 1);
    surfacePos = position;
    worldPos = p.xyz;
    surfaceNormal = mat3(worldMatrix) * normal;
    bakedColor = color;
    shadowPos = lightSpace * p;
    water = waterTag;
    gl_Position = viewProjection * p;
}
