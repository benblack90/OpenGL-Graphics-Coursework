#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D brightTex;


in Vertex{

	vec2 texCoord;

} IN;

out vec4 fragColour;

void main(void) {

	fragColour = texture(diffuseTex, IN.texCoord);	
	fragColour += texture(brightTex, IN.texCoord);

}