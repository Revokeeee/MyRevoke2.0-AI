#type vertex
#version 450
			
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in int a_EntityID;

uniform mat4 u_PVmatrix;	

out vec2 v_TexCoord;
out vec4 v_Color;
out flat float v_TexIndex;
out flat int v_EntityID;

void main()
{
	v_TexIndex = a_TexIndex;
	v_TexCoord = a_TexCoord;
	v_Color = a_Color;
	v_EntityID = a_EntityID;
	gl_Position = u_PVmatrix * vec4(a_Pos, 1.0);
}


#type fragment
#version 450
 
layout(location = 0) out vec4 color;
layout(location = 1) out int pixelIDs;

in vec2 v_TexCoord;
in vec4 v_Color;
in flat float v_TexIndex;
in flat int v_EntityID;

uniform sampler2D u_Textures[32];

void main()
{
	color = texture(u_Textures[int(v_TexIndex)], v_TexCoord * 1.0) * v_Color;

	pixelIDs = v_EntityID;
}
