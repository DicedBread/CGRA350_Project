#version 330 core

// uniform data
uniform mat4  uProjectionMatrix;
uniform mat4  uModelViewMatrix;
uniform vec3  uColor;

// for deformation
uniform int   uIsDeformation;
uniform float uDeformation;

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// model data (this must match the input of the vertex shader)
out VertexData {
    vec3 orgPosition;
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} v_out;

void main() {
    // deformation
	float amplitude = uDeformation;
	vec3 aDeformPosition = aPosition;
	if(uIsDeformation == 1) {
		float h = amplitude * fract(aPosition.x * 12306.2322);
		aDeformPosition.y += h;
	}
	// transform vertex data to viewspace
	v_out.orgPosition = aPosition;
	v_out.position = (uModelViewMatrix * vec4(aDeformPosition, 1)).xyz;
	v_out.normal = normalize((uModelViewMatrix * vec4(aNormal, 0)).xyz);
	v_out.textureCoord = aTexCoord;

	// set the screenspace position (needed for converting to fragment data)
	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1);
}