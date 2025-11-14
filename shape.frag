#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform ShapeParams {
    bool useTexture;
    bool isRounded;
    bool isHollow;
    int shapeType;
    float radius;
    float borderWidth;
    vec2 size;
    vec2 padding;
} params;

float roundedBoxSDF(vec2 p, vec2 b, float r) {
    // Signed Distance Function for rounded box
    vec2 d = abs(p) - b + vec2(r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
}

void main() {
    vec2 center = vec2(0.5, 0.5);
    vec2 pos = fragTexCoord - center;
    vec2 halfSize = params.size * 0.5;

    // Early discard for fully transparent pixels (avoids unnecessary calculations)
    if (params.useTexture) {
        outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0);
        if (outColor.a == 0.0) discard;
        return;
    }

    if (params.isRounded) {
        // Calculate distance from current position to the edges of a rounded box
        float distance = roundedBoxSDF(pos, halfSize, params.radius);

        if (params.isHollow) {
            // Hollow rounded rectangle (border)
            float alpha = 1.0 - smoothstep(-0.01, 0.01, abs(distance) - params.borderWidth);
            outColor = vec4(fragColor, alpha);
        } else {
            // Filled rounded rectangle
            float alpha = 1.0 - smoothstep(-0.01, 0.01, distance);
            outColor = vec4(fragColor, alpha);
        }
    } else {
        // Regular rectangle
        if (params.isHollow) {
            // Hollow rectangle (border)
            vec2 innerHalfSize = halfSize - vec2(params.borderWidth);  // Adjusted scaling without *0.01
            bool inOuter = abs(pos.x) <= halfSize.x && abs(pos.y) <= halfSize.y;
            bool inInner = abs(pos.x) <= innerHalfSize.x && abs(pos.y) <= innerHalfSize.y;
            float alpha = inOuter && !inInner ? 1.0 : 0.0;
            outColor = vec4(fragColor, alpha);
        } else {
            // Filled rectangle
            outColor = vec4(fragColor, 1.0);
        }
    }

    // Early discard for transparent pixels (no need to proceed further if fully transparent)
    if (outColor.a == 0.0) {
        discard;
    }
}
