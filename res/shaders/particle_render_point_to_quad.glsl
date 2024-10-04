#version 330 core
#extension GL_ARB_geometry_shader4 : enable


uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform vec3 uCameraPos;


layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in VertexData{
    float type;
    vec3 position;
    vec3 velocity;
    float age;
    vec2 textCord;
} g_in[];

out VertexData{
    float type;
    vec4 position;
    vec3 velocity;
    float age;
    vec2 textCord;
} g_out;

void main(){
    vec3 pos = gl_in[0].gl_Position.xyz;
    vec3 camPos = uCameraPos;
    vec3 camToPoint = normalize(pos - camPos); 
    vec3 camUp = vec3(uModelViewMatrix[0][1], uModelViewMatrix[1][1], uModelViewMatrix[2][1]);
    vec3 camRight = vec3(uModelViewMatrix[0][0], uModelViewMatrix[1][0], uModelViewMatrix[2][0]);

    float billboardScale = 1;

    g_out.type = g_in[0].type;
    g_out.velocity = g_in[0].velocity;
    g_out.age = g_in[0].age;

    // top left
    vec3 topLeftPos = pos + camRight * -0.5 * billboardScale + camUp * 0.5 * billboardScale;
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(topLeftPos, 1));
    g_out.textCord = vec2(0,1);
    gl_Position = g_out.position;
    EmitVertex();

    // bottom left
    vec3 bottomLeftPos = pos + camRight * -0.5 * billboardScale + camUp * -0.5 * billboardScale;
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(bottomLeftPos, 1));
    g_out.textCord = vec2(0,0);
    gl_Position = g_out.position;
    EmitVertex();

    // top right
    vec3 topRightPos = pos + camRight * 0.5 * billboardScale + camUp * 0.5 * billboardScale;
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(topRightPos, 1));
    g_out.textCord = vec2(1,1);
    gl_Position = g_out.position;
    EmitVertex();

    // bottom right
    vec3 bottomRightPos = pos + camRight * 0.5 * billboardScale + camUp * -0.5 * billboardScale;
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(bottomRightPos, 1));
    g_out.textCord = vec2(1,0);
    gl_Position = g_out.position;
    EmitVertex();
    EndPrimitive();
}