#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 skyTint;   // color from day/night cycle
uniform float tintStrength; // 0 = no tint, 1 = full tint

void main() {
    vec3 texColor = texture(skybox, TexCoords).rgb;
    vec3 result = mix(texColor, texColor * skyTint, tintStrength);
    FragColor = vec4(result, 1.0);
}