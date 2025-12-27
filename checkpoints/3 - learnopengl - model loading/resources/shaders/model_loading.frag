#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    result += CalcPointLight(pointLights[0], norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +  light.quadratic * (distance * distance));    
    
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    
	ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
	
    return (ambient + diffuse + specular);
}