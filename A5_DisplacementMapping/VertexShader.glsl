#version 400

//Vertex Attribute
in vec4 vVertex;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vBinormal;
in vec3 vTangent;

//ModelViewProjection Matrix
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

out vec3 vPosTC;
out vec2 vTexCoordTC;


void main(void)
{
    vPosTC = vVertex.xyz;
    vTexCoordTC = vTexCoord;

	gl_Position = mvpMatrix * vVertex; 
}
