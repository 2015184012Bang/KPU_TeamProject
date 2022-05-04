cbuffer cbWorld : register(b0)
{
    row_major matrix gWorld;
}

cbuffer cbViewProj : register(b1)
{
    row_major matrix gViewProj;
}

cbuffer cbLight : register(b3)
{
    float3 gAmbientColor : packoffset(c0);
    float gSpecularStrength : packoffset(c0.w);
    float3 gLightPos : packoffset(c1);
    float3 gCameraPos : packoffset(c2);
}

Texture2D tex0 : register(t0);
SamplerState sam0 : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float3 fragPos : POSITION;
    float3 normal : NORMAL;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = mul(float4(input.position, 1.0f), gWorld);
    result.fragPos = (float3)result.position;
    result.position = mul(result.position, gViewProj);
    result.uv = input.uv;    
    result.normal = mul(input.normal, (float3x3)gWorld);
    
    return result;
}


float4 PSMain(PSInput input) : SV_Target
{
    float3 norm = normalize(input.normal);
    float3 lightDir = normalize(gLightPos - input.fragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    float3 diffuse = float3(diff, diff, diff);
    
    float3 viewDir = normalize(gCameraPos - input.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    float3 specular = gSpecularStrength * spec;
    
    float4 color = tex0.Sample(sam0, input.uv);
    color.xyz = (gAmbientColor + diffuse + specular) * color.xyz;
    return color;
}