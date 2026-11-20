#version 330 core

uniform sampler2D landscapeColor;
uniform highp sampler2D landscapeDepth;
in vec2 uv;
out vec4 FragResult;

void main()
{
    FragResult = texture(landscapeColor, uv);
    gl_FragDepth = texture(landscapeDepth, uv).r;
}
