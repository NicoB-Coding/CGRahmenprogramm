#version 400

layout(triangles) in;
//TODO: Maximale Anzahl an ausgegebenen Vertices anpassen
layout(triangle_strip, max_vertices = 3) out;

in vec4 position[];
in vec4 color[];

out vec4 positionOut;
out vec4 colorOut;


void main() {
	//Simple pass through
	for (int i=0; i<3;i++) {
		//Hier nichts transformieren, es wird später bei der Darstellung im Vertexshader transformiert.
		positionOut = position[i];
		colorOut = color[i];
		EmitVertex();
	}
	EndPrimitive();

}