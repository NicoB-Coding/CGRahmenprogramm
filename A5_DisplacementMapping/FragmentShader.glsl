#version 400

uniform vec4 lightColorAmbient;
uniform vec4 lightColorSpecular;
uniform vec4 lightColorDiffuse;
uniform float specPow;

uniform vec3 lightPos;

uniform sampler2D diffuseMap;
uniform sampler2D bumpMap;

in vec2 vTexCoordFS;
in vec3 vNormalFS;
in vec3 vPosFS;

out vec4 vFragColor;

void main(void)
{
    vec3 normal = normalize(vNormalFS);
    vec3 lightDir = normalize(lightPos - vPosFS);
    vec3 viewDir = normalize(-vPosFS); // assuming camera is at origin

    // Ambient component
    vec4 ambient = lightColorAmbient;

    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec4 diffuse = diff * lightColorDiffuse;

    // Specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specPow);
    vec4 specular = spec * lightColorSpecular;

    // Texture color
    vec4 texColor = texture(diffuseMap, vTexCoordFS);

    // Combine all components
    vFragColor = texColor * (ambient + diffuse + specular);
}
