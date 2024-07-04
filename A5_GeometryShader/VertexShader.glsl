#version 400

//Vertex Attribute
in vec4 vVertex;
in vec4 vColor;

out vec4 color;
uniform mat4 mvpMatrix;

void main(void)
{
	color = vColor; 
	gl_Position = mvpMatrix* vVertex;
}