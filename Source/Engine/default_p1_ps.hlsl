//Includes
#include "default_shared.hlsli"

//Shadowmap Function
float ShadowCalculation(float4 _vPosLight)
{
	float bias = 0.0001f;
	float pcfAmount = 2.0f;
	bool bDoPCF = true;

	//perform perspective divide
	float3 projCoords = _vPosLight.xyz / _vPosLight.w;

	//transform to [0,1] range
	projCoords.xy = (projCoords.xy * 0.5f) + 0.5f;

	//get depth of current pxiel from light's persp
	float currentDepth = projCoords.z;

	//Shadow value output
	float shadow = 0.0f;

	//PCF
	if(bDoPCF)
	{
		float pcfPasses = 0.0f;
		float2 shadowMapSize = float2(0.0f, 0.0f);
		g_txShadowMap.GetDimensions(shadowMapSize.x, shadowMapSize.y);
		float2 texelSize = 1.0f / shadowMapSize;
		for (int x = -pcfAmount; x <= pcfAmount; ++x)
		{
			for (int y = -pcfAmount; y <= pcfAmount; ++y)
			{
				float pcfDepth = g_txShadowMap.Sample(g_sShadowSampler, float2(projCoords.x, 1.0f - projCoords.y) + (float2(x, y) * texelSize)).r;
				shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
				++pcfPasses;
			}
		}

		shadow /= pcfPasses;
	}
	else
	{
		//get closest depth value from light's perspective
		float closestDepth = g_txShadowMap.Sample(g_sShadowSampler, float2(projCoords.x, 1.0f - projCoords.y)).r;

		//check whether current pixel pos is in shadow
		shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	}

	//Depth out of range of shadowmap should not have a shadow
	if (projCoords.z > 1.0f) shadow = 0.0f;

	return(shadow);
}

//Lighting functions
float3 ParallelLight(SurfaceInfo v, LightDesc L)
{
	float3 litColor = float3(0.0f, 0.0f, 0.0f);
	float shadow = ShadowCalculation(v.posLightSpace);

	//The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.dir;

	//Add the ambient term.
	//litColor += v.diffuse.rgb * (L.ambient.rgb * v.aoValue);

	float3 ambient = v.diffuse.rgb * (L.ambient.rgb * v.aoValue);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	//Add diffuse and specular term, provided the surface is in 
	//the line of site of the light.

	float diffuseFactor = dot(lightVec, v.normal) * v.aoValue;
	if (diffuseFactor > 0.0f)
	{
		float specPower = max(v.spec, 1.0f);
		float3 toEye = normalize(g_tCamera.vec3EyePos - v.pos);
		float3 R = reflect(-lightVec, v.normal);
		float specFactor = pow(max(dot(R, toEye), 0.0f), specPower);

		// diffuse and specular terms
		diffuse = diffuseFactor * v.diffuse.rgb * L.diffuse.rgb;
		specular = specFactor * v.spec * L.spec.xyz;
	}

	litColor = (ambient + ((diffuse + specular) * (1.0f - shadow)));

	return(litColor);
}

//Default pass (pass1)
float4 main(VS_OUT Input) : SV_TARGET
{
	//Interpolating normal can make it not be of unit length so normalize it.
	float3 normalW = normalize(Input.vNormalW.xyz);

	float4 vDiffuse =	g_txDiffuse.Sample(g_sAnisoSampler,	Input.vTexCoord);
	float4 vNormal =	g_txNormal.Sample(g_sAnisoSampler,	Input.vTexCoord);
	float4 vSpec =		g_txSpec.Sample(g_sAnisoSampler,	Input.vTexCoord);
	float4 vAO =		g_txAO.Sample(g_sAnisoSampler,		Input.vTexCoord);

	//TODO: Tangent Normal Support (replace normalW with vNormal)
	SurfaceInfo v = {Input.vPosW.xyz, Input.vPosLight, normalW, vDiffuse, vSpec.r, vAO.r};

	float4 pixelColor = float4(ParallelLight(v, g_tLight), vDiffuse.a);
	pixelColor = saturate(pixelColor);

	return(pixelColor);
}