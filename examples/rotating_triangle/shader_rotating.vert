#version 450
layout(location = 0) out vec3 fragColor;

// Updated Push Constant Block
layout(push_constant) uniform Push {
    float time;
    float aspectRatio;
} push;

vec2 positions[3] = vec2[](
    vec2(-0.5, -0.2887),
    vec2(0.5, -0.2887),
    vec2(0, 0.5774)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    float c = cos(push.time);
    float s = sin(push.time);
    //float c = 1;
    //float s = 0;
    mat2 rot = mat2(c, -s, s, c);

    //float scale = 1.0 + sin(push.time * 3.0) * 0.2;
    float scale = 1;
    vec2 pos = scale * rot * positions[gl_VertexIndex];

    //pos.y += cos(push.time * 1.75) * 0.2;
    //pos.x -= sin(push.time * 2.0) * 0.25;

    pos.x /= push.aspectRatio;

    gl_Position = vec4(pos, 0.0, 1.0);

    //fragColor = colors[gl_VertexIndex];
    //fragColor = vec3(c*c, c*c, c*c);

    //float hue = fract(push.time * 0.1); // speed control
    float hue = fract(push.time * 0.1 + float(gl_VertexIndex) / 9.0);
    fragColor = hsv2rgb(vec3(hue, 1.0, 1.0));

}

