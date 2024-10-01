#pragma once


#include "ParticleEmitter.hpp"

#include <glm/glm.hpp>


class ParticleModifier
{
private:
    ParticleEmitter& pe;
public:
    
    ParticleModifier(ParticleEmitter& pEmit);
    ~ParticleModifier();
    void drawUi();
};

