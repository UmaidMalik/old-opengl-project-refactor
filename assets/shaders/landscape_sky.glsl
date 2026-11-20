#version 330 core
#include "weather.glsl"

uniform mat4 clipToTerrain;
uniform samplerCube skyMap;
in vec2 uv;
out vec4 FragResult;

vec3 unproject(vec2 ndc, float depth) {
    vec4 point = clipToTerrain * vec4(ndc, depth, 1.0);
    return point.xyz / point.w;
}

vec3 sampleCloudLayer(vec3 origin, vec3 ray, vec3 background) {
    if (abs(ray.y) < 0.018) return background;
    float a = (cloudBase - origin.y) / ray.y;
    float b = (cloudTop - origin.y) / ray.y;
    float start = max(0.0, min(a, b));
    float stop = min(410.0, max(a, b));
    if (stop <= start) return background;

    const int samples = 12;
    float stride = (stop - start) / float(samples);
    // Stagger the ray samples to avoid visible bands along shallow sightlines.
    float jitter = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    vec3 cloud = vec3(0);
    float transmission = 1.0;
    float scattering = pow(max(dot(ray, terrainSun), 0.0), 10.0);
    for (int i = 0; i < samples; ++i) {
        float dist = start + stride * (float(i) + mix(0.18, 0.82, jitter));
        vec3 p = origin + ray * dist;
        vec2 local = cloudCoordinates(p.xz);
        float layer = clamp((p.y - cloudBase) / (cloudTop - cloudBase), 0.0, 1.0);
        float profile = 4.0 * layer * (1.0 - layer);
        float shape = cloudField(local + vec2(0.24, -0.18) * layer);
        float erosion = weatherNoise(local * 5.6 + vec2(layer * 1.8, layer * 0.8));
        float density = smoothstep(0.52, 0.74, shape - (1.0 - profile) * 0.29 - erosion * 0.063);
        float haze = 1.0 - smoothstep(170.0, 390.0, dist);
        float opacity = (1.0 - exp(-density * stride * 0.11)) * haze;
        float upperShade = cloudField(local + terrainSun.xz * 0.26);
        float illumination = clamp(0.26 + layer * 0.57 + (shape - upperShade) * 1.7, 0.0, 1.0);
        vec3 color = mix(vec3(0.58, 0.64, 0.73), vec3(0.97, 0.95, 0.91), illumination);
        color += vec3(0.06, 0.046, 0.028) * scattering;
        cloud += transmission * opacity * color;
        transmission *= 1.0 - opacity;
        if (transmission < 0.025) break;
    }
    return cloud + background * transmission;
}

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec3 origin = unproject(ndc, -1.0);
    vec3 ray = normalize(unproject(ndc, 0.99) - origin);
    vec3 sky = texture(skyMap, ray).rgb;
    float sun = max(dot(ray, terrainSun), 0.0);
    float pixel = max(fwidth(sun), 0.000001);
    float disk = smoothstep(0.999989 - pixel, 0.999989 + pixel, sun);
    vec3 glare = vec3(1.0, 0.86, 0.69) * (0.15 * pow(sun, 110.0) + 0.38 * pow(sun, 950.0));
    sky = mix(sky + glare, vec3(1.0, 0.98, 0.94), disk);
    FragResult = vec4(clamp(sampleCloudLayer(origin, ray, sky), 0.0, 1.0), 1.0);
}
