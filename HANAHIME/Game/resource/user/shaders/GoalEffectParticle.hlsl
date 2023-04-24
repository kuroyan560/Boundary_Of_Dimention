#include"GPUParticle.hlsli"

struct LocusPartilceData
{
    float3 pos;
    float3 vel;
    int timer;
    int id;
    int initFlag;
};

RWStructuredBuffer<LocusPartilceData> LocusParticleArray : register(u0);
RWStructuredBuffer<float3> EmittPos : register(u1);
AppendStructuredBuffer<ParticleData> OutputParticle : register(u1);

cbuffer scaleRota:register(b0)
{
    matrix scaleRota;
}

//�O�Ճp�[�e�B�N��----------------------------------------
[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);

    if(LocusParticleArray[index].initFlag)
    {
        return;
    }

    //�����l������
    LocusParticleArray[index].vel = float3(0,-0.1f,0);
    LocusParticleArray[index].timer = 60;
    LocusParticleArray[index].initFlag = true;

    //�ǂ̍��W�ɂ��Ă�����ID��t����
    if(index < 1024 / 2)
    {
        LocusParticleArray[index].id = 0;
    }
    else
    {
        LocusParticleArray[index].id = 1;
    }
    
    uint id = LocusParticleArray[index].id;
    LocusParticleArray[index].pos = EmittPos[id];
}

[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);

    //���ԂƋ��ɏ���
    if(LocusParticleArray[index].timer <= 0)
    {
        LocusParticleArray[index].initFlag = false;
    }
    else
    {
        LocusParticleArray[index].pos += LocusParticleArray[index].vel;
        --LocusParticleArray[index].timer;
    }
    
    //�o��--------------------------------------------
    ParticleData outputMat;
    outputMat.world = SetPosInMatrix(scaleRota,LocusParticleArray[index].pos);
    outputMat.color = float4(1,1,1,GetRate(LocusParticleArray[index].timer,60));
    if(LocusParticleArray[index].initFlag)
    {
        OutputParticle.Append(outputMat);
    }
    //�o��--------------------------------------------
}
//�O�Ճp�[�e�B�N��----------------------------------------


struct FirePartilceData
{
    float3 pos;
    float3 vel;
    int timer;
    int initFlag;
};

RWStructuredBuffer<FirePartilceData> FireWorkParticleArray : register(u0);
AppendStructuredBuffer<ParticleData> particleData : register(u1);

//�ԉ΃p�[�e�B�N��----------------------------------------
[numthreads(1024, 1, 1)]
void InitFireWorkMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);
    //������
    FireWorkParticleArray[index].pos = float3(10,10,0);
    FireWorkParticleArray[index].timer = 60;
    FireWorkParticleArray[index].initFlag = 1;

    float maxVelue = (1 + index / 360) * 10.0f;
    float minValue = (index / 360) * 5.0f;

    float vel = Rand(index,maxVelue,0.0f);
    //�S�[�����猩�Đ��ʂɂȂ�悤�ɔz�u����
    FireWorkParticleArray[index].vel = FireWorkParticleArray[index].pos + float3(cos(AngleToRadian(index)) * vel,sin(AngleToRadian(index)) * vel,0.0f);
}

[numthreads(1024, 1, 1)]
void UpdateFireWorkMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);

    //���[�v�ŏ���
    FireWorkParticleArray[index].pos = Larp(FireWorkParticleArray[index].vel,FireWorkParticleArray[index].pos,0.05f);

    bool flashFlag = false;
    //�I���
    if(FireWorkParticleArray[index].timer <= 0)
    {
        FireWorkParticleArray[index].timer = 0;
        FireWorkParticleArray[index].initFlag = false;
    }
    //�����钼�O�ɂȂ�����_�ł�����
    else if(GetRate(FireWorkParticleArray[index].timer,60) <= 0.2f)
    {
        flashFlag = true;
        --FireWorkParticleArray[index].timer;
    }
    //�^�C�}�[
    else
    {
        --FireWorkParticleArray[index].timer;
    }

    //�o��--------------------------------------------
    ParticleData outputMat;
    outputMat.world = SetPosInMatrix(scaleRota,FireWorkParticleArray[index].pos);
    float alpha = 1.0f;
    //�`�拖��
    if(flashFlag)
    {
        //����
        alpha = GetRate(FireWorkParticleArray[index].timer,60.0f * 0.2f);
    }
    if(!FireWorkParticleArray[index].initFlag)
    {
        alpha = 0.0f;
    }
    outputMat.color = float4(1,1,1,alpha);
    particleData.Append(outputMat);
    //�o��--------------------------------------------
}
//�ԉ΃p�[�e�B�N��----------------------------------------