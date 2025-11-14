#version 450

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D text;
layout(binding = 1) uniform TextParams {
    vec3 textColor;
    float padding;
} params;

void main() {
    float alpha = texture(text, TexCoords).r;
    color = vec4(params.textColor, alpha);
}