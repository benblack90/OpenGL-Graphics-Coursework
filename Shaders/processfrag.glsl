#version 330 core

uniform sampler2D sceneTex;
uniform int isVertical;


out vec4 fragColor;
const float scaleFactors[7] = float[](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);


void main(void) {
	fragColor = vec4(0,0,0,1);
	vec2 delta = vec2(0,0);

	vec2 texSize  = textureSize(sceneTex, 0).xy;  
	vec2 texCoord = gl_FragCoord.xy / texSize;

	if(isVertical == 1) {
		delta = dFdy(texCoord);
	}
	else {
		delta = dFdx(texCoord);
	}

	for(int i = 0;i < 7; i++)
	{
		vec2 offset = delta * (i - 3);
		vec4 tmp = texture2D(sceneTex, texCoord.xy + offset);
		fragColor += tmp * scaleFactors[i];
	}
}

