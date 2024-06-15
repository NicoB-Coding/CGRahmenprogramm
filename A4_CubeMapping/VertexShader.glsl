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

// Attribute 
in vec4 vVertex;
in vec3 vNormal;

uniform mat4 modelViewMatrix;  // Model-View-Matrix
uniform mat4 projectionMatrix; // Projektionsmatrix
uniform mat4 mvpMatrix;        // Model-View-Projection-Matrix
uniform mat3 normalMatrix;     // Normalmatrix
uniform mat4 textureMatrix;    // Texturmatrix

smooth out vec3 texCoords;

// Funktion zur Berechnung der Reflexionskoordinaten
vec3 ReflectionMap(const in vec3 eyePos, const in vec3 eyeNormal) {
    vec3 u = normalize(eyePos);
    return reflect(u, eyeNormal);
}

void main()
{
    // Transformiere den Scheitelpunkt in den Eye Space
    vec4 eyePos = modelViewMatrix * vVertex;
    vec3 eyeNormal = normalize(normalMatrix * vNormal); // Transformiere die Normalen in den Eye Space

    // Berechne die Reflexionskoordinaten
    vec4 reflectionCoords = vec4(ReflectionMap(eyePos.xyz, eyeNormal), 1.0);

    // Transformiere die Reflexionskoordinaten gemäß der Texture Matrix
    vec4 transformedCoords = textureMatrix * reflectionCoords;
    texCoords = transformedCoords.xyz;

    // Transformiere den Scheitelpunkt in den Clip Space
    gl_Position = mvpMatrix * vVertex;
}