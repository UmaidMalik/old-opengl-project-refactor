#version 330 core

uniform mat4 clipToTerrain;
uniform mat4 terrainToClip;
uniform vec3 sunDirection;

in vec2 uv;
out vec4 FragResult;

const float viewRange = 145.0;
const float forestRange = 92.0;
const float cellSize = 5.0;
const float waterLevel = -0.22;

struct TreeHit {
    float distance;
    vec3 normal;
    bool leaves;
};

float hash21(vec2 p)
{
    vec3 q = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    q += dot(q, q.yzx + 33.33);
    return fract((q.x + q.y) * q.z);
}

float noise2(vec2 p)
{
    vec2 tile = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(tile), hash21(tile + vec2(1, 0)), f.x),
               mix(hash21(tile + vec2(0, 1)), hash21(tile + vec2(1, 1)), f.x), f.y);
}

float pondRadius(vec2 p)
{
    return length((p - vec2(-1.0, -16.0)) / vec2(5.0, 4.0));
}

float groundHeight(vec2 p)
{
    float clearing = smoothstep(5.25, 12.5, max(abs(p.x), abs(p.y)));
    float hills = 5.0 * noise2(p * 0.083 + 6.2) - 1.55;
    hills += 1.0 * noise2(p * 0.24 + 14.7);
    float mountains = smoothstep(22.0, 100.0, length(p));
    hills += mountains * (3.0 + 11.0 * noise2(p * 0.049 - 3.0));
    float height = -0.35 + clearing * hills;
    if (abs(p.x + 1.0) < 6.5 && abs(p.y + 16.0) < 5.2) {
        height = mix(-1.6, height, smoothstep(0.69, 1.27, pondRadius(p)));
    }
    return height;
}

vec3 groundNormal(vec2 p)
{
    const float e = 0.12;
    return normalize(vec3(groundHeight(p - vec2(e, 0)) - groundHeight(p + vec2(e, 0)), 2.0 * e,
                          groundHeight(p - vec2(0, e)) - groundHeight(p + vec2(0, e))));
}

float traceGround(vec3 origin, vec3 ray, float limit)
{
    float distance = 0.05;
    float previous = 0.0;
    for (int i = 0; i < 132; ++i) {
        vec3 p = origin + ray * distance;
        float clearance = p.y - groundHeight(p.xz);
        if (clearance < 0.0) {
            // Refine the crossing for stable depth where the terrain meets the tiled platform.
            for (int step = 0; step < 5; ++step) {
                float middle = 0.5 * (previous + distance);
                vec3 samplePoint = origin + ray * middle;
                if (samplePoint.y < groundHeight(samplePoint.xz)) distance = middle;
                else previous = middle;
            }
            return distance;
        }
        if (clearance < 0.025 + distance * 0.0004) return distance;
        previous = distance;
        // Larger minimum steps keep grazing rays moving across distant hills.
        distance += clamp(clearance * 0.52, 0.10 + distance * 0.003, 7.0);
        if (distance >= limit || (p.y > 22.0 && ray.y >= 0.0)) break;
    }
    return limit;
}

void hitSide(float distance, vec3 p, vec3 ray, float height, float slope,
             bool leaves, inout TreeHit hit)
{
    if (distance <= 0.035 || distance >= hit.distance) return;
    vec3 q = p + ray * distance;
    if (q.y < 0.0 || q.y > height) return;
    hit = TreeHit(distance, normalize(vec3(q.x, slope * length(q.xz), q.z)), leaves);
}

// Intersect a capped cone (or a cylinder when the taper is zero).
void hitCone(vec3 origin, vec3 ray, vec3 base, float height, float radius,
             bool leaves, inout TreeHit hit)
{
    vec3 p = origin - base;
    float slope = leaves ? radius / height : 0.0;
    float startRadius = radius - slope * p.y;
    float radiusRate = -slope * ray.y;
    float a = dot(ray.xz, ray.xz) - radiusRate * radiusRate;
    float b = dot(p.xz, ray.xz) - startRadius * radiusRate;
    float c = dot(p.xz, p.xz) - startRadius * startRadius;
    float discriminant = b * b - a * c;
    if (abs(a) < 0.00001) {
        if (abs(b) > 0.00001) hitSide(-c / (2.0 * b), p, ray, height, slope, leaves, hit);
    } else if (discriminant >= 0.0) {
        float root = sqrt(discriminant);
        float t0 = (-b - root) / a;
        float t1 = (-b + root) / a;
        hitSide(min(t0, t1), p, ray, height, slope, leaves, hit);
        hitSide(max(t0, t1), p, ray, height, slope, leaves, hit);
    }
    if (abs(ray.y) > 0.00001) {
        float cap = -p.y / ray.y;
        vec2 point = p.xz + ray.xz * cap;
        if (cap > 0.035 && cap < hit.distance && dot(point, point) <= radius * radius) {
            hit = TreeHit(cap, vec3(0, -1, 0), leaves);
        }
    }
}

TreeHit traceTrees(vec3 origin, vec3 ray, float limit)
{
    // One tree fits entirely inside a cell. Traverse cells along the ray in entry order.
    vec2 cell = floor(origin.xz / cellSize);
    vec2 direction = step(vec2(0), ray.xz) * 2.0 - 1.0;
    vec2 inverse = 1.0 / max(abs(ray.xz), vec2(0.00001));
    vec2 boundary = ((cell + max(direction, vec2(0))) * cellSize - origin.xz) * direction * inverse;
    vec2 stride = cellSize * inverse;
    float entry = 0.0;
    for (int i = 0; i < 30; ++i) {
        if (entry >= limit) break;
        float exitDistance = min(boundary.x, boundary.y);
        float seed = hash21(cell + 27.3);
        if (seed > 0.40) {
            vec2 jitter = vec2(hash21(cell + 17.0), hash21(cell - 63.0)) - 0.5;
            vec2 center = (cell + 0.5) * cellSize + jitter * 1.8;
            if (max(abs(center.x), abs(center.y)) > 7.6 && pondRadius(center) > 1.16) {
                float scale = mix(0.68, 1.27, hash21(cell - 3.0));
                float height = 4.1 * scale;
                vec3 base = vec3(center.x, groundHeight(center) - 0.05, center.y);
                vec3 sphere = base + vec3(0, height * 0.5, 0);
                float closest = clamp(dot(sphere - origin, ray), entry, min(limit, exitDistance));
                vec3 offset = origin + ray * closest - sphere;
                if (dot(offset, offset) < height * height * 0.32) {
                    TreeHit tree = TreeHit(limit, vec3(0), false);
                    hitCone(origin, ray, base, height * 0.65, 0.12 * scale, false, tree);
                    hitCone(origin, ray, base + vec3(0, height * 0.19, 0), height * 0.55, 1.28 * scale, true, tree);
                    hitCone(origin, ray, base + vec3(0, height * 0.36, 0), height * 0.48, 1.02 * scale, true, tree);
                    hitCone(origin, ray, base + vec3(0, height * 0.53, 0), height * 0.47, 0.74 * scale, true, tree);
                    if (tree.distance < limit) return tree;
                }
            }
        }

        entry = exitDistance;
        if (boundary.x < boundary.y) {
            cell.x += direction.x;
            boundary.x += stride.x;
        } else {
            cell.y += direction.y;
            boundary.y += stride.y;
        }
    }
    return TreeHit(limit, vec3(0), false);
}

vec3 skyColor(vec3 ray)
{
    float elevation = max(ray.y, 0.0);
    vec3 color = mix(vec3(0.80, 0.86, 0.83), vec3(0.24, 0.52, 0.76), smoothstep(0.0, 0.46, elevation));
    float sun = max(dot(ray, sunDirection), 0.0);
    color += vec3(0.23, 0.12, 0.031) * pow(sun, 24.0);
    color = mix(color, vec3(1.0, 0.94, 0.68), smoothstep(0.99945, 0.99972, sun));

    if (ray.y > 0.015) {
        vec2 clouds = ray.xz * (0.8 / (ray.y + 0.18)) + vec2(18.5, 4.0);
        float cloud = noise2(clouds) * 0.65 + noise2(clouds * 2.4) * 0.24 + noise2(clouds * 6.2) * 0.11;
        float coverage = smoothstep(0.58, 0.74, cloud) * smoothstep(0.015, 0.12, ray.y);
        color = mix(color, vec3(0.91, 0.92, 0.87), coverage * 0.75);
    }
    return color;
}

vec3 fog(vec3 color, vec3 ray, float distance)
{
    vec3 air = mix(vec3(0.54, 0.69, 0.73), skyColor(vec3(ray.x, 0, ray.z)), 0.35);
    float haze = 1.0 - exp(-distance * 0.006 - distance * distance * 0.00004);
    return mix(color, air, haze);
}

vec3 turfColor(vec3 p, vec3 normal, float distance)
{
    float fields = noise2(p.xz * 0.38) * 0.76 + noise2(p.xz * 1.1) * 0.24;
    vec3 grass = mix(vec3(0.113, 0.235, 0.071), vec3(0.39, 0.475, 0.157), fields);
    float details = 1.0 - smoothstep(12.0, 62.0, distance);
    float blades = noise2(p.xz * vec2(16.0, 24.0));
    grass *= 1.0 + details * (blades - 0.5) * 0.45;
    float seeds = smoothstep(0.87, 0.98, blades) * smoothstep(0.45, 0.62, fields);
    grass = mix(grass, vec3(0.60, 0.55, 0.24), seeds * details * 0.58);
    float steep = 1.0 - smoothstep(0.58, 0.88, normal.y);
    grass = mix(grass, mix(vec3(0.29, 0.34, 0.28), vec3(0.47, 0.46, 0.32), fields), steep);
    if (pondRadius(p.xz) < 1.30) {
        float shore = (1.0 - smoothstep(0.045, 0.34, abs(p.y - waterLevel)));
        grass = mix(grass, vec3(0.40, 0.373, 0.257) * (0.83 + blades * 0.30), shore * 0.82);
    }
    return grass;
}

vec3 treeColor(vec3 p, TreeHit hit)
{
    vec3 base = hit.leaves ?
        mix(vec3(0.048, 0.135, 0.085), vec3(0.235, 0.358, 0.149),
            noise2(p.xz * 7.5 + p.y * 11.6)) :
        mix(vec3(0.14, 0.097, 0.061), vec3(0.32, 0.23, 0.139),
            noise2(p.xz * 19.0 + vec2(p.y * 0.24)));
    return base * (0.65 + 0.57 * max(dot(hit.normal, sunDirection), 0.0))
         + vec3(0.012, 0.020, 0.026) * max(hit.normal.y, 0.0);
}

bool platformBlocksSun(vec3 origin)
{
    vec3 low = (vec3(-5.0, -0.42, -5.0) - origin) / sunDirection;
    vec3 high = (vec3(5.0, 0.0, 5.0) - origin) / sunDirection;
    vec3 nearPoint = min(low, high);
    vec3 farPoint = max(low, high);
    float enter = max(max(nearPoint.x, nearPoint.y), nearPoint.z);
    float leave = min(min(farPoint.x, farPoint.y), farPoint.z);
    return leave >= max(enter, 0.03);
}

float sunVisibility(vec3 p, vec3 normal, float distance)
{
    if (distance > 95.0) return 1.0;
    vec3 start = p + normal * 0.075;
    if (platformBlocksSun(start)) return 0.11;
    float occlusion = 1.0;
    float travel = 0.45;
    for (int i = 0; i < 14; ++i) {
        vec3 q = start + sunDirection * travel;
        float clearance = q.y - groundHeight(q.xz);
        occlusion = min(occlusion, clamp(clearance * 8.0 / travel, 0.0, 1.0));
        travel += clamp(clearance, 0.6, 3.8);
        if (occlusion < 0.06 || travel > 27.0) break;
    }
    if (occlusion > 0.12 && distance < 70.0 && traceTrees(start, sunDirection, 22.0).distance < 22.0) {
        occlusion = 0.12;
    }
    return mix(0.08, 1.0, occlusion);
}

vec3 waterColor(vec3 p, vec3 ray)
{
    vec2 waves = p.xz;
    vec3 normal = normalize(vec3(sin(dot(waves, vec2(5.6, 3.4))) * 0.018, 1.0,
                                 cos(dot(waves, vec2(2.6, -7.3))) * 0.018));
    vec3 reflected = reflect(ray, normal);
    vec3 start = p + vec3(0, 0.055, 0);
    const float reflectionRange = 36.0;
    float land = traceGround(start, reflected, reflectionRange);
    TreeHit tree = traceTrees(start, reflected, land);
    vec3 reflection = skyColor(reflected);
    if (tree.distance < land) {
        reflection = fog(treeColor(start + reflected * tree.distance, tree), reflected, tree.distance);
    } else if (land < reflectionRange) {
        vec3 shore = start + reflected * land;
        reflection = fog(turfColor(shore, groundNormal(shore.xz), land) * 0.92, reflected, land);
    }
    float fresnel = 0.10 + 0.86 * pow(1.0 - max(dot(normal, -ray), 0.0), 4.0);
    float glint = pow(max(dot(reflected, sunDirection), 0.0), 230.0);
    return mix(vec3(0.075, 0.235, 0.236), reflection, fresnel)
         + glint * vec3(0.70, 0.52, 0.27);
}

vec3 unproject(vec2 ndc, float depth)
{
    vec4 p = clipToTerrain * vec4(ndc, depth, 1.0);
    return p.xyz / p.w;
}

void main()
{
    vec2 ndc = uv * 2.0 - 1.0;
    vec3 origin = unproject(ndc, -1.0);
    vec3 ray = normalize(unproject(ndc, 0.99) - origin);
    float land = traceGround(origin, ray, viewRange);

    float water = viewRange;
    if (ray.y < -0.0001) {
        float t = (waterLevel - origin.y) / ray.y;
        if (t > 0.0 && t < land) {
            vec2 p = (origin + ray * t).xz;
            if (pondRadius(p) < 1.17 && groundHeight(p) < waterLevel - 0.015) water = t;
        }
    }
    float surface = min(land, water);
    float treeLimit = min(surface, forestRange);
    TreeHit tree = traceTrees(origin, ray, treeLimit);
    float distance = surface;
    vec3 color;
    if (tree.distance < treeLimit) {
        distance = tree.distance;
        color = treeColor(origin + ray * distance, tree);
    } else if (water < land) {
        color = waterColor(origin + ray * water, ray);
    } else if (land < viewRange) {
        vec3 p = origin + ray * land;
        vec3 normal = groundNormal(p.xz);
        float diffuse = max(dot(normal, sunDirection), 0.0) * sunVisibility(p, normal, land);
        color = turfColor(p, normal, land) * (vec3(0.60, 0.68, 0.73) + diffuse * vec3(0.75, 0.68, 0.54));
    } else {
        // Fog closes the terrain horizon and the sky itself writes the far depth.
        FragResult = vec4(clamp(skyColor(ray), 0.0, 1.0), 1.0);
        gl_FragDepth = 1.0;
        return;
    }

    vec3 point = origin + ray * distance;
    vec4 clip = terrainToClip * vec4(point, 1.0);
    gl_FragDepth = clamp(0.5 + 0.5 * clip.z / clip.w, 0.0, 1.0);
    FragResult = vec4(clamp(fog(color, ray, distance), 0.0, 1.0), 1.0);
}
