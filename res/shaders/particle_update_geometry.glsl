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

const float EMITTER_TYPE = 1;
const float PARTICLE_TYPE = 2;

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

// returns random float between 0 and 1
float rand(){
    offset += delta * 100 + randIteratorIn * 1000;
    return randNoise(vec2(offset, offset / 2));
}

// returns random value between min and max range
float randRange(float min, float max){
    return mix(min, max, rand());
}

// emits vertex with given parameters 
void emit(float type, vec3 position, vec3 velocity, float age){
    type1 = type;
    position1 = position;
    velocity1 = velocity;
    age1 = age;
    EmitVertex();
    EndPrimitive(); 
}

// handles emitter type primative 
void handleEmitter(){
    float age = age0[0] + delta;
    float emitterrType = type0[0];
    vec3 emitterPosition = position0[0];
    vec3 emitterVelocity = velocity0[0];
    bool emitterHasNonZeroVelocity = length(emitterVelocity) > 0;
    if(emitterHasNonZeroVelocity){
        emitterPosition = position0[0] + (velocity0[0] * emitterSpeed * delta);
        emitterVelocity = normalize(emitterVelocity);
    }
    bool isTimeToEmit = age >= emitTime; 
    if(isTimeToEmit){ 
        emit(emitterrType, emitterPosition, emitterVelocity, 0);
        for(int i = 0; i < emitCount; i++){
            vec3 newPartVel = vec3(randRange(-1, 1), 1, randRange(-1, 1));
            emit(PARTICLE_TYPE, position0[0], newPartVel, 0);
        }
    }else{
        emit(emitterrType, emitterPosition, emitterVelocity, age);
    }
}

// handles particle type primative
void handleParticle(){
    float age = age0[0] + delta;
    if(age < lifeTime){
        vec3 newPosition = position0[0] + (velocity0[0] * speed * delta);
        emit(PARTICLE_TYPE, newPosition, velocity0[0], age);
    }
}

void main(){
    switch (int(type0[0])){
        case int(EMITTER_TYPE):
            handleEmitter();
            break;
        case int(PARTICLE_TYPE):
            handleParticle();
            break;
    }
} 

