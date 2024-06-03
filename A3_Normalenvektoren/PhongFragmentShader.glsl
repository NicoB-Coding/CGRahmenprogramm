#version 130

// Eingaben vom Vertex-Shader
smooth in vec3 Position;
smooth in vec3 Normal;
in vec4 ambientEmissiveColor; // Ambient- und Emissionsfarbe

// Uniforms
uniform vec4 light_pos_vs;
uniform vec4 light_diffuse;
uniform vec4 light_specular;
uniform float spec_power;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;

// Ausgabe-Farbe
out vec4 fragColor;

void main()
{
    // Normalisiere den interpolierten Normalenvektor
    vec3 normal = normalize(Normal);

    // Berechne die Lichtrichtung
    vec3 light_dir_vs;
    if (light_pos_vs.w == 0.0) {
        // Richtungslichtquelle (w == 0)
        light_dir_vs = normalize(light_pos_vs.xyz);
    }
    else {
        // Punktlichtquelle (w != 0)
        light_dir_vs = normalize(light_pos_vs.xyz - Position);
    }

    // Berechne die Blickrichtung (angenommen der Betrachter ist am Ursprung)
    vec3 view_dir_vs = normalize(-Position);

    // *** ÄNDERUNG: Berechne den Reflexionsvektor für die Phong-Methode ***
    vec3 reflect_dir_vs = reflect(-light_dir_vs, normal);

    // Berechne den diffusen Term
    float NdotL = max(dot(normal, light_dir_vs), 0.0);
    vec4 diffuse_color = NdotL * mat_diffuse * light_diffuse;

    // *** ÄNDERUNG: Berechne den spekularen Term nach der Phong-Methode ***
    float RdotV = max(dot(reflect_dir_vs, view_dir_vs), 0.0);
    vec4 specular_color = pow(RdotV, spec_power) * mat_specular * light_specular;

    // Kombiniere alle Farben
    fragColor = ambientEmissiveColor + diffuse_color + specular_color;
}