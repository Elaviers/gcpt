#version 410
uniform mat4 M_Model;
uniform mat4 M_View;
uniform mat4 M_Projection;

uniform vec2 UVOffset;
uniform vec2 UVScale;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV_IN;
layout(location = 2) in vec3 VertexColour_IN;

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 VertexColour;

void main()
{
	UV = (UV_IN * UVScale) + UVOffset;
	VertexColour = VertexColour_IN;
	gl_Position = M_Projection * M_View * M_Model * vec4(Position, 1);
}
