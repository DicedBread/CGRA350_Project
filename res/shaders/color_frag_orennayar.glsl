#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform float uRoughness;
uniform float uE_0;
uniform float uLightIntensity;
uniform sampler2D uTexture;
uniform bool uUseTexture;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

// Noticed a common pattern in all the C_n functions
float C_helper(float sigma, float x) {
    return sigma * sigma / (sigma * sigma + x);
}

void main() {
    // Following https://en.wikipedia.org/wiki/Oren%E2%80%93Nayar_reflectance_model
    // Constants
    const vec3 light_pos = vec3(0, 50, 100); // (world-space)
    const float PI = 3.14159265359;

    vec3 texture_color = uColor;

    if (uUseTexture) {
        texture_color = texture(uTexture, f_in.textureCoord).rgb;
    }

    // Useful vectors for later
    vec3 L = normalize((vec4(light_pos, 1)).xyz - f_in.position);
    vec3 V = normalize(-f_in.position);
    vec3 N = normalize(f_in.normal);

    // Angles
    float theta_i = acos(dot(L, N));
    float theta_r = acos(dot(V, N));

    // Azimuthal angles
    vec3 T1 = normalize(cross(N, V));
    vec3 T2 = cross(N, T1);
    vec3 L_proj = normalize(L - dot(L, N) * N);
    vec3 V_proj = normalize(V - dot(V, N) * N);
    float phi_i = atan(dot(L_proj, T2), dot(L_proj, T1));
    float phi_r = atan(dot(V_proj, T2), dot(V_proj, T1));

    float alpha = max(theta_i, theta_r);
    float beta = min(theta_i, theta_r);

    float C_1 = 1 - 0.5 * C_helper(uRoughness, 0.33);
    float C_2 = 0.45 * C_helper(uRoughness, 0.09);
    if (cos(phi_i - phi_r) >= 0) {
        C_2 *= sin(alpha);
    } else {
        C_2 *= sin(alpha) - pow((2 * beta) / PI, 3);
    }
    float C_3 = 0.125 * C_helper(uRoughness, 0.09) * pow(4 * alpha * beta / (PI * PI), 2);

    vec3 L_1 = texture_color / PI * uE_0 * cos(theta_i) * (C_1 + C_2 * cos(phi_i - phi_r) * tan(beta) + C_3 * (1 - abs(cos(phi_i - phi_r)) * tan((alpha + beta) / 2)));

    vec3 L_2 = 0.17 * texture_color * texture_color / PI * uE_0 * cos(theta_i) * C_helper(uRoughness, 0.13) * (1 - cos(phi_i - phi_r) * pow(2 * beta / PI, 2));

    // Fixed weird artifacts
    if (dot(N, L) <= 0) {
        L_1 = vec3(0);
    }

    vec3 L_r = L_1 + L_2;

    fb_color = vec4(L_r, 1.0);
}
