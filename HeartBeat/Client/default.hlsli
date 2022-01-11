cbuffer cbWorld : register(b0)
{
    row_major matrix gWorld;
}

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D tex0 : register(t0);
SamplerState sam0 : register(s0);

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = mul(float4(input.position, 1.0f), gWorld);
    result.uv = input.uv;
    
    return result;
}

float4 PSMain(PSInput input) : SV_Target
{
    float4 color = tex0.Sample(sam0, input.uv);
    return color;
}