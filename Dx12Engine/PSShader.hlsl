Texture2D albedoTex : register(t0);
Texture2D aoTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D roughnessTex : register(t4);

SamplerState g_sampler : register(s0);

cbuffer GLOBAL_CONSTANT : register(b0)
{
    matrix ViewProj;
    float4 eyePos;
    float4 lightPos;
};

cbuffer MATERIAL_CONSTANT : register(b3)
{
    int useNormalMap;
}

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
};

float3 GetNormal(PS_INPUT input)
{
    float3 normalWorld = normalize(input.normal);
    
    if (useNormalMap)
    {
        float3 normal = normalTex.Sample(g_sampler, input.uv).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]
        
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 normalWorld = GetNormal(input);
    
    float3 lightDir = normalize(lightPos.xyz - input.worldPos);
    float3 viewDir = normalize(eyePos.xyz - input.worldPos);
    float3 reflectDir = reflect(-lightDir, normalWorld);

    // Ambient
    float3 ambient = float3(0.5f, 0.5f, 0.5f);
      
    // Diffuse
    float diff = max(dot(normalWorld, lightDir), 0.0f);
    float3 diffuse = diff * float3(1.0f, 1.0f, 1.0f);
     
    // Specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 5.0f); // shininess
    float3 specular = spec * float3(1.0f, 1.0f, 1.0f);

    float3 lighting = ambient + diffuse + specular;
    float4 color = albedoTex.Sample(g_sampler, input.uv);
    
    return float4(color.rgb * lighting, color.a);
}