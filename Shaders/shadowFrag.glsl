#version 330 core

out vec4 fragColour;

void main(void) {
	gl_FragDepth = gl_FragCoord.z;
}