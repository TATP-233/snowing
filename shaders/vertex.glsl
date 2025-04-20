#version 120

attribute vec3 aPos;
attribute vec2 aTexCoord;

varying vec2 TexCoord;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
} 