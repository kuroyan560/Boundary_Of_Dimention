#include"../../engine/Math.hlsli"
#include"Grass.hlsli"

//�A���鑐�̏������l
struct GrassInitializer
{
    float3 m_pos;
    float3 m_up;
    float m_sineLength;
    int m_texIdx;
};

//���茋��
struct CheckResult
{
    int m_aroundGrassCount;
};

RWStructuredBuffer<PlantGrass> aliveGrassBuffer : register(u0);
ConsumeStructuredBuffer<PlantGrass> consumeAliveGrassBuffer : register(u0);
AppendStructuredBuffer<PlantGrass> appendAliveGrassBuffer : register(u0);

RWStructuredBuffer<CheckResult> checkResultBuffer : register(u1);

StructuredBuffer<GrassInitializer> stackGrassInitializerBuffer : register(t0);

cbuffer cbuff0 : register(b0)
{
    TransformData otherTransformData;
}

cbuffer cbuff1 : register(b1)
{
    CommonGrassInfo commonInfo;
}

[numthreads(1, 1, 1)]
void Init(uint DTid : SV_DispatchThreadID)
{
    consumeAliveGrassBuffer.Consume();
};

[numthreads(1, 1, 1)]
void Appear(uint DTid : SV_DispatchThreadID)
{
    PlantGrass newGrass;
    
    //�C�j�V�����C�U���擾���ď�����
    GrassInitializer initializer = stackGrassInitializerBuffer[DTid];
    newGrass.m_pos = initializer.m_pos;
    newGrass.m_normal = initializer.m_up;
    newGrass.m_sineLength = initializer.m_sineLength;
    newGrass.m_texIdx = initializer.m_texIdx;
    newGrass.m_appearYTimer = 0;
    newGrass.m_appearY = 0;
    
    appendAliveGrassBuffer.Append(newGrass);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //�f�[�^�擾
    PlantGrass grass = aliveGrassBuffer[DTid];
  
    //�C�[�W���O�^�C�}�[�X�V
    grass.m_appearYTimer = min(grass.m_appearYTimer + commonInfo.m_appearEaseSpeed, 1.0f);
    
    //�C�[�W���O�ʂ����߂�
    grass.m_appearY = Easing_Cubic_In(grass.m_appearYTimer, 1.0f, 0.0f, 1.0f);
    
    //�X�V���ꂽ�f�[�^��K�p
    aliveGrassBuffer[DTid] = grass;
};

[numthreads(1,1,1)]
void Check(uint DTid : SV_DispatchThreadID)
{
    //���ނ�𐶂₷�\��̈ʒu(�v���C���[�̈ʒu)�擾
    float3 appearPos = otherTransformData.m_checkPlantPos;
    
    //�f�[�^�擾
    PlantGrass grass = aliveGrassBuffer[DTid];
    
    //������x����Ă������΂��B
    bool isAwayX = (grass.m_pos.x < appearPos.x - commonInfo.m_checkClipOffset || appearPos.x + commonInfo.m_checkClipOffset < grass.m_pos.x);
    bool isAwayY = (grass.m_pos.y < appearPos.y - commonInfo.m_checkClipOffset || appearPos.y + commonInfo.m_checkClipOffset < grass.m_pos.y);
    if (isAwayX || isAwayY)
        return;

    if (commonInfo.m_checkRange < distance(grass.m_pos, appearPos))
        return;
 
    //�߂��ɂ����������C���N�������g
    CheckResult result = checkResultBuffer[0];
    result.m_aroundGrassCount++;
    checkResultBuffer[0] = result;
}