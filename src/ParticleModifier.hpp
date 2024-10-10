#pragma once


#include "ParticleEmitter.hpp"

#include <glm/glm.hpp>


class ParticleModifier
{
private:
    ParticleEmitter& pe;
    float randF();
public:
    
    ParticleModifier(ParticleEmitter& pEmit);
    ~ParticleModifier();
    void example1();
    void randomize();
    void drawUi();
};

