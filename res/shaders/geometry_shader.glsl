#version 330 core
#extension GL_ARB_geometry_shader4 : enable

layout(points) in;
layout(points) out;
layout(max_vertices = 20) out;

in float type0[];
in vec3 position0[];
in vec3 velocity0[];
in float age0[];

out float type1;
out vec3 position1;
out vec3 velocity1;
out float age1;

uniform float delta;
uniform float lifeTime = 1;

void main(){

    float age = age0[0] + delta;
    // if(type0[0] == 1){
        // if(age >= lifeTime){
            type1 = 2;
            position1 = position0[0];
            velocity1 = vec3(0, 1, age0[0]);
            age1 = 0.0;
            EmitVertex();
            EndPrimitive();
            age = 0;
        // }
    // }

    type1 = 1;
    position1 = position0[0] + velocity0[0];
    velocity1 = velocity0[0];
    age1 = age;
    EmitVertex();
    EndPrimitive();

} 