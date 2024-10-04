#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform sampler2D uText;

// viewspace data (this must match the output of the geo shader)
in VertexData{
    float type;
    vec4 position;
    vec3 velocity;
    float age;
    vec2 textCord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting (hack)
	// vec3 eye = normalize(-f_in.position);
	// float light = 1;

	vec4 color = mix(texture(uText, f_in.textCord).rgba, vec4(f_in.age, 0, 0, 0), 0.5);
	if(color == vec4(0,0,0,0)){
		discard;
	}
	// output to the frambuffer
	fb_color = color;
}