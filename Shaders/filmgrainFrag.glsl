#version 330 core

uniform float sceneTime;
uniform sampler2D sceneTex;
uniform float signalLoss;

out vec4 fragColour;

in Vertex {
	vec2 texCoord;
} IN;

void main() {

 

  vec4 colour = texture(sceneTex, IN.texCoord);


 float noise = fract( 16000 * cos(( gl_FragCoord.x + gl_FragCoord.y * ((sin(sceneTime*40)))) * (3.141/180)));

    colour.rgb += signalLoss * noise;
  

  fragColour = colour;
}
