//
// The gouraud vertex shader.
// ---------------------------------
// Angepasst für Core Profile
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule München, HM
//
// ---------------------------------
#version 130
//Attribute 
in vec4 vVertex;
in vec3 vNormal;

uniform mat4 mvpMatrix;
out vec4 color;
//Texturkoordinaten an Fragmentshader übergeben
//CubeMap benötigt 3 Texturkoordinaten
smooth out vec3 texCoords;
uniform vec4 ObjectPlaneS;
uniform vec4 ObjectPlaneT;
uniform vec4 ObjectPlaneR;

void main()
{
    gl_Position = mvpMatrix * vVertex;

    // Texcoords für Cube sind in Weltkoordinaten
	texCoords = vec3(
		dot(vVertex, ObjectPlaneS),
		dot(vVertex, ObjectPlaneT),
		dot(vVertex, ObjectPlaneR)
	);
}