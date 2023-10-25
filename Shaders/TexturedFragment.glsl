#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D glassTex;
uniform float mixFactor;

in Vertex{
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 fragColour;

void main(void) {
	fragColour = mix(texture(diffuseTex, IN.texCoord), texture(glassTex, IN.texCoord),mixFactor);	
}