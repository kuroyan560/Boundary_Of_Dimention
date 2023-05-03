struct EdgeParameter
{
    matrix m_matView;
    matrix m_matProj;
    //�G�b�W�`��̔��f������[�x���̂������l
    float m_depthThreshold;
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
Texture2D<float4> g_worldMap : register(t4);
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
    
    //��ʂ̒��S
    float2 centerPos = float2(0.5f, 0.5f);
    
    //���̃s�N�Z���Ɍ����������Ă��邩
    float myBright = g_brightMap.Sample(g_sampler, input.m_uv).x;
    float defBright = g_brightMap.Sample(g_sampler, input.m_uv).y; //�f�t�H���g�̃��C�g�͈̔�
    float playerBright = g_brightMap.Sample(g_sampler, input.m_uv).z; //�v���C���[�p�A�E�g���C��
    
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

    //�v���C���[�̃G�b�W�̑���
    edgeThickness = 0.002f;
    float2 playerEdgeOffsetUV[8];
    playerEdgeOffsetUV[0] = float2(edgeThickness, 0.0f);
    playerEdgeOffsetUV[1] = float2(-edgeThickness, 0.0f);
    playerEdgeOffsetUV[2] = float2(0.0f, edgeThickness);
    playerEdgeOffsetUV[3] = float2(0.0f, -edgeThickness);
    playerEdgeOffsetUV[4] = float2(edgeThickness, edgeThickness);
    playerEdgeOffsetUV[5] = float2(-edgeThickness, edgeThickness);
    playerEdgeOffsetUV[6] = float2(edgeThickness, -edgeThickness);
    playerEdgeOffsetUV[7] = float2(-edgeThickness, -edgeThickness);

    // �ߖT8�e�N�Z���̐[�x�l�̍��̕��ϒl���v�Z����
    float depthDiffer = 0.0f;
    for( int i = 0; i < 8; i++)
    {

        //�v���C���[�̗֊s���̏���
        if (playerBright == 0)
        {
        
            //���邳�擾
            float2 brihgtPickUv = input.m_uv + playerEdgeOffsetUV[i];
            float pickBright = g_brightMap.Sample(g_sampler, brihgtPickUv).z;
            if (pickBright != playerBright)
            {
                return float4(0.35f, 0.90f, 0.57f, 1.0f);
            }

        }

        //���C�g�͈̗̔͂֊s��
        if(myBright == 0){
        
            //���邳�擾
            float2 brihgtPickUv = input.m_uv + edgeOffsetUV[i];
            float pickBright = g_brightMap.Sample(g_sampler, brihgtPickUv).x;
            float isPlayer = g_brightMap.Sample(g_sampler, brihgtPickUv).z;
            if(pickBright != myBright && !isPlayer)
            {
                return g_edgeColorMap.Sample(g_sampler, brihgtPickUv);
            }

        }

        //�f�t�H���g�̃��C�g�͈̗̔͂֊s��
        if (defBright == 0)
        {
        
            //���邳�擾
            float2 brihgtPickUv = input.m_uv + edgeOffsetUV[i];
            float pickBright = saturate(g_brightMap.Sample(g_sampler, brihgtPickUv).y);
            float isPlayer = g_brightMap.Sample(g_sampler, brihgtPickUv).z;
            if (pickBright != defBright && !isPlayer)
            {
                return float4(0.5f, 0.5f, 0.5f, 1.0f);
            }

        }
        
        //�[�x�擾
        float2 pickUv = input.m_uv + edgeOffsetUV[i];
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
    if (!sameNormal && m_edgeParam.m_depthThreshold <= depthDiffer)
    {
        //��Ԏ�O���̃G�b�W�J���[���̗p����
        return g_edgeColorMap.Sample(g_sampler, nearestUv);
    }
    
    discard;
    return float4(0.0f,0.0f,0.0f,0.0f);
}