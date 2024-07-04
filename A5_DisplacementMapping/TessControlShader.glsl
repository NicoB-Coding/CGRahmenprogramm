#version 400

layout(vertices = 3) out;

in vec3 vPosTC[];
in vec2 vTexCoordTC[];

uniform float tessLevelInner = 2.0;
uniform float tessLevelOuter = 2.0;

out vec3 vPosTE[];
out vec2 vTexCoordTE[];

void main ()
{
	//Tesselationsparameter für Tesselator setzen
    gl_TessLevelInner[0] = tessLevelInner;
    gl_TessLevelOuter[0] = tessLevelOuter;
    gl_TessLevelOuter[1] = tessLevelOuter;
    gl_TessLevelOuter[2] = tessLevelOuter;

	//Attribute kopieren
    vPosTE[gl_InvocationID] = vPosTC[gl_InvocationID];
    vTexCoordTE[gl_InvocationID] = vTexCoordTC[gl_InvocationID];
}
