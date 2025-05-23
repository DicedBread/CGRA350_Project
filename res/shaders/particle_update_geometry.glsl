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
uniform float spawnRadius = 1;

uniform bool shouldUpdatePosition = false;
uniform vec3 updatePos = vec3(0,0,0);

uniform vec3 initVelocity = vec3(0, 1, 0);
uniform float initSpeed = 3;
uniform float lifeTime = 20;
uniform float maxSpeed = 3;
uniform float speedDropPercent = 0.5;

uniform bool isOneOff = false;
uniform bool shouldEmitOneOff = false;

uniform float maxAccel = 100;
uniform vec3 velVariance = vec3(0,0,0);
uniform vec3 constForceDir = vec3(0,0,0);
uniform float constForceStrength = 0;

uniform float dragStrength = 50;

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
    vec3 newEmitterVelocity = velocity0[0];
    bool emitterHasNonZeroVelocity = length(emitterVelocity) > 0;
    if(emitterHasNonZeroVelocity){
        emitterPosition = position0[0] + (normalize(emitterVelocity) * emitterSpeed * delta);
        newEmitterVelocity = normalize(emitterVelocity);
    }
    if(shouldUpdatePosition){
        emitterPosition = updatePos + (normalize(emitterVelocity) * emitterSpeed * delta);;
    }

    bool needToEmitOneOff = isOneOff && shouldEmitOneOff;
    bool isTimeToEmitForPassiveEmit = age >= emitTime && !isOneOff;
    bool isTimeToEmit = isTimeToEmitForPassiveEmit || needToEmitOneOff; 
    if(isTimeToEmit){ 
        emit(emitterrType, emitterPosition, emitterVelocity, 0);
        for(int i = 0; i < emitCount; i++){
            vec3 newPartVel = initVelocity;
            newPartVel = vec3(newPartVel.x + randRange(-velVariance.x, velVariance.x),
                            newPartVel.y + randRange(-velVariance.y, velVariance.y),
                            newPartVel.z + randRange(-velVariance.z, velVariance.z));
            newPartVel = normalize(newPartVel) * initSpeed;
            vec3 spawnPos = position0[0] + vec3(randRange(-1, 1), randRange(-1, 1), randRange(-1, 1)) * spawnRadius;
            emit(PARTICLE_TYPE, spawnPos, newPartVel, 0);
        }
    }else{
        emit(emitterrType, emitterPosition, emitterVelocity, age);
    }
}

// handles particle type primative
void handleParticle(){
    float age = age0[0] + (delta * rand());
	float agePer = age / lifeTime; 

    // float lSpeed = mix(speed, speed * speedDropPercent, agePer);

    if(age < lifeTime){
        vec3 acceleration = (-velocity0[0] * dragStrength) + (constForceDir * constForceStrength);
        // acceleration = min(length(acceleration), maxAccel) * normalize(acceleration);

        // vec3 acceleration = vec3(0,1,0);
        vec3 vel = velocity0[0] + (acceleration * delta);
        // vec3 vel = velocity0[0];
        float sp = length(vel);

        // sp = clamp(sp, 0, maxSpeed);
        vel = normalize(vel) * sp; 
        vec3 newPosition = position0[0] + (vel * delta);
        emit(PARTICLE_TYPE, newPosition, vel, age);
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

