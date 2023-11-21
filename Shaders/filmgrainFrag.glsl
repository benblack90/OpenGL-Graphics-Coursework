#version 330 core

uniform float sceneTime;
uniform sampler2D sceneTex;
uniform float signalLoss;

out vec4 fragColor;

void main() {

  vec2 texSize  = textureSize(sceneTex, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 color = texture(sceneTex, texCoord);


 float noise = fract( 16000 * cos(( gl_FragCoord.x + gl_FragCoord.y * ((sin(sceneTime*40)))) * (3.141/180)));

    color.rgb += signalLoss * noise;
  

  fragColor = color;
}
