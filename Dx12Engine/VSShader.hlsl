cbuffer GLOBAL_CONSTANT : register(b0)
{
    matrix ViewProj;
    float4 eyePos;
    float4 lightPos;
};

cbuffer MODEL_CONSTANT : register(b1)
{
    matrix Model;
    matrix NormalModel;
};

cbuffer SkinnedConstants : register(b2)
{
    Matrix boneTransforms[64];
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    
    float4 boneWeights0 : BLENDWEIGHT0;
    uint4 boneIndices0 : BLENDINDICES0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
};
 
PSInput VSMain(VS_INPUT input)
{
    PSInput Output;
    
    float weights[4];
    weights[0] = input.boneWeights0.x;
    weights[1] = input.boneWeights0.y;
    weights[2] = input.boneWeights0.z;
    weights[3] = input.boneWeights0.w;
    
    uint indices[4];
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    
    //float3 posModel = float3(0.0f, 0.0f, 0.0f);
    //float3 normalModel = float3(0.0f, 0.0f, 0.0f);
   // float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    float3 posModel = input.pos;
    float3 normalModel = input.normal;
    float3 tangentModel = input.tangent;

    // Uniform Scaling 가정
    // (float3x3)boneTransforms 캐스팅으로 Translation 제외
    /*
    for (int i = 0; i < 4; ++i)
    {
        if (indices[i] != 255)
        {
            float4x4 boneMatrix = boneTransforms[indices[i]];

            posModel += mul(float4(input.pos, 1.0), boneMatrix).xyz * weights[i];
            normalModel += mul(input.normal, (float3x3) boneMatrix) * weights[i];
            tangentModel += mul(input.tangent, (float3x3) boneMatrix) * weights[i];
    
        }
    }*/
    
    
    float4 worldPos = mul(float4(posModel, 1.0f), Model);
    Output.worldPos = worldPos.xyz;
    Output.position = mul(worldPos, ViewProj);
    Output.normal = normalize(mul(float4(normalModel, 0.0f), NormalModel).xyz);
    Output.tangent = mul(float4(tangentModel, 0.0f), Model).xyz;
    Output.uv = input.uv;

    return Output;
}

