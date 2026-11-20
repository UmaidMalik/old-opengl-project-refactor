#version 330 core

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D texture_1;
uniform sampler2D shadowMap;
uniform float ambientStrength;
uniform float light_cutoff_outer;
uniform float light_cutoff_inner;
uniform bool drawTexture;

in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec3 Normal;
in vec2 vertexUV;

out vec4 FragResult;

float visibility(vec3 normal, vec3 towardLight)
{
    if (FragPosLightSpace.w <= 0.0) return 1.0;
    vec3 position = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;
    if (any(lessThan(position, vec3(0.0))) || any(greaterThan(position, vec3(1.0)))) return 1.0;
    float bias = max(0.003 * (1.0 - dot(normal, towardLight)), 0.0005);
    return step(position.z - bias, texture(shadowMap, position.xy).r);
}

void main()
{
    vec3 lighting = ambientStrength * lightColor;
    if (dot(Normal, Normal) > 0.0 && ambientStrength < 1.0) {
        vec3 normal = normalize(Normal);
        vec3 towardLight = normalize(lightPos - FragPos);
        float diffuse = 2.9 * max(dot(normal, towardLight), 0.0);
        vec3 reflected = reflect(-towardLight, normal);
        float specular = 0.4 * pow(max(dot(reflected, normalize(viewPos - FragPos)), 0.0), 32.0);
        float cone = smoothstep(light_cutoff_outer, light_cutoff_inner, dot(-towardLight, lightDir));
        lighting += visibility(normal, towardLight) * cone * (diffuse + specular) * lightColor;
    }
    vec4 surface = drawTexture ? texture(texture_1, vertexUV) : vec4(1.0);
    FragResult = surface * vec4(lighting * objectColor, 1.0);
}
