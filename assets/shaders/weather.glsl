uniform vec3 terrainSun;
uniform float skyTime;

const float cloudBase = 42.0;
const float cloudTop = 58.0;

float weatherHash(vec2 p) {
    vec3 q = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    q += dot(q, q.yzx + 33.33);
    return fract((q.x + q.y) * q.z);
}

float weatherNoise(vec2 p) {
    vec2 tile = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(weatherHash(tile), weatherHash(tile + vec2(1, 0)), f.x),
               mix(weatherHash(tile + vec2(0, 1)), weatherHash(tile + 1.0), f.x), f.y);
}

vec2 cloudCoordinates(vec2 world) {
    return (world - skyTime * vec2(0.94, 0.41)) * 0.026 + vec2(13.1, 4.6);
}

float cloudField(vec2 p) {
    return weatherNoise(p) * 0.69 + weatherNoise(p * 2.12 + 7.2) * 0.31;
}

float daylight(vec3 position) {
    vec3 ray = terrainSun;
    float distance = (0.5 * (cloudBase + cloudTop) - position.y) / max(ray.y, 0.05);
    if (position.y > cloudTop) return 1.0;
    float cover = cloudField(cloudCoordinates(position.xz + ray.xz * max(distance, 0.0)));
    return mix(1.0, 0.58, smoothstep(0.51, 0.74, cover));
}
