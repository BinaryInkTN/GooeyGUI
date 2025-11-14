#version 450

layout(location = 0) in vec4 vertex;

layout(location = 0) out vec2 TexCoords;

layout(binding = 1) uniform TextMatrices {
    mat4 projection;
} matrices;

void main() {
    gl_Position = matrices.projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vec2(vertex.z, 1.0 - vertex.w);
}