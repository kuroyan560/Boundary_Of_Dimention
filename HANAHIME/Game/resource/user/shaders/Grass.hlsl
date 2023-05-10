#include"../../engine/Math.hlsli"
#include"../../engine/Camera.hlsli"
#include"Grass.hlsli"

//植える草の初期化値
struct GrassInitializer
{
    float3 m_pos;
    float m_sineLength;
    float3 m_up;
    int m_modelIdx;
    int m_isAlive;
};

//判定結果
struct CheckResult
{
    //int m_aroundGrassCount;
    float3 m_plantPos;
    int m_isSuccess;
    float3 m_plantNormal;
    int m_pad;
};

RWStructuredBuffer<PlantGrass> aliveGrassBuffer : register(u0);
ConsumeStructuredBuffer<PlantGrass> consumeAliveGrassBuffer : register(u0);
AppendStructuredBuffer<PlantGrass> appendAliveGrassBuffer : register(u0);

RWStructuredBuffer<CheckResult> checkResultBuffer : register(u1);

StructuredBuffer<GrassInitializer> stackGrassInitializerBuffer : register(t0);
Texture2D<float4> g_worldMap : register(t1);
Texture2D<float4> g_normalMap : register(t2);
Texture2D<float4> g_brightMap : register(t3);


cbuffer cbuff0 : register(b0)
{
    TransformData otherTransformData;
}
cbuffer cbuff1 : register(b1)
{
    CommonGrassInfo commonInfo;
}

cbuffer cbuff2 : register(b2)
{
    int consumeNum;
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
    
    //イニシャライザを取得して初期化
    GrassInitializer initializer = stackGrassInitializerBuffer[DTid];
    newGrass.m_localPos = initializer.m_pos;
    newGrass.m_worldPos = initializer.m_pos;
    newGrass.m_normal = initializer.m_up;
    newGrass.m_sineLength = initializer.m_sineLength;
    newGrass.m_modelIdx = initializer.m_modelIdx;
    newGrass.m_appearYTimer = 0;
    newGrass.m_appearY = 0;
    newGrass.m_isAlive = 1;
    newGrass.m_isCheckGround = 0;
    newGrass.m_terrianIdx = -1;
    
    appendAliveGrassBuffer.Append(newGrass);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //データ取得
    PlantGrass grass = aliveGrassBuffer[DTid];
    
    //現在位置にライトが当たっているかを確認する。
    float4 viewPos = mul(commonInfo.matView, float4(grass.m_worldPos, 1.0f));
    float4 clipPos = mul(commonInfo.matProjection, viewPos);
    float3 ndcPos = clipPos.xyz / clipPos.w;
    uint2 screenPos = round(float2((ndcPos.x * 0.5f + 0.5f) * 1280.0f, (1.0f - (ndcPos.y * 0.5f + 0.5f)) * 720.0f));
    float texColor = g_brightMap[screenPos].x;
    
    //プレイヤーとの距離
    float playerDistance = length(grass.m_worldPos - otherTransformData.m_playerPos);
    bool isFar = playerDistance <= otherTransformData.m_playerPlantLightRange;
  
    //光にあたっていたら草を生やし、 当たっていなかったらイージングタイマーを減らして草を枯らす。
    if (0.9f < texColor || isFar)
    {
    
        //イージングタイマー更新
        grass.m_appearYTimer = min(grass.m_appearYTimer + commonInfo.m_appearEaseSpeed, 1.0f);
        
    }
    else
    {
        
        //イージングタイマー更新
        grass.m_appearYTimer = max(grass.m_appearYTimer - commonInfo.m_deadEaseSpeed, 0.0f);
        
        //0以下になったらフラグを折る。
        if (grass.m_appearYTimer <= 0)
        {
            grass.m_isAlive = 0;
        }
        
    }
    
    //イージング量を求める
    if (1.0f < grass.m_appearY)
    {
        grass.m_appearY += (1.0f - grass.m_appearY) / 10.0f;
    }
    else
    {
        grass.m_appearY = grass.m_appearYTimer;
    }
    
    //更新されたデータを適用
    aliveGrassBuffer[DTid] = grass;
};

[numthreads(1, 1, 1)]
void Disappear(uint DTid : SV_DispatchThreadID)
{
    for (int i = 0; i < consumeNum; ++i)
    {
        consumeAliveGrassBuffer.Consume();
    }
};

[numthreads(16, 16, 1)]
void SearchPlantPos(uint3 GlobalID : SV_DispatchThreadID, uint3 GroupID : SV_GroupID, uint3 LocalID : SV_GroupThreadID)
{
    
    //グローバルIDの計算
    const int GRASS_SPAN = 10;
    uint index = GlobalID.y * (1280 / GRASS_SPAN) + GlobalID.x;
    
    //スクリーン座標からワールド座標へ変換。
    CheckResult result = checkResultBuffer[index];
    
    //探す回数。
    uint2 screenPos = (GroupID.xy * uint2(16, 16) + LocalID.xy) * uint2(GRASS_SPAN, GRASS_SPAN);
    
    //ランダムでちょっとだけ散らす。
    uint randomScatter = 10;
    uint2 random = uint2(RandomIntInRange(otherTransformData.m_seed * LocalID.x * GlobalID.y + screenPos.x) * (randomScatter * 2), RandomIntInRange(otherTransformData.m_seed * LocalID.y * GlobalID.x + screenPos.y) * (randomScatter * 2));
    random -= uint2(randomScatter, randomScatter);
    screenPos += random;
    
    result.m_isSuccess = false;
        
    //サンプリングした座標がライトに当たっている位置かどうかを判断。
    result.m_isSuccess = step(0.9f, g_brightMap[screenPos].x);
        
    //サンプリングに失敗したら次へ。
    if (!result.m_isSuccess)
    {
        checkResultBuffer[index] = result;
        return;
    }
        
    //サンプリングに成功したらワールド座標を求める。
    result.m_plantPos = g_worldMap[screenPos].xyz;
        
    //法線も求める。
    result.m_plantNormal = g_normalMap[screenPos].xyz;
    
    //すでに生えているところにもう一度生やしていないかをチェック。
    const float DEADLINE = 1.1f;
    for (int grassIndex = 0; grassIndex < otherTransformData.m_grassCount; ++grassIndex)
    {
        float distance = length(result.m_plantPos - aliveGrassBuffer[grassIndex].m_worldPos);
        if (distance < DEADLINE)
        {
            result.m_isSuccess = false;
            break;
        }

    }
        
    checkResultBuffer[index] = result;
    
}