#version 330 core

uniform sampler2D diffuseTex;
in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {

    vec3 colour = texture(diffuseTex, IN.texCoord).rgb;
	float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.65)
        fragColour = vec4(colour.rgb, 1.0);
    else
        fragColour = vec4(0,0,0,1);


}