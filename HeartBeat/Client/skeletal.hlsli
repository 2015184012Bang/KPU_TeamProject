cbuffer cbWorld : register(b0)
{
    row_major matrix gWorld;
}

cbuffer cbViewProj : register(b1)
{
    row_major matrix gViewProj;
}

cbuffer cbBoneTransform : register(b2)
{
    row_major matrix gBoneTransform[96];
}

Texture2D tex0 : register(t0);
SamplerState sam0 : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    uint4 bone : BONE;
    float4 weight : WEIGHT;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    float4 pos = float4(input.position, 1.0f);
    
    float4 skinnedPos = input.weight.x * mul(pos, gBoneTransform[input.bone.x]);
    skinnedPos += input.weight.y * mul(pos, gBoneTransform[input.bone.y]);
    skinnedPos += input.weight.z * mul(pos, gBoneTransform[input.bone.z]);
    skinnedPos += input.weight.w * mul(pos, gBoneTransform[input.bone.w]);
    
    skinnedPos = mul(skinnedPos, gWorld);
    
    result.position = mul(skinnedPos, gViewProj);
    result.uv = input.uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = tex0.Sample(sam0, input.uv);
    return color;
}
