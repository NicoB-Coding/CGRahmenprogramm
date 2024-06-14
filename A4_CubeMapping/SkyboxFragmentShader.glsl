//
// Angepasst für Core Profile
// ---------------------------------
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule München, HM
//
// ---------------------------------
#version 130


smooth in vec3 texCoords;
out vec4 fragColor;
//Sampler für die Cube Map
uniform samplerCube cubeMap;
void main()
{
	//hier auf textur zugreifen
	fragColor = texture(cubeMap, texCoords);

}