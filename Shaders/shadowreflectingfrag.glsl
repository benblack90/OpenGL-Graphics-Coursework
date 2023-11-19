#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex1;
uniform sampler2D shadowTex2;
uniform samplerCube cubeTex;
uniform vec4 spotlightColour;
uniform vec3 spotlightPos;
uniform vec3 spotlightDir;
uniform vec3 cameraPos;
uniform float spotlightRadius;
uniform float minDotProd;
uniform float dimProdMin;
uniform vec4 dirlightColour;
uniform vec3 dirlightDir;

in Vertex{
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProjSpot;
	vec4 shadowProjDir;
} IN;

out vec4 fragColour;


vec3 CalculateDiffuse(vec4 diffuseTex, float attenuation, vec3 incident, vec3 normal)
{
	float lambert = max(dot(incident, normal), 0.0f);
	vec3 diffuseLight = (diffuseTex.rgb * spotlightColour.rgb) * attenuation * lambert;

	return diffuseLight;
}

vec3 CalculateSpecular(vec3 incident, vec3 viewDir, vec3 normal, float attenuation)
{
	vec3 halfDir = normalize(incident + viewDir);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 5.0);	
	vec3 specularLight = spotlightColour.rgb * specFactor * attenuation * 0.33;;
	return specularLight;
}

float CalculateShadow(vec4 shadowProj, sampler2D tex)
{
	float shadow = 1.0;
	vec3 shadowNDC = shadowProj.xyz / shadowProj.w;
		if(abs(shadowNDC.x) < 1.0f &&
		abs(shadowNDC.y) < 1.0f &&
		abs(shadowNDC.z) < 1.0f) {
			vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
			float shadowZ = texture(tex, biasCoord.xy).x;
			if(shadowZ < biasCoord.z) {
				shadow = 0.0f;
			}
		}
		return shadow;
}

void main(void) {

	float attenuation = 1.0f;	
	float ringDiff = dimProdMin - minDotProd;
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	mat3 TBN = mat3(normalize(IN.tangent),normalize(IN.binormal), normalize(IN.normal));
	vec3 normal = texture(bumpTex, IN.texCoord).rgb;
	normal = normalize(TBN * normalize(normal * 2.0 - 1.0));
	vec4 diffuseTex = texture(diffuseTex, IN.texCoord);

	vec3 dirDiffuse = CalculateDiffuse(diffuseTex, attenuation, dirlightDir,normal);
	vec3 dirSpecular = CalculateSpecular(dirlightDir, viewDir, normal, attenuation);
	float dirShadow = CalculateShadow(IN.shadowProjDir, shadowTex2);
	vec3 spotDiffuse;
	vec3 spotSpecular;
	float spotShadow = 1.0;
	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
	vec4 reflectTex = texture(cubeTex, reflectDir);

	vec3 spotIncident = normalize(spotlightPos - IN.worldPos);	
	float dotProd = dot(-spotlightDir,spotIncident);
	if(dotProd > minDotProd) {
		float distance = length(spotlightPos - IN.worldPos);
		attenuation = 1.0f - clamp(distance / spotlightRadius, 0.0, 1.0);
		float intensity = clamp((dotProd - minDotProd) / ringDiff, 0.0, 1.0);
		spotDiffuse = CalculateDiffuse(diffuseTex, attenuation, spotIncident, normal) * intensity;		
		spotSpecular = CalculateSpecular(spotIncident, viewDir, normal, attenuation);
		spotShadow = CalculateShadow(IN.shadowProjSpot, shadowTex1);		
	}
	
	vec3 surface = diffuseTex.rgb * dirlightColour.rgb;
	fragColour.rgb = spotDiffuse + dirDiffuse;
	fragColour.rgb += spotSpecular + dirSpecular;
	fragColour.rgb *= spotShadow * dirShadow;
	fragColour.rgb += surface * 0.1f;
	fragColour += reflectTex + (diffuseTex * 0.25f); 
	fragColour.a = diffuseTex.a;
}