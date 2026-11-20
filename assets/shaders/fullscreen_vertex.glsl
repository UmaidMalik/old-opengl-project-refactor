#version 330 core

out vec2 uv;

void main()
{
    // Clockwise so the triangle also draws when the scene has face culling enabled.
    const vec2 corners[3] = vec2[3](vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));
    vec2 position = corners[gl_VertexID];
    uv = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
