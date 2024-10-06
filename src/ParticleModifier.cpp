#pragma once

#include "ParticleModifier.hpp"

#include "ParticleEmitter.hpp"


ParticleModifier::ParticleModifier(ParticleEmitter& pEmit) : pe(pEmit)
{
}

ParticleModifier::~ParticleModifier(){}

void ParticleModifier::drawUi(){
    if(ImGui::CollapsingHeader("particle emitter")){

        ImGui::SliderFloat3("emitter velocity", value_ptr(pe.emitterVelocity), -1, 1);
        ImGui::SliderFloat("emitter speed", &pe.emitterSpeed, 0, 10);
        ImGui::SliderFloat("emitTime", &pe.emitTime, 0, 10);
        ImGui::SliderInt("emitCount", &pe.emitCount, 0, 100);
        ImGui::Separator();

        ImGui::SliderFloat("lifeTime", &pe.lifeTime, 0, 30);
        ImGui::SliderFloat3("inital velocity", value_ptr(pe.initVelocity), -1, 1);

        ImGui::SliderFloat("speed", &pe.speed, 0, 10);
        ImGui::SliderFloat("spawn radius", &pe.spawnRadius, 0, 10);
        ImGui::SliderFloat("billboard size", &pe.billboardSize, 0, 10);

        ImGui::SliderFloat3("inital color", value_ptr(pe.initColor), 0, 1);
        ImGui::SliderFloat3("end color", value_ptr(pe.endColor), 0, 1);

    }

}