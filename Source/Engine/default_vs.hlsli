//Shared data include, used by both VS & PS stages
#include "default_shared.hlsli"

//Shared function to save duplicate code
VS_OUT calculateVertex(VS_IN _input)
{
	VS_OUT output;

	//Build WVP
	float4x4 matWVP = mul(matWorld, g_matViewProj);

	//Transform to world space
	output.vPosW =		mul(float4(_input.vPosL, 1.0f), matWorld);
	output.vNormalW =	mul(float4(_input.vNormalL, 0.0f), matWorld);
	//output.vTangentW =	mul(float4(_input.vTangentL, 0.0f), matWorld).xyz;

	//Transform to homogeneous clip space
	output.vPosH =		mul(float4(_input.vPosL, 1.0f), matWVP);

	//Transform to lightspace
	output.vPosLight =	mul(output.vPosW, matSunVP);

	//Tex coords
	output.vTexCoord = _input.vTexCoord;

	//TBN
	//float3x3 TBN = CalcTBN(_input);

	//Tangents
	//output.vTangentW = mul(output.vPosW.xyz, TBN);
	//output.vTangentLight = mul(g_tLight.pos, TBN);
	//output.vTangentView = mul(g_tCamera.vec3EyePos, TBN);

	return(output);
}




//--------------------------------------------------------------------------------
// Vertex shader entry points
//--------------------------------------------------------------------------------

//Shadowmap pass (pass0)
float4 main_vsp0(VS_IN Input): SV_POSITION
{
	float4x4 matWVP = mul(matWorld, matSunVP);
	return(mul(float4(Input.vPosL, 1.0f), matWVP));
}

//Shadowmap pass for instanced meshes (pass0)
float4 main_vsinstp0(VS_IN_INSTANCED Input): SV_POSITION
{
	//Scale, rotate, position the vertex
	float3 vec3Pos = Input.vPosL * Input.vInstanceScale;
	vec3Pos = RotateVectorByQuaternion(Input.vInstanceRot, vec3Pos);
	vec3Pos += Input.vInstancePos;

	//Clip to the Sun's view proj
	return(mul(float4(vec3Pos, 1.0f), matSunVP));
}

//Default pass (pass1)
VS_OUT main_vsp1(VS_IN _input)
{
	return(calculateVertex(_input));
}

//Default pass for instanced meshes (pass1)
VS_OUT main_vsinstp1(VS_IN_INSTANCED _input)
{
	//Copy the input vertex data
	//We are cheating here as they're identical for the required bytes, zero risk (supposedly)
	VS_IN vertex = (VS_IN)_input;

	//Scale, rotate, position the vertex
	vertex.vPosL *= _input.vInstanceScale;
	vertex.vPosL = RotateVectorByQuaternion(_input.vInstanceRot, vertex.vPosL);
	vertex.vPosL += _input.vInstancePos;

	//Rotate vertex normal
	vertex.vNormalL = RotateVectorByQuaternion(_input.vInstanceRot, vertex.vNormalL);

	//Even though this uses matWorld, it is forced to identity so it doesn't affect the position data
	return(calculateVertex(vertex));
}

VS_OUT main_vsriggedp1(RIGGED_VS_IN _input)
{
	//Calculate bone manipulation/weights etc. here
	VS_IN vertex = (VS_IN)_input;

	float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < 4; ++i)
	{
		if(_input.vBoneIds[i] == -1) continue;
		if(_input.vBoneIds[i] < 100) //max_bones
		{
			//float4 localPosition = finalBoneMatrices[_input.vBoneIds[i]] * float4(_input.vPosL, 1.0f);
			//float3 localNormal = float3x3(finalBoneMatrices[_input.vBoneIds[i]]) * _input.vNormalL;
			//totalPosition += localPosition * _input.vBoneWeights[i];
		}
		else
		{
			totalPosition = float4(_input.vPosL, 1.0f);
			break;
		}
	}

	//set vertex position to totalPosition
	//Calc vertex using func

	return(calculateVertex(vertex));
}
