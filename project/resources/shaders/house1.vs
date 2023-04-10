#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

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

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    DirLight dLight;
    PointLight pLight[NR_POINT_LIGHTS];
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;



uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLight;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = vec2(aTexCoords.x, aTexCoords.y);

    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.dLight.ambient = dirLight.ambient;
    vs_out.dLight.diffuse = dirLight.diffuse;
    vs_out.dLight.specular = dirLight.specular;
    vs_out.dLight.direction = TBN * dirLight.direction;

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vs_out.pLight[i].position = TBN * pointLights[i].position;
        vs_out.pLight[i].ambient = pointLights[i].ambient;
        vs_out.pLight[i].diffuse = pointLights[i].diffuse;
        vs_out.pLight[i].specular = pointLights[i].specular;
        vs_out.pLight[i].constant = pointLights[i].constant;
        vs_out.pLight[i].linear = pointLights[i].linear;
        vs_out.pLight[i].quadratic = pointLights[i].quadratic;
    }

    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}