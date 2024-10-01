#version 330 core
#extension GL_ARB_geometry_shader4 : enable


uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform vec3 uCameraPos;


layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

out VertexData{
    vec4 position;
    vec2 textCord;
} g_out;

void main(){
    vec3 pos = gl_in[0].gl_Position.xyz;
    vec3 camPos = uCameraPos;
    vec3 camToPoint = normalize(pos - camPos); 
    vec3 up = vec3(0,1,0);
    vec3 right = normalize(cross(up, camToPoint));

    // top left
    vec3 topLeftPos = pos + vec3(0, 0.5, 0) + (-right * 0.5);
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(topLeftPos, 1));
    g_out.textCord = vec2(0,1);
    gl_Position = g_out.position;
    EmitVertex();

    // bottom left
    vec3 bottomLeftPos = pos + vec3(0, -0.5, 0) + (-right * 0.5);
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(bottomLeftPos, 1));
    g_out.textCord = vec2(0,0);
    gl_Position = g_out.position;
    EmitVertex();

    // top right
    vec3 topRightPos = pos + vec3(0, 0.5, 0) + (right * 0.5);
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(topRightPos, 1));
    g_out.textCord = vec2(1,1);
    gl_Position = g_out.position;
    EmitVertex();

    // bottom right
    vec3 bottomRightPos = pos + vec3(0, -0.5, 0) + (right * 0.5);
    g_out.position = uProjectionMatrix * (uModelViewMatrix * vec4(bottomRightPos, 1));
    g_out.textCord = vec2(1,0);
    gl_Position = g_out.position;
    EmitVertex();
    EndPrimitive();
}