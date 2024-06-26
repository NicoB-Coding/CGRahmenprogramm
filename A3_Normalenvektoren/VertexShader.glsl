//
// The gouraud vertex shader.
// ---------------------------------
// Angepasst f�r Core Profile
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule M�nchen, HM
//
// ---------------------------------
#version 130
//Attribute 
in vec4 vVertex;
in vec4 vNormal;
// Texturkoordinaten Attribut hinzuf�gen
in vec2 vTexCoord;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
/// Light position in Eye-Space
uniform vec4 light_pos_vs;
/// Light diffuse color
uniform vec4 light_diffuse;
/// Light specular color
uniform vec4 light_specular;
/// Specular power (shininess)
uniform float spec_power;

/// Material parameters
uniform vec4 emissive_color;
uniform vec4 ambient_color;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;

//�bergabe an den Fragment-Shader
out vec4 color;
// �bergabe der Texturkoordinaten an den Fragment-Shader
out vec2 TexCoord;

void main()
{
	// Transformiere Vertex von Objekt- in den Clip-Space
	gl_Position = mvpMatrix * vVertex;

	// Transformiere Vertex von Objekt in den Eye-Space
	vec4 vertex_vs = mvMatrix * vVertex;
	vec3 ecPos = vertex_vs.xyz / vertex_vs.w;

	// Berechne Lichtrichtung in Eye-Space abh�ngig von der w-Komponente der Lichtposition
	vec3 light_dir_vs;
	if (light_pos_vs.w == 0.0) {
		// Richtungslichtquelle (w == 0)
		light_dir_vs = normalize(light_pos_vs.xyz);
	}
	else {
		// Punktlichtquelle (w != 0)
		light_dir_vs = normalize(light_pos_vs.xyz - ecPos);
	}

	// Normalen von Objekt- in Eye-Space transformieren
	vec3 normal_vs = normalize(normalMatrix * vNormal.xyz);

	// Betrachtervektor in Eye-Space
	vec3 view_dir_vs = normalize(-ecPos);

	// Halfway Vektor f�r das Phong-Blinn Beleuchtungsmodell berechnen
	vec3 halfway_vs = normalize(view_dir_vs + light_dir_vs);

	// Diffusen Term berechnen
	float NdotL = max(dot(normal_vs, light_dir_vs), 0.0);
	vec4 diffuse_color = NdotL * mat_diffuse * light_diffuse;

	// Spekularen Term berechnen
	float NdotH = max(dot(normal_vs, halfway_vs), 0.0);
	vec4 specular_color = pow(NdotH, spec_power) * mat_specular * light_specular;

	// Alle Farben addieren
	color = emissive_color + ambient_color + diffuse_color + specular_color;

	// �bergabe der Texturkoordinaten
	TexCoord = vTexCoord; // NEU: Texturkoordinaten an den Fragment-Shader �bergeben
}
