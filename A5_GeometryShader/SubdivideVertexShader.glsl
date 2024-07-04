#version 400

//Vertex Attribute
layout (location = 0) in vec4 vVertex;
layout (location = 1) in vec4 vColor;

//Vertex Daten für den Geometry Shader angeben
out vec4 color;
out vec4 position;

void main(void)
{
	color = vColor; 
	//Hier nichts transformieren, es wird später bei der Darstellung im Vertexshader transformiert.
	position = vVertex; 
}