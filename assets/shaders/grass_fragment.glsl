#version 330 core
#include "weather.glsl"

uniform highp sampler2D shadowMap;
uniform vec3 sunDirection;
in vec4 shadowPos;
in vec3 bladeNormal;
in float bladeHeight;
in float variation;
in float distanceToCamera;
in vec3 terrainRoot;
out vec4 FragResult;

float sunVisibility() {
    vec3 p = shadowPos.xyz / shadowPos.w * 0.5 + 0.5;
    if (any(lessThan(p, vec3(0.0))) || any(greaterThan(p, vec3(1.0)))) return 1.0;
    vec2 texel = 1.1 / vec2(textureSize(shadowMap, 0));
    float sum = 0.0;
    for (int y = -1; y <= 1; y += 2) {
        for (int x = -1; x <= 1; x += 2) {
            sum += step(p.z - 0.00045, texture(shadowMap, p.xy + vec2(x,y) * texel).r);
        }
    }
    return sum * 0.25;
}

void main() {
    vec3 green = mix(vec3(0.14, 0.23, 0.091), vec3(0.33, 0.44, 0.17), variation);
    float faceLight = 0.37 + 0.63 * abs(dot(normalize(bladeNormal), sunDirection));
    vec3 sky = vec3(0.66, 0.73, 0.81);
    vec3 sun = vec3(0.92, 0.84, 0.70) * faceLight * sunVisibility() * daylight(terrainRoot);
    float rootShade = mix(0.57, 1.1, smoothstep(0.0, 0.89, bladeHeight));
    vec3 color = green * (sky + sun) * rootShade;
    float haze = 1.0 - exp(-distanceToCamera * 0.008);
    FragResult = vec4(mix(color, vec3(0.62, 0.73, 0.75), haze), 1.0);
}
