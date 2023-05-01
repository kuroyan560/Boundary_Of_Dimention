#include"GPUParticle.hlsli"


struct FirePartilceData
{
    float3 pos;
    float3 vel;
    int timer;
    int initFlag;
};

RWStructuredBuffer<FirePartilceData> FireWorkParticleArray : register(u0);
AppendStructuredBuffer<ParticleData> particleData : register(u1);

cbuffer RootConstants : register(b0)
{    
    matrix scaleRotate;
};

cbuffer RootConstants2 : register(b0)
{    
    float3 pos;
    float3 vel;
};

static const int PARTICLE_MAX_NUM = 1024;

//�ԉ΃p�[�e�B�N��----------------------------------------
[numthreads(PARTICLE_MAX_NUM, 1, 1)]
void InitFireWorkMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);
    //������
    FireWorkParticleArray[index].pos = pos;
    FireWorkParticleArray[index].timer = 60;
    FireWorkParticleArray[index].initFlag = 1;

    float maxVelue = (1 + index / 360) * 3.0f;
    float minValue = (index / 360) * 1.0f;

    float vel = Rand(index,maxVelue,0.0f);
    //�S�[�����猩�Đ��ʂɂȂ�悤�ɔz�u����
    FireWorkParticleArray[index].vel = FireWorkParticleArray[index].pos + float3(0.0f,cos(AngleToRadian(index)) * vel,sin(AngleToRadian(index)) * vel);
}

[numthreads(PARTICLE_MAX_NUM, 1, 1)]
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
    outputMat.world = SetPosInMatrix(scaleRotate, FireWorkParticleArray[index].pos);   
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