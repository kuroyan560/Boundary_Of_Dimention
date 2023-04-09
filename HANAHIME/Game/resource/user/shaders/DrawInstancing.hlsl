struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 color : COLOR;
};

struct DrawParticleData
{
    matrix mat;
    float4 color;
};

RWStructuredBuffer<DrawParticleData> particleData : register(u0);

VSOutput VSmain(float4 pos : POSITION, uint instanceID : SV_InstanceID)
{  
    VSOutput output;
    output.svpos = mul(particleData[instanceID].mat,pos);
    output.color = particleData[instanceID].color;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    return input.color;
}