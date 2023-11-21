#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
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
uniform float dirHorizonCheck;

in Vertex{
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
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
	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
	vec4 reflectTex = textureLod(cubeTex, reflectDir,1.5);

	vec3 spotIncident = normalize(spotlightPos - IN.worldPos);	
	float spotDotProd = dot(-spotlightDir,spotIncident);
	vec3 ambient = 0.3 * diffuseTex.rgb * dirlightColour.rgb;
	vec3 diffuse = dirDiffuse * dirlightColour.rgb;
	vec3 specular = dirSpecular;

	if(spotDotProd > minDotProd) {
		float distance = length(spotlightPos - IN.worldPos);
		attenuation = 1.0f - clamp(distance / spotlightRadius, 0.0, 1.0);
		float intensity = clamp((spotDotProd - minDotProd) / ringDiff, 0.0, 1.0);
		spotDiffuse = CalculateDiffuse(attenuation, spotIncident, normal, spotlightColour) * intensity;		
		spotSpecular = CalculateSpecular(spotIncident, viewDir, normal, attenuation, spotlightColour);	
		ambient *= spotlightColour.rgb;
		diffuse += spotDiffuse;
		specular += spotSpecular;
	}


	vec3 lighting = (ambient + (diffuse + specular)) * diffuseTex.rgb;
	fragColour = vec4(lighting,1);
	fragColour += reflectTex + (diffuseTex * 0.25);
	
	fragColour.a = diffuseTex.a;
}