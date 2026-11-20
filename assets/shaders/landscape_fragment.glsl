#version 330 core
#include "weather.glsl"

uniform highp sampler2D shadowMap;
uniform vec3 sunDirection;
uniform vec3 viewPos;
uniform vec3 terrainCamera;
uniform samplerCube skyMap;
uniform float time;
in vec3 surfacePos;
in vec3 worldPos;
in vec3 surfaceNormal;
in vec3 bakedColor;
in vec4 shadowPos;
in float water;
out vec4 FragResult;

float shadowVisibility(vec3 normal) {
    vec3 p = shadowPos.xyz / shadowPos.w * 0.5 + 0.5;
    if (any(lessThan(p, vec3(0.0))) || any(greaterThan(p, vec3(1.0)))) return 1.0;
    float bias = max(0.00055 * (1.0 - dot(normal, sunDirection)), 0.00016);
    float blocker = texture(shadowMap, p.xy).r;
    float spread = clamp(0.9 + (p.z - blocker) * 110.0, 0.9, 2.6);
    vec2 texel = spread / vec2(textureSize(shadowMap, 0));
    float sum = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            sum += step(p.z - bias, texture(shadowMap, p.xy + vec2(x,y) * texel).r);
        }
    }
    float edge = min(min(p.x, p.y), min(1.0 - p.x, 1.0 - p.y));
    return mix(1.0, sum / 9.0, smoothstep(0.0, 0.11, edge));
}

float grain(vec2 p) {
    vec3 q = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    q += dot(q, q.yzx + 33.33);
    return fract((q.x + q.y) * q.z);
}

void main() {
    vec3 normal = normalize(surfaceNormal);
    float distance = length(viewPos - worldPos);
    vec3 towardEye = normalize(viewPos - worldPos);
    float cloudLight = daylight(surfacePos);
    float sun = max(dot(normal, sunDirection), 0.0) * shadowVisibility(normal) * cloudLight;
    vec3 color = bakedColor;
    if (water > 0.5) {
        vec2 p = surfacePos.xz;
        vec2 pond = (p - vec2(-1.0, -16.0)) / vec2(5.0, 4.0);
        if (dot(pond, pond) > 1.18 * 1.18) discard;
        float waves = sin(dot(p, vec2(3.1, 4.8)) + time * 1.2);
        float swell = cos(dot(p, vec2(0.83, 2.9)) - time * 0.88);
        float waveX = 0.64 * waves + 0.36 * sin(dot(p, vec2(7.3, -3.4)) + time * 1.8);
        float waveZ = 0.53 * swell + 0.47 * cos(dot(p, vec2(5.2, -6.5)) - time);
        vec3 rippleNormal = normalize(normal + vec3(waveX * 0.026, 0.0, waveZ * 0.026));
        float fresnel = pow(1.0 - clamp(dot(rippleNormal, towardEye), 0.0, 1.0), 4.0);
        float glint = pow(max(dot(reflect(-sunDirection, rippleNormal), towardEye), 0.0), 155.0);
        vec3 terrainRay = normalize(surfacePos - terrainCamera);
        vec3 lakeNormal = normalize(vec3(waveX * 0.026, 1.0, waveZ * 0.026));
        vec3 reflectedSky = textureLod(skyMap, reflect(terrainRay, lakeNormal), 2.1).rgb;
        color = mix(color, reflectedSky, 0.16 + fresnel * 0.73) + waveX * waveZ * 0.004
                + glint * vec3(0.96, 0.88, 0.75) * shadowVisibility(normal) * cloudLight;
    } else {
        vec3 texturePoint = surfacePos * 23.0;
        vec3 footprint = fwidth(texturePoint);
        float detail = 1.0 - smoothstep(0.26, 0.98, max(footprint.x, max(footprint.y, footprint.z)));
        float textureDetail = grain(floor(texturePoint.xz) + floor(texturePoint.y) * vec2(5.13, 7.31));
        float skyExposure = normal.y * 0.5 + 0.5;
        vec3 indirect = mix(vec3(0.46, 0.43, 0.37), vec3(0.66, 0.73, 0.81), skyExposure);
        vec3 direct = sun * vec3(0.91, 0.83, 0.69);
        color *= (1.0 + (textureDetail - 0.5) * detail * 0.35) * (indirect + direct);
    }
    vec3 ray = normalize(surfacePos - terrainCamera);
    vec3 air = textureLod(skyMap, ray, 1.0).rgb;
    float haze = max(1.0 - exp(-distance * 0.004 - distance * distance * 0.000085),
                     smoothstep(53.0, 71.0, distance));
    FragResult = vec4(mix(color, air, haze), 1.0);
}
