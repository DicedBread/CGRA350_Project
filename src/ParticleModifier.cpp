#pragma once

#include "ParticleModifier.hpp"

#include "ParticleEmitter.hpp"


ParticleModifier::ParticleModifier(ParticleEmitter& pEmit) : pe(pEmit)
{
}

ParticleModifier::~ParticleModifier(){}

void ParticleModifier::drawUi(){
    if(ImGui::CollapsingHeader("paticle emiter")){

        ImGui::SliderFloat("emitTime", &pe.emitTime, 0, 10);
        ImGui::SliderInt("emitCount", &pe.emitCount, 0, 100);

        ImGui::SliderFloat("lifeTime", &pe.lifeTime, 0, 30);
        ImGui::SliderFloat("speed", &pe.speed, 0, 10);

        ImGui::SliderFloat("billboard size", &pe.billboardSize, 0, 10);
    }

}