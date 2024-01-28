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


 float noise = fract( 16000 * cos(( IN.texCoord.x * 1920 + IN.texCoord.y * 1080 * ((sin(sceneTime*40))))));

    colour.rgb += signalLoss * noise;
  

  fragColour = colour;
}
