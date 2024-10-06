#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

// veg-cov
uniform float uCovDensity;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 oColor = uColor;
	if(fract(f_in.position.x * f_in.position.y * f_in.position.z) < uCovDensity) {
		oColor = vec3(0, 0.8, 0);
	}
	vec3 color = mix(oColor / 4, oColor, light);

	// output to the frambuffer
	fb_color = vec4(color, 1);
}