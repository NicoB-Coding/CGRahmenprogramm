//
// The gouraud vertex shader.
// ---------------------------------
// Angepasst f�r Core Profile
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule M�nchen, HM
//
// ---------------------------------
#version 130
//Attribute 
in vec4 vVertex;
in vec3 vNormal;

uniform mat4 mvpMatrix;
out vec4 color;
//Texturkoordinaten an Fragmentshader �bergeben
//CubeMap ben�tigt 3 Texturkoordinaten
smooth out vec3 texCoords;
uniform vec4 ObjectPlaneS;
uniform vec4 ObjectPlaneT;
uniform vec4 ObjectPlaneR;

void main()
{	
	gl_Position = mvpMatrix * vVertex ;
	//TODO texcoords bestimmen.
	texCoords = vec3(
		dot(gl_Position, ObjectPlaneS),
		dot(gl_Position, ObjectPlaneT),
		dot(gl_Position, ObjectPlaneR)
	);
}