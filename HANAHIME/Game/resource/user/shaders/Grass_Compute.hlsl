#include"Grass.hlsli"

static const int THREAD_PER_NUM = 10;

struct ConstData
{
    float m_timeScale;
};

cbuffer cbuff0 : register(b0)
{
    ConstData g_constData;
}
RWStructuredBuffer<Grass> g_grassArray : register(u0);

//����������
[numthreads(THREAD_PER_NUM, 1, 1)]
void InitMain(uint3 DTid : SV_DispatchThreadID)
{
    //���擾
    Grass grass = g_grassArray[DTid.x];

    //����������==================
    
    //=========================

    //�X�V��̑��K�p
    g_grassArray[DTid.x] = grass;
}

//�X�V����
[numthreads(THREAD_PER_NUM, 1, 1)]
void UpdateMain(uint3 DTid : SV_DispatchThreadID)
{
    //���擾
    Grass grass = g_grassArray[DTid.x];

    //�X�V����==================
    
    //=========================

    //�X�V��̑��K�p
    g_grassArray[DTid.x] = grass;
}