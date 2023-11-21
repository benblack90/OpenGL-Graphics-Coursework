#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex1;
uniform vec4 spotlightColour;
uniform vec3 spotlightPos;
uniform vec3 spotlightDir;
uniform vec3 cameraPos;
uniform float spotlightRadius;
uniform float minDotProd;
uniform float dimProdMin;
uniform vec4 dirlightColour;
uniform vec3 dirlightDir;
uniform float dirHorizonCheck;

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


vec3 CalculateDiffuse(float attenuation, vec3 incident, vec3 normal, vec4 lightColour)
{
	float lambert = max(dot(incident, normal), 0.0f);
	vec3 diffuseLight = lightColour.rgb * attenuation * lambert;

	return diffuseLight;
}

vec3 CalculateSpecular(vec3 incident, vec3 viewDir, vec3 normal, float attenuation, vec4 lightColour)
{
	vec3 halfDir = normalize(incident + viewDir);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 50.0);	
	vec3 specularLight = lightColour.rgb * specFactor * attenuation * 0.33;
	return specularLight;
}

float CalculateShadow(vec4 shadowProj, sampler2D tex, float shadowIntensity)
{
	float shadow = 1.0;
	vec3 shadowNDC = shadowProj.xyz / shadowProj.w;
		if(abs(shadowNDC.x) < 1.0f &&
		abs(shadowNDC.y) < 1.0f &&
		abs(shadowNDC.z) < 1.0f) {
			vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
			float shadowZ = texture(tex, biasCoord.xy).x;
			if(shadowZ < biasCoord.z) {
				shadow = shadowIntensity;
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

	vec3 dirDiffuse = CalculateDiffuse(attenuation, dirlightDir,normal, dirlightColour) * dirHorizonCheck;
	vec3 dirSpecular = CalculateSpecular(dirlightDir, viewDir, normal, attenuation, dirlightColour) * dirHorizonCheck;
	
	vec3 spotDiffuse;
	vec3 spotSpecular;
	vec3 spotIncident = normalize(spotlightPos - IN.worldPos);	
	float spotDotProd = dot(-spotlightDir,spotIncident);
	vec3 ambient = 0.3 * diffuseTex.rgb * dirlightColour.rgb;
	vec3 diffuse = dirDiffuse * dirlightColour.rgb;
	vec3 specular = dirSpecular;
	float shadow = 1.0;

	if(spotDotProd > minDotProd) {
		float distance = length(spotlightPos - IN.worldPos);
		attenuation = 1.0f - clamp(distance / spotlightRadius, 0.0, 1.0);
		float intensity = clamp((spotDotProd - minDotProd) / ringDiff, 0.0, 1.0);
		spotDiffuse = CalculateDiffuse(attenuation, spotIncident, normal, spotlightColour) * intensity;		
		spotSpecular = CalculateSpecular(spotIncident, viewDir, normal, attenuation, spotlightColour);
		float spotShadow = CalculateShadow(IN.shadowProjSpot, shadowTex1, 0.25);
		ambient *= spotlightColour.rgb;
		diffuse += spotDiffuse;
		specular += spotSpecular;
		//factor in attenuation to deal with additive shadows: without this,spotlight range is basically infinite!
		shadow += spotShadow * attenuation;
	}
	
	vec3 lighting = (ambient + shadow * (diffuse + specular)) * diffuseTex.rgb;
	fragColour = vec4(lighting,1);
	
	fragColour.a = diffuseTex.a;
}
