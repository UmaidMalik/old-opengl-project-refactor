#version 330 core

uniform mat4 clipToTerrain;
uniform vec3 sunDirection;
in vec2 uv;
out vec4 FragResult;

vec3 unproject(vec2 ndc, float depth) {
    vec4 p = clipToTerrain * vec4(ndc, depth, 1.0);
    return p.xyz / p.w;
}

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec3 view = normalize(unproject(ndc, 0.99) - unproject(ndc, -1.0));
    vec3 sky = mix(vec3(0.62, 0.73, 0.75), vec3(0.24, 0.50, 0.76),
                   smoothstep(0.0, 0.48, view.y));
    float sun = max(dot(view, sunDirection), 0.0);
    sky += vec3(0.29, 0.16, 0.05) * pow(sun, 22.0);
    sky = mix(sky, vec3(1.0, 0.92, 0.68), smoothstep(0.9994, 0.99975, sun));
    FragResult = vec4(sky, 1.0);
}
