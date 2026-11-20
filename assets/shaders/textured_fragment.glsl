#version 330 core

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D texture_1;
uniform highp sampler2D shadowMap;
uniform float ambientStrength;
uniform float light_cutoff_outer;
uniform float light_cutoff_inner;
uniform bool drawTexture;
uniform bool outdoorLight;
uniform bool stoneTiles;

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
    if (!outdoorLight) return step(position.z - bias, texture(shadowMap, position.xy).r);
    bias = max(0.00055 * (1.0 - dot(normal, towardLight)), 0.00016);
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float visibilitySum = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            visibilitySum += step(position.z - bias, texture(shadowMap, position.xy + vec2(x, y) * texel).r);
        }
    }
    return visibilitySum / 9.0;
}

void main()
{
    vec3 lighting = ambientStrength * lightColor;
    if (dot(Normal, Normal) > 0.0 && ambientStrength < 1.0) {
        vec3 normal = normalize(Normal);
        vec3 towardLight = outdoorLight ? -lightDir : normalize(lightPos - FragPos);
        float diffuse = (outdoorLight ? 0.55 : 2.9) * max(dot(normal, towardLight), 0.0);
        vec3 reflected = reflect(-towardLight, normal);
        float specular = (outdoorLight ? 0.14 : 0.4) * pow(max(dot(reflected, normalize(viewPos - FragPos)), 0.0), 32.0);
        float cone = outdoorLight ? 1.0 : smoothstep(light_cutoff_outer, light_cutoff_inner, dot(-towardLight, lightDir));
        vec3 directColor = outdoorLight ? vec3(1.0, 0.94, 0.81) : lightColor;
        lighting += visibility(normal, towardLight) * cone * (diffuse + specular) * directColor;
    }
    vec4 surface = drawTexture ? texture(texture_1, vertexUV) : vec4(1.0);
    if (stoneTiles && drawTexture) {
        float tone = dot(surface.rgb, vec3(0.2126, 0.7152, 0.0722));
        surface.rgb = mix(vec3(0.28, 0.25, 0.20), vec3(0.62, 0.58, 0.49), tone);
    }
    FragResult = surface * vec4(lighting * objectColor, 1.0);
}
