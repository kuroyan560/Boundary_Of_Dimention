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


Texture2D<float4> tex : register(t0);
SamplerState gpuParticleSmp : register(s0);

struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
};
PSOutput PSmain(VSOutput input) : SV_TARGET
{
    float4 texColor = float4(tex.Sample(gpuParticleSmp, input.uv));

    PSOutput output;
    output.color = texColor * input.color;
    output.emissive = texColor * input.color;
    output.emissive.a = clamp(output.emissive.a,0.0f,0.3f);
    return output;
}