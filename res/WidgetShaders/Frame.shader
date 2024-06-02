#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform vec2 u_Resolution;
uniform vec2 u_Center;
uniform float u_Rotation;

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
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

layout(origin_upper_left) in vec4 gl_FragCoord;

uniform float u_Alpha;
uniform vec4 u_Bounds;

uniform vec3 u_Color;

void main()
{
	bool boundsCheck = gl_FragCoord.x > u_Bounds.x && gl_FragCoord.y > u_Bounds.y && gl_FragCoord.x < u_Bounds.z && gl_FragCoord.y < u_Bounds.w;
	
	fragColor = vec4(u_Color, u_Alpha * float(int(boundsCheck)));
};