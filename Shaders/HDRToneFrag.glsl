#version 330 core
out vec4 fragColour;

uniform sampler2D hdrTex;

void main()
{             

    vec2 texSize  = textureSize(hdrTex, 0).xy;  
    vec2 texCoord = gl_FragCoord.xy / texSize;
    vec3 hdrTex = texture(hdrTex, texCoord).rgb;

    vec3 rhard = hdrTex / (hdrTex + vec3(1.0));
  
    fragColour = vec4(rhard, 1.0);
}