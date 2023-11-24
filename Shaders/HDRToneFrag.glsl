#version 330 core
out vec4 fragColour;

uniform sampler2D hdrTex;

void main()
{             

    vec2 texSize  = textureSize(hdrTex, 0).xy;  
    vec2 texCoord = gl_FragCoord.xy / texSize;
    vec3 hdrColor = texture(hdrTex, texCoord).rgb;

      // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
  
    fragColour = vec4(mapped, 1.0);
}