// -----------------------------------------------------
// Global variables
// -----------------------------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float4x4 gWorldMatrix : World;
float4x4 gViewInverseMatrix : ViewInverse;

// -----------------------------------------------------
// SamplerStates
// -----------------------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};
// -----------------------------------------------------
// BRDF
// -----------------------------------------------------
float4 Lambert(float kd, const float4 cd)
{
	float pi = 3.14159265358979311600;
	return cd * kd / pi ;
}

float4 Phong(float ks, float exp, const float3 l, const float3 v, const float3 n)
{
	const float3 r =	reflect(l,n);
	const float cosAlpha = saturate(dot(r,v));
	return pow(cosAlpha,ks*exp);
}
// -----------------------------------------------------
// Input/Output structs
// -----------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : COLOR;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

// -----------------------------------------------------
// Vertex Shader
// -----------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = float4(input.Position,1.f);
    output.Position = mul(output.Position, gWorldViewProj);
    output.UV = input.UV;
    output.Normal = mul(normalize(input.Normal),(float3x3)gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent),(float3x3)gWorldMatrix);
    return output;
}

// -----------------------------------------------------
// Pixel Shader
// -----------------------------------------------------
float4 PS_Combined(VS_OUTPUT input, SamplerState currentState) : SV_TARGET
{
	const float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	const float3 lightDir = float3(0.577f,-0.577f,0.577f);
	const float3 binormal = cross(input.Normal,input.Tangent);
	const float3x4 tangentSpaceAxis = float3x4(input.Tangent,binormal,input.Normal,float3(0.f,0.f,0.f));

	float3 sampledNormal = gNormalMap.Sample(currentState,input.UV).rgb;
	sampledNormal = 2.f * sampledNormal - float3( 1.f,1.f,1.f );

	const float3 normal = mul(sampledNormal,tangentSpaceAxis);

	const float observedArea = saturate(dot(normalize(normal),normalize(-lightDir)));
	
	const float intensity = 7.f ;
	const float shinyness = 25.f ;

	const float specularComponent =  gGlossinessMap.Sample(currentState, input.UV).r ;
	const float4 specular = gSpecularMap.Sample(currentState, input.UV) * Phong(shinyness,specularComponent,-lightDir,viewDirection,normal);

	const float4 finalColor = (Lambert(intensity, gDiffuseMap.Sample(currentState, input.UV)) + specular) * observedArea;
   	 return finalColor;
	
}

float4 PS_PointTechnique(VS_OUTPUT input) : SV_TARGET
{
    return PS_Combined(input, samPoint);
}

float4 PS_LinearTechnique(VS_OUTPUT input) : SV_TARGET
{
    return PS_Combined(input, samLinear);
}

float4 PS_AnisotropicTechnique(VS_OUTPUT input) : SV_TARGET
{
    return PS_Combined(input, samAnisotropic);
}




// -----------------------------------------------------
// Technique 
// -----------------------------------------------------
technique11 PointTechnique
{
    pass P0
    {
  	 SetVertexShader(CompileShader(vs_5_0, VS() ) );
	 SetGeometryShader( NULL );
	 SetPixelShader( CompileShader(ps_5_0, PS_PointTechnique() ) );
    }
}

technique11 LinearTechnique
{
    pass P0
    {
  	 SetVertexShader(CompileShader(vs_5_0, VS() ) );
	 SetGeometryShader( NULL );
	 SetPixelShader( CompileShader(ps_5_0, PS_LinearTechnique() ) );
    }
}

technique11 AnisotropicTechnique
{
    pass P0
    {
  	 SetVertexShader(CompileShader(vs_5_0, VS() ) );
	 SetGeometryShader( NULL );
	 SetPixelShader( CompileShader(ps_5_0, PS_AnisotropicTechnique() ) );
    }
}

