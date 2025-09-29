#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;

struct Flashlight {
    bool enabled;
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

uniform Flashlight flashlight;

void main() {
    vec3 result = vec3(0.0);

    if (flashlight.enabled) {
        vec3 lightDir = normalize(flashlight.position - FragPos);
        float theta = dot(lightDir, normalize(-flashlight.direction));

        float epsilon   = flashlight.cutOff - flashlight.outerCutOff;
        float intensity = clamp((theta - flashlight.outerCutOff) / epsilon, 0.0, 1.0);

        float distance    = length(flashlight.position - FragPos);
        float attenuation = 1.0 / (flashlight.constant +
                                   flashlight.linear * distance +
                                   flashlight.quadratic * (distance * distance));

        vec3 ambient = flashlight.ambient;

        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = flashlight.diffuse * diff;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = flashlight.specular * spec;

        result = (ambient + (diffuse + specular) * intensity) * attenuation;
    }

    FragColor = vec4(result, 1.0);
}