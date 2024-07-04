#version 400

layout(triangles, equal_spacing, ccw) in;

in vec3 vPosTE[];
in vec2 vTexCoordTE[];

uniform sampler2D displacementMap;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform float displacement;

// Lichtposition
uniform vec3 lightPos;

// Werte, die an den Fragment Shader übergeben werden
out vec2 vTexCoordFS;
out vec3 vNormalFS;
out vec3 vPosFS;

void main()
{
    // Interpolation der Vertex-Positionen und Texturkoordinaten
    vec3 pos = gl_TessCoord.x * vPosTE[0] + gl_TessCoord.y * vPosTE[1] + gl_TessCoord.z * vPosTE[2];
    vTexCoordFS = gl_TessCoord.x * vTexCoordTE[0] + gl_TessCoord.y * vTexCoordTE[1] + gl_TessCoord.z * vTexCoordTE[2];

    // Normale entspricht bei einer Einheitskugel der Position
    vec3 normal = normalize(pos);

    // Abfrage der Displacement-Werte
    float disp = texture(displacementMap, vTexCoordFS).r;

    // Verschieben im Object Space entlang der Normalen
    pos += normal * (disp * displacement - 0.5 * displacement);

    // Berechnung der Normalen im World Space
    vNormalFS = normalMatrix * normal;
    vPosFS = vec3(mvMatrix * vec4(pos, 1.0));

    // Transformiere von Object Space in Clip Space
    gl_Position = mvpMatrix * vec4(pos, 1.0);
}
