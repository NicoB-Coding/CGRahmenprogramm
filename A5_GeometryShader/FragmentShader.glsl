#version 400
//Interpolierte Eingabe von Vertex Shader
in vec4 color;
//Ausgabe
out vec4 vFragColor;

void main(void) {
    vFragColor = color;
}
