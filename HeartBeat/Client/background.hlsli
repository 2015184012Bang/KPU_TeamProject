cbuffer cbWorld : register(b0)
{
    row_major matrix gWorld;
}

cbuffer cbViewProj : register(b1)
{
    row_major matrix gViewProj;
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
};

PSInput VSMain(VSInput input)
{
    PSInput result = (PSInput)0;
    
    result.position = float4(input.position, 1.0f);
    result.position.z = 0.999999f;
    result.uv = input.uv;
    
    return result;
}

float4 PSMain(PSInput input) : SV_Target
{
    float4 color = tex0.Sample(sam0, input.uv);
    return color;
}