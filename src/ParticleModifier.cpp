#pragma once

#include "ParticleModifier.hpp"

#include "ParticleEmitter.hpp"


ParticleModifier::ParticleModifier(ParticleEmitter& pEmit) : pe(pEmit)
{
}

ParticleModifier::~ParticleModifier(){}

void ParticleModifier::drawUi(){
    std::stringstream ss;
    ss << "particle emitter " << &pe;
    if(ImGui::CollapsingHeader(ss.str().c_str())){

        ImGui::SliderFloat3("emitter velocity", value_ptr(pe.emitterVelocity), -1, 1);
        ImGui::SliderFloat("emitter speed", &pe.emitterSpeed, 0, 10);
        ImGui::Separator();

        ImGui::SliderFloat("emitTime", &pe.emitTime, 0, 10);
        ImGui::SliderInt("emitCount", &pe.emitCount, 1, 100);

        ImGui::Checkbox("one off", &pe.isOneOff);
        if(pe.isOneOff){
            if(ImGui::Button("emit")){
                pe.emitOneOff();
            }
        }

        ImGui::Separator();

        ImGui::SliderFloat("lifeTime", &pe.lifeTime, 0, 30);
        ImGui::SliderFloat3("inital velocity", value_ptr(pe.initVelocity), -1, 1);
        ImGui::SliderFloat("init speed", &pe.initSpeed, 0, 10);
        ImGui::SliderFloat3("velocity variance", value_ptr(pe.velVariance), 0, 1);
        ImGui::SliderFloat3("constant force dir", value_ptr(pe.constForceDir), -1, 1);
        ImGui::SliderFloat("constant force strength", &pe.constForceStrength, 0, 1);
        ImGui::SliderFloat("drag force strength", &pe.dragStrength, 0, 1);


        ImGui::SliderFloat("max speed", &pe.maxSpeed, 0, 1000);
        // ImGui::SliderFloat("speed drop to percent", &pe.speedDropPercent, 0, 10);

        ImGui::SliderFloat("spawn radius", &pe.spawnRadius, 0, 10);
        ImGui::SliderFloat("init billboard size", &pe.initBillboardSize, 0, 10);
        ImGui::SliderFloat("end billboard size", &pe.endBillboardSize, 0, 10);


        ImGui::SliderFloat3("inital color", value_ptr(pe.initColor), 0, 1);
        ImGui::SliderFloat3("end color", value_ptr(pe.endColor), 0, 1);


        if(ImGui::Button("example 1")){
            example1();
        }
    }
}


void ParticleModifier::example1(){
    pe.emitTime = 0.1;
    pe.emitCount = 9;
    
    pe.velVariance = glm::vec3(1, 0, 1);
    pe.constForceDir = glm::vec3(0, -1, 0);
    pe.constForceStrength = 1;
    pe.initSpeed = 6;   
    pe.initColor = glm::vec3(0, 1, 0.5);
    pe.endColor = glm::vec3(1, 0, 0);

}