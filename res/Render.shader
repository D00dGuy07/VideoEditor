#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
	gl_Position = position;
	v_TexCoords = texCoords;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
	vec4 color = texture(u_Texture, v_TexCoords);
	fragColor = vec4(color.x, color.y, color.z, 1.0f);
};