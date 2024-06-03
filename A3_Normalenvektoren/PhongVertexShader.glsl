#version 130

// Attribute
in vec4 vVertex;
in vec4 vNormal;

// Uniforms
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

// Smooth out Variablen für den Fragment-Shader
smooth out vec3 Position;
smooth out vec3 Normal;

// Farben werden an den Fragment-Shader übergeben
uniform vec4 emissive_color;
uniform vec4 ambient_color;
out vec4 ambientEmissiveColor;

void main()
{
    // Transformiere Vertex-Position in den Clip-Space
    gl_Position = mvpMatrix * vVertex;

    // Transformiere Vertex-Position in den Eye-Space
    vec4 vertex_vs = mvMatrix * vVertex;
    Position = vertex_vs.xyz / vertex_vs.w;

    // Transformiere Normalen in den Eye-Space
    Normal = normalize(normalMatrix * vNormal.xyz);

    // Übergabe der Ambient- und Emissionsfarbe an den Fragment-Shader
    ambientEmissiveColor = emissive_color + ambient_color;
}