#version 330 core
#extension GL_ARB_geometry_shader4 : enable

layout(points) in;
layout(points) out;
layout(max_vertices = 100) out;

in float type0[];
in vec3 position0[];
in vec3 velocity0[];
in float age0[];

out float type1;
out vec3 position1;
out vec3 velocity1;
out float age1;


uniform float randIteratorIn;

uniform float delta;
uniform vec3 emitterVelocity = vec3(0,0,0);
uniform float emitterSpeed = 0;
uniform float emitTime = 0.01;
uniform int emitCount = 10;

uniform float lifeTime = 20;
uniform float speed = 3;

float offset = 1;

float randNoise(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand(){
    offset += delta * 100 + randIteratorIn * 1000;
    return randNoise(vec2(offset, offset / 2));
}

float randRange(float min, float max){
    return mix(min, max, rand());
}

void main(){
    float age = age0[0] + delta;
    if(type0[0] == 1){
        type1 = type0[0];
        position1 = position0[0];
        velocity1 = velocity0[0];
        if(length(emitterVelocity) > 0){
            position1 = position0[0] + (velocity0[0] * emitterSpeed * delta);
            velocity1 = normalize(emitterVelocity);
        }
        if(age >= emitTime){
            age1 = 0;
            EmitVertex();
            EndPrimitive(); // emit emitter 
            for(int i = 0; i < emitCount; i++){
                type1 = 2;
                position1 = position0[0];
                velocity1 = vec3(randRange(-1, 1), 1, randRange(-1, 1));
                age1 = 0;
                EmitVertex(); // new particle
                EndPrimitive();
            }
        }else{
            age1 = age;
            EmitVertex(); 
            EndPrimitive();
        }
    }

    if(type0[0] == 2){
        if(age < lifeTime){
            type1 = 2;
            position1 = position0[0] + (velocity0[0] * speed * delta);
            velocity1 = velocity0[0];
            age1 = age;
            EmitVertex();
            EndPrimitive();
        }
    }
} 

