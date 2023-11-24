#version 330 core

uniform sampler2D diffuseTex;


in Vertex{

	vec2 texCoord;

} IN;

out vec4 fragColour;

void main(void) {

	fragColour = texture(diffuseTex, IN.texCoord);	
	fragColour.rgb *= vec3(10,5,5);
}