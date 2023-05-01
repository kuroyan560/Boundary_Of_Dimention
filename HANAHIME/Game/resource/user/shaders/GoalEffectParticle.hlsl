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

//花火パーティクル----------------------------------------
[numthreads(PARTICLE_MAX_NUM, 1, 1)]
void InitFireWorkMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);
    //初期化
    FireWorkParticleArray[index].pos = pos;
    FireWorkParticleArray[index].timer = 60;
    FireWorkParticleArray[index].initFlag = 1;

    float maxVelue = (1 + index / 360) * 3.0f;
    float minValue = (index / 360) * 1.0f;

    float vel = Rand(index,maxVelue,0.0f);
    //ゴールから見て正面になるように配置する
    FireWorkParticleArray[index].vel = FireWorkParticleArray[index].pos + float3(0.0f,cos(AngleToRadian(index)) * vel,sin(AngleToRadian(index)) * vel);
}

[numthreads(PARTICLE_MAX_NUM, 1, 1)]
void UpdateFireWorkMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);

    //ラープで処理
    FireWorkParticleArray[index].pos = Larp(FireWorkParticleArray[index].vel,FireWorkParticleArray[index].pos,0.05f);

    bool flashFlag = false;
    //終わる
    if(FireWorkParticleArray[index].timer <= 0)
    {
        FireWorkParticleArray[index].timer = 0;
        FireWorkParticleArray[index].initFlag = false;
    }
    //消える直前になったら点滅させる
    else if(GetRate(FireWorkParticleArray[index].timer,60) <= 0.2f)
    {
        flashFlag = true;
        --FireWorkParticleArray[index].timer;
    }
    //タイマー
    else
    {
        --FireWorkParticleArray[index].timer;
    }

    //出力--------------------------------------------
    ParticleData outputMat;    
    outputMat.world = SetPosInMatrix(scaleRotate, FireWorkParticleArray[index].pos);   
    float alpha = 1.0f;
    //描画許可
    if(flashFlag)
    {
        //消滅
        alpha = GetRate(FireWorkParticleArray[index].timer,60.0f * 0.2f);
    }
    if(!FireWorkParticleArray[index].initFlag)
    {
        alpha = 0.0f;
    }
    outputMat.color = float4(1,1,1,alpha);
    particleData.Append(outputMat);
    //出力--------------------------------------------
}
//花火パーティクル----------------------------------------