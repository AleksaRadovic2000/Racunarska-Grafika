#version 330 core
out vec4 FragColor;


struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

#define NR_POINT_LIGHTS 2

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    DirLight dLight;
    PointLight pLight[NR_POINT_LIGHTS];
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

uniform Material material;
uniform float heightScale;
uniform bool dan;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    // obtain normal from normal map
    vec3 normal = texture(material.texture_normal1, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // get diffuse color
//    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
//     vec3 ambient = 0.2 * color;
//     // diffuse
//     vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
//     float diff = max(dot(lightDir, normal), 0.0);
//     vec3 diffuse = diff * color;
//     // specular
//     vec3 reflectDir = reflect(-lightDir, normal);
//     vec3 halfwayDir = normalize(lightDir + viewDir);
//     float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
//
//     vec3 specular = (vec3(1.0)-texture(specMap, texCoords).rgb) * spec;
// //   vec3 specular = vec3(0.2)*spec;
//     FragColor = vec4(ambient + diffuse + specular, 1.0);

    vec3 result = vec3(0.0);
    if(dan){
        result += CalcDirLight(fs_in.dLight, normal, viewDir, texCoords);
    }else{
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(fs_in.pLight[i], normal, fs_in.TangentFragPos, viewDir, texCoords);

    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoords).rgb);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoords).rgb);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoords).rgb;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoords).rgb);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoords).rgb);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoords).rgb;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}