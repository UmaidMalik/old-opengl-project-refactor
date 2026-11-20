#version 330 core

uniform sampler2D panorama;
uniform int face;
in vec2 uv;
out vec4 FragResult;

void main() {
    vec2 p = uv * 2.0 - 1.0;
    vec3 ray;
    if (face == 0) ray = vec3(1, -p.y, -p.x);
    else if (face == 1) ray = vec3(-1, -p.y, p.x);
    else if (face == 2) ray = vec3(p.x, 1, p.y);
    else if (face == 3) ray = vec3(p.x, -1, -p.y);
    else if (face == 4) ray = vec3(p.x, -p.y, 1);
    else ray = vec3(-p.x, -p.y, -1);

    ray = normalize(ray);
    float latitude = acos(clamp(ray.y, -1.0, 1.0)) / 3.14159265;
    float heading = dot(ray.xz, ray.xz) > 1e-7 ? atan(ray.z, ray.x) / 6.28318530 : 0.0;
    FragResult = texture(panorama, vec2(fract(heading + 0.748), latitude));
}
