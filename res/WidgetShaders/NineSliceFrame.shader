#shader vertex
#version 420 core

layout(location = 0) in vec4 position;

uniform vec2 u_Resolution;
uniform vec2 u_Center;
uniform float u_Rotation;

layout(std140, binding = 0) uniform instanceBuffer
{
	vec4[18] instanceData;
};

flat out vec4 v_TexRange;
out vec2 v_TexPosition;

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
	vec4 screenSpace = instanceData[gl_InstanceID * 3];
	vec4 texRect = instanceData[gl_InstanceID * 3 + 1];
	vec4 ranges = instanceData[gl_InstanceID * 3 + 2];

	// Texture calculations
	v_TexRange = vec4(texRect.x, texRect.x + texRect.z, texRect.y, texRect.y + texRect.w);
	v_TexPosition = ranges.xz + ranges.yw * position.xy;

	// Transformations
	vec2 pos = screenSpace.xy + screenSpace.zw * position.xy;
	pos = RotateAroundPivot(pos, u_Center, u_Rotation);
	pos = vec2(pos.x, u_Resolution.y - pos.y) / u_Resolution * 2.0f - 1.0f;

	gl_Position = vec4(pos.x, pos.y, 0.0f, position.w);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

layout(origin_upper_left) in vec4 gl_FragCoord;

flat in vec4 v_TexRange;
in vec2 v_TexPosition;

uniform float u_Alpha;
uniform vec4 u_Bounds;

uniform sampler2D u_Texture;
uniform vec2 u_TextureResolution;
uniform bool u_UseFetch;

void main()
{
	bool boundsCheck = gl_FragCoord.x > u_Bounds.x && gl_FragCoord.y > u_Bounds.y && gl_FragCoord.x < u_Bounds.z && gl_FragCoord.y < u_Bounds.w;
	
	vec2 actualPosition = (v_TexRange.yw - v_TexRange.xz) * vec2(mod(v_TexPosition.x, 1.0f), mod(v_TexPosition.y, 1.0f)) + v_TexRange.xz;

	vec4 texColor;
	if (u_UseFetch)
	{ texColor = texelFetch(u_Texture, ivec2(int(actualPosition.x), int(actualPosition.y)), 0); }
	else
	{ texColor = texture(u_Texture, actualPosition / u_TextureResolution); }

	fragColor = vec4(texColor.rgb, u_Alpha * texColor.a * float(int(boundsCheck)));
};