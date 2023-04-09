struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct DrawParticleData
{
    matrix mat;
    float4 color;
};

RWStructuredBuffer<DrawParticleData> particleData : register(u0);

VSOutput VSmain(float4 pos : POSITION,float2 uv:TEXCOORD, uint instanceID : SV_InstanceID)
{  
    VSOutput output;
    output.svpos = mul(particleData[instanceID].mat,pos);
    output.uv = uv;
    output.color = particleData[instanceID].color;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    float2 uvC = input.uv;
    return input.color;
}