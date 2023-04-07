#include"../../engine/Camera.hlsli"
#include"MaskInk.hlsli"

RWStructuredBuffer<MaskInk> aliveInkBuffer : register(u0);
ConsumeStructuredBuffer<MaskInk> consumeAliveInkBuffer : register(u0);
AppendStructuredBuffer<MaskInk> appendAliveInkBuffer : register(u0);

StructuredBuffer<float3> stackInkPosBuffer : register(t0);

cbuffer cbuff0 : register(b0)
{
    ConstData constData;
}

//�^������(-1.0f ~ 1.0f)
float GetRand(float arg_seed)
{
    float rand = frac(sin(arg_seed) * 143758.5453);
    //-1.0f ~ 1.0f�Ɋg��
    rand = rand * 2.0f - 1.0f;
    return rand;
}
float3 GetRand(float3 arg_seed)
{
    return float3(GetRand(arg_seed.x), GetRand(arg_seed.y), GetRand(arg_seed.z));
}

[numthreads(1, 1, 1)]
void Init(uint DTid : SV_DispatchThreadID)
{
    consumeAliveInkBuffer.Consume();
};

[numthreads(1, 1, 1)]
void Appear(uint DTid : SV_DispatchThreadID)
{
    MaskInk newInk;
    newInk.m_pos = stackInkPosBuffer[DTid];
    newInk.m_scale = constData.m_initScale;
    newInk.m_texIdx = 0;
    newInk.m_posOffset = GetRand(newInk.m_pos) * constData.m_posOffsetMax;
    appendAliveInkBuffer.Append(newInk);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //�f�[�^�擾
    MaskInk ink = aliveInkBuffer[DTid];
  
    //���̃e�N�X�`����
    ink.m_texIdx = ink.m_texIdx + 1;
    //����Ȃ�O�ɖ߂�
    if (constData.m_texMax <= ink.m_texIdx)
        ink.m_texIdx = 0;
    
    //���W�I�t�Z�b�g�X�V
    ink.m_posOffset = GetRand(ink.m_posOffset) * constData.m_posOffsetMax;
    
    //�X�V���ꂽ�f�[�^��K�p
    aliveInkBuffer[DTid] = ink;
};