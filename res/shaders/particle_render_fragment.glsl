#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform sampler2D uText;

uniform float totalLifeTime;
uniform vec3 initColor;
uniform vec3 endColor;

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
	if(f_in.type == 1) discard;
	float agePer = f_in.age / totalLifeTime; 

	vec3 col = mix(initColor, endColor, agePer);  
	float alpha = texture(uText, f_in.textCord).a;

	alpha = mix(alpha, 0, agePer);

	vec4 color = vec4(col, alpha);
	if(color == vec4(0,0,0,0)){
		discard;
	}
	// output to the frambuffer
	fb_color = color;
}