#version 120

varying vec2 TexCoord;

uniform sampler2D texture1;
uniform float alpha;

void main()
{
    vec4 texColor = texture2D(texture1, TexCoord);
    gl_FragColor = vec4(texColor.rgb, texColor.a * alpha);
} 