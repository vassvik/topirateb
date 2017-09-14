#version 330 core

out vec2 uv;

void main()
{
	float x = gl_VertexID / 2;
	float y = gl_VertexID % 2;
	uv = vec2(x, y);
    gl_Position = vec4(2.0*uv - 1.0, 0.0, 1.0);
}
