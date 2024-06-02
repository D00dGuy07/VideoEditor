#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;

uniform vec2 u_Resolution;
uniform vec2 u_Center;
uniform float u_Rotation;

out vec2 v_TexCoords;

vec2 RotateAroundPivot(vec2 start, vec2 pivot, float theta)
{
	vec2 pivotSpace = start - pivot;
	vec2 rotatedPoint = vec2(
		pivotSpace.x * cos(theta) - pivotSpace.y * sin(theta),
		pivotSpace.y * cos(theta) + pivotSpace.x * sin(theta)
	);

	return rotatedPoint + pivot;
}

void main()
{
	// Transformations
	vec2 pos = RotateAroundPivot(position.xy, u_Center, u_Rotation);
	pos = vec2(pos.x, u_Resolution.y - pos.y) / u_Resolution * 2.0f - 1.0f;

	gl_Position = vec4(pos.x, pos.y, 0.0f, position.w);
	v_TexCoords = texCoords;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

layout(origin_upper_left) in vec4 gl_FragCoord;

in vec2 v_TexCoords;

uniform float u_Alpha;
uniform vec4 u_Bounds;

uniform vec3 u_Color;
uniform sampler2D u_TextureAtlas;

void main()
{
	bool boundsCheck = gl_FragCoord.x > u_Bounds.x && gl_FragCoord.y > u_Bounds.y && gl_FragCoord.x < u_Bounds.z && gl_FragCoord.y < u_Bounds.w;
	
	vec4 texColor = texelFetch(u_TextureAtlas, ivec2(v_TexCoords), 0);
	fragColor = vec4(texColor.rgb * u_Color, u_Alpha * texColor.a * float(int(boundsCheck)));
};