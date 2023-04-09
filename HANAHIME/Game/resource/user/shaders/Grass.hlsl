#include"../../engine/Math.hlsli"
#include"Grass.hlsli"

//�A���鑐�̏������l
struct GrassInitializer
{
    float3 m_pos;
    float3 m_up;
    float m_sineLength;
    int m_texIdx;
    int m_isAlive;
};

//���茋��
struct CheckResult
{
    //int m_aroundGrassCount;
    float3 m_plantPos;
    float3 m_plantNormal;
    int m_isSuccess;
};

RWStructuredBuffer<PlantGrass> aliveGrassBuffer : register(u0);
ConsumeStructuredBuffer<PlantGrass> consumeAliveGrassBuffer : register(u0);
AppendStructuredBuffer<PlantGrass> appendAliveGrassBuffer : register(u0);

RWStructuredBuffer<CheckResult> checkResultBuffer : register(u1);

Texture2D<float4> g_worldMap : register(t0);
Texture2D<float4> g_normalMap : register(t1);
Texture2D<float4> g_brightMap : register(t2);

StructuredBuffer<GrassInitializer> stackGrassInitializerBuffer : register(t3);

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
    newGrass.m_isAlive = 1;
    
    appendAliveGrassBuffer.Append(newGrass);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //�f�[�^�擾
    PlantGrass grass = aliveGrassBuffer[DTid];
  
    //���������ȉ���������C�[�W���O�^�C�}�[���X�V���A���ȏゾ������C�[�W���O�^�C�}�[�����炵�đ����͂炷�B
    if (length(grass.m_pos - commonInfo.m_playerPos) <= commonInfo.m_deathDistance)
    {
    
        //�C�[�W���O�^�C�}�[�X�V
        grass.m_appearYTimer = min(grass.m_appearYTimer + commonInfo.m_appearEaseSpeed, 1.0f);
        
    }
    else
    {
        
        //�C�[�W���O�^�C�}�[�X�V
        grass.m_appearYTimer = max(grass.m_appearYTimer - commonInfo.m_deadEaseSpeed, 0.0f);
        
        //0�ȉ��ɂȂ�����t���O��܂�B
        if (grass.m_appearYTimer <= 0)
        {
            grass.m_isAlive = false;
        }
        
    }
    
    //�C�[�W���O�ʂ����߂�
    grass.m_appearY = grass.m_appearYTimer;
    
    //�X�V���ꂽ�f�[�^��K�p
    aliveGrassBuffer[DTid] = grass;
};

[numthreads(1, 1, 1)]
void SearchPlantPos(uint DTid : SV_DispatchThreadID)
{
    
    //�X�N���[�����W���烏�[���h���W�֕ϊ��B
    CheckResult result = checkResultBuffer[DTid];
    
    //�T���񐔁B
    int searchCount = 5;
    uint2 screenPos = uint2(0, 0);
    result.m_isSuccess = false;
    for (int index = 0; index < searchCount; ++index)
    {
        
        //�����̎�
        int seed = otherTransformData.m_seed + (index * 2.0f) + DTid * 103.0f;
    
        //�T���v�����O����X�N���[�����W�����߂�B
        screenPos = uint2(RandomIntInRange(0, 1280, seed), RandomIntInRange(0, 720, seed * 2.0f));
        
        //�T���v�����O�������W�����C�g�ɓ������Ă���ʒu���ǂ����𔻒f�B
        result.m_isSuccess = 0.9f <= g_brightMap[screenPos].x;
        
        //�T���v�����O�Ɏ��s�����玟�ցB
        if (!result.m_isSuccess)
            continue;
        
        //�T���v�����O�ɐ��������烏�[���h���W�����߂�B
        result.m_plantPos = g_worldMap[screenPos].xyz;
        
        //�@�������߂�B
        result.m_plantNormal = g_normalMap[screenPos].xyz;
        
        //�����߂��ɂ��邩�������B
        bool isNearGrass = false;
        for (int grass = 0; grass < otherTransformData.m_grassCount; ++grass)
        {
            
            //�f�[�^�擾
            PlantGrass grassData = aliveGrassBuffer[grass];
    
            //����̋�����藣��Ă�������Ȃ��̂Ŕ�΂��B
            if (commonInfo.m_checkRange < distance(grassData.m_pos, result.m_plantPos))
                continue;
 
            //�����߂��ɂ���B
            isNearGrass = true;
            break;
            
        }
        
        //����F�Ő������ꂽ������������B
        for (int grassIndex = 0; grassIndex < otherTransformData.m_plantOnceCount; ++grassIndex)
        {
            
            if (grassIndex == DTid)
                continue;
            
            //�f�[�^�擾
            CheckResult grassData = checkResultBuffer[grassIndex];
    
            //����̋�����藣��Ă�������Ȃ��̂Ŕ�΂��B
            if (commonInfo.m_checkRange < distance(grassData.m_plantPos, result.m_plantPos))
                continue;
 
            //�����߂��ɂ���B
            isNearGrass = true;
            break;
            
        }
        
        //�����߂��ɂ������玟�B�Ȃ������炱�̒l��Ԃ��B
        if (isNearGrass)
        {
            continue;
        }
        else
        {
            checkResultBuffer[DTid] = result;
            return;
        }
        
    }
    
    //�����܂ł���Ƃ������Ƃ̓T���v�����O�Ɏ��s���Ă���̂ő����͂₳�Ȃ��B
    checkResultBuffer[DTid].m_isSuccess = false;
    return;
    
}