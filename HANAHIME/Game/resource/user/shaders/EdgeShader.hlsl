struct EdgeParameter
{
    //�G�b�W�`��̔��f������[�x���̂������l
    float m_depthThreshold;
    //�[�x�l���ׂ�e�N�Z���ւ�UV�I�t�Z�b�g
    float2 m_uvOffset[8];
};


struct VSOutput
{
    float4 m_pos : SV_POSITION;
    float2 m_uv : TEXCORRD;
};

#define FLT_EPSILON 0.001

Texture2D<float4> g_depthMap : register(t0);
Texture2D<float4> g_brightMap : register(t1);
Texture2D<float4> g_edgeColorMap : register(t2);
Texture2D<float4> g_normalMap : register(t3);
SamplerState g_sampler : register(s0);
cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat;
}

cbuffer cbuff0 : register(b1)
{
    EdgeParameter m_edgeParam;
}

VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //�s�N�Z���V�F�[�_�ɓn���l
    output.m_pos = mul(parallelProjMat, pos);
    pos.z = 0.0f;
    output.m_uv = uv;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    //���̃s�N�Z���Ɍ����������Ă��邩
    float myBright = g_brightMap.Sample(g_sampler, input.m_uv).x;
    
    // ���̃s�N�Z���̐[�x�l���擾
    float depth = g_depthMap.Sample(g_sampler, input.m_uv).x;

    //��ԋ߂��i�[�x�l���Ⴂ�j�s�N�Z�����L�^
    float nearest = depth;
    float2 nearestUv = input.m_uv;

    //�G�b�W�̑���
    float edgeThickness = 0.002f;
    float2 edgeOffsetUV[8];
    edgeOffsetUV[0] = float2(edgeThickness, 0.0f);
    edgeOffsetUV[1] = float2(-edgeThickness, 0.0f);
    edgeOffsetUV[2] = float2(0.0f, edgeThickness);
    edgeOffsetUV[3] = float2(0.0f, -edgeThickness);
    edgeOffsetUV[4] = float2(edgeThickness, edgeThickness);
    edgeOffsetUV[5] = float2(-edgeThickness, edgeThickness);
    edgeOffsetUV[6] = float2(edgeThickness, -edgeThickness);
    edgeOffsetUV[7] = float2(-edgeThickness, -edgeThickness);
    
    // �ߖT8�e�N�Z���̐[�x�l�̍��̕��ϒl���v�Z����
    float depthDiffer = 0.0f;
    for( int i = 0; i < 8; i++)
    {

        //�Â��Ƃ��낾������
        if(myBright == 0){
        
            //���邳�擾
            float2 brihgtPickUv = input.m_uv + edgeOffsetUV[i];
            float pickBright = g_brightMap.Sample(g_sampler, brihgtPickUv).x;
            if(pickBright != myBright)
            {
                return g_edgeColorMap.Sample(g_sampler, brihgtPickUv);
            }

        }
        
        //�[�x�擾
        float2 pickUv = input.m_uv + m_edgeParam.m_uvOffset[i];
        float pickDepth = g_depthMap.Sample(g_sampler, pickUv).x;
        depthDiffer += abs(depth - pickDepth);
        
        //�[�x�l�ŏ����X�V
        if(pickDepth < nearest)
        {
            nearestUv = pickUv;
            nearest = pickDepth;
        }
    }
    depthDiffer /= 8.0f;
    
    //���̃s�N�Z���̖@���擾
    float3 normal = g_normalMap.Sample(g_sampler, input.m_uv).xyz;
    //��ԋ߂��s�N�Z���̖@���擾
    float3 nearestNormal = g_normalMap.Sample(g_sampler, nearestUv).xyz;
    //�@�����ꏏ��
    bool sameNormal = (length(normal - nearestNormal) < FLT_EPSILON);
    
    // ���g�̐[�x�l�ƋߖT8�e�N�Z���̐[�x�l�̍��𒲂ׂ�
    // �@�����قȂ�@���@�[�x�l�����\�Ⴄ�ꍇ�̓G�b�W�o��
    if (!sameNormal || m_edgeParam.m_depthThreshold <= depthDiffer)
    {
        //��Ԏ�O���̃G�b�W�J���[���̗p����
        return g_edgeColorMap.Sample(g_sampler, nearestUv);
    }
    
    discard;
    return float4(0.0f,0.0f,0.0f,0.0f);
}