#version 330 core

uniform highp sampler2D shadowMap;
uniform vec3 sunDirection;
uniform vec3 viewPos;
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
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float sum = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            sum += step(p.z - bias, texture(shadowMap, p.xy + vec2(x,y) * texel).r);
        }
    }
    return sum / 9.0;
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
    float sun = max(dot(normal, sunDirection), 0.0) * shadowVisibility(normal);
    vec3 color = bakedColor;
    if (water > 0.5) {
        vec2 p = surfacePos.xz;
        vec2 pond = (p - vec2(-1.0, -16.0)) / vec2(5.0, 4.0);
        if (dot(pond, pond) > 1.18 * 1.18) discard;
        float ripples = sin(p.x * 9.0 + p.y * 12.0) * cos(p.x * 6.0 - p.y * 9.0);
        float fresnel = pow(1.0 - clamp(dot(normal, towardEye), 0.0, 1.0), 4.0);
        float glint = pow(max(dot(reflect(-sunDirection, normal), towardEye), 0.0), 56.0);
        color = mix(color, vec3(0.42, 0.64, 0.71), fresnel) + ripples * 0.024
                + glint * vec3(0.43, 0.38, 0.22);
    } else {
        vec3 texturePoint = surfacePos * 23.0;
        vec3 footprint = fwidth(texturePoint);
        float detail = 1.0 - smoothstep(0.26, 0.98, max(footprint.x, max(footprint.y, footprint.z)));
        float textureDetail = grain(floor(texturePoint.xz) + floor(texturePoint.y) * vec2(5.13, 7.31));
        color *= (1.0 + (textureDetail - 0.5) * detail * 0.42)
               * (vec3(0.64, 0.72, 0.76) + sun * vec3(0.69, 0.59, 0.43));
    }
    float haze = 1.0 - exp(-distance * 0.006 - distance * distance * 0.00011);
    FragResult = vec4(mix(color, vec3(0.62, 0.73, 0.75), haze), 1.0);
}
