//
// Angepasst für Core Profile
// ---------------------------------
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule München, HM
//
//---------------------------------
#version 130
in vec4 color;
out vec4 fragColor;
// Sampler für die Textur hinzufügen
uniform sampler2D texture1;
// Texturkoordinaten vom Vertex-Shader
in vec2 TexCoord;

void main()
{
	// Texturfarbe abfragen
	vec4 texColor = texture(texture1, TexCoord); // NEU: Texturfarbe abfragen

	// Kombiniere die Texturfarbe mit der berechneten Farbe
	fragColor = color * texColor; // NEU: Texturfarbe mit der berechneten Farbe kombinieren
}
