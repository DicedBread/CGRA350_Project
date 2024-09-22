#version 330


layout (location = 0) in float type;
layout (location = 1) in vec3 pos;


out float type0;
out vec3 pos0;


void main(){
    type0 = type;
    pos0 = pos;

}