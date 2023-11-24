#version 330 core

uniform sampler2D diffuseTex;


out vec4 fragColour;

void main(void) {

    vec2 texSize  = textureSize(diffuseTex, 0).xy;  
    vec2 texCoord = gl_FragCoord.xy / texSize;
    vec3 colour = texture(diffuseTex, texCoord).rgb;
	float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        fragColour = vec4(colour.rgb, 1.0);
    else
        discard;


}