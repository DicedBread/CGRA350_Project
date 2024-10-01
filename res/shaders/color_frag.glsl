#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform sampler2D uText;

// viewspace data (this must match the output of the geo shader)
in VertexData {
	vec3 position;
	vec2 textCord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = 1;
	vec3 color = texture(uText, f_in.textCord).rgb;

	// output to the frambuffer
	fb_color = vec4(color, 1);
}