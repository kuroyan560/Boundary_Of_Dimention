#include"../../engine/Math.hlsli"
#include"Grass.hlsli"

//植える草の初期化値
struct GrassInitializer
{
    float3 m_pos;
    float3 m_up;
    float m_sineLength;
    int m_texIdx;
    int m_isAlive;
};

//判定結果
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
    
    //イニシャライザを取得して初期化
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
    //データ取得
    PlantGrass grass = aliveGrassBuffer[DTid];
  
    //距離が一定以下だったらイージングタイマーを更新し、一定以上だったらイージングタイマーを減らして草を枯らす。
    if (length(grass.m_pos - commonInfo.m_playerPos) <= commonInfo.m_deathDistance)
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
            grass.m_isAlive = false;
        }
        
    }
    
    //イージング量を求める
    grass.m_appearY = grass.m_appearYTimer;
    
    //更新されたデータを適用
    aliveGrassBuffer[DTid] = grass;
};

[numthreads(1, 1, 1)]
void SearchPlantPos(uint DTid : SV_DispatchThreadID)
{
    
    //スクリーン座標からワールド座標へ変換。
    CheckResult result = checkResultBuffer[DTid];
    
    //探す回数。
    int searchCount = 5;
    uint2 screenPos = uint2(0, 0);
    result.m_isSuccess = false;
    for (int index = 0; index < searchCount; ++index)
    {
        
        //乱数の種
        int seed = otherTransformData.m_seed + (index * 2.0f) + DTid * 103.0f;
    
        //サンプリングするスクリーン座標を求める。
        screenPos = uint2(RandomIntInRange(0, 1280, seed), RandomIntInRange(0, 720, seed * 2.0f));
        
        //サンプリングした座標がライトに当たっている位置かどうかを判断。
        result.m_isSuccess = 0.9f <= g_brightMap[screenPos].x;
        
        //サンプリングに失敗したら次へ。
        if (!result.m_isSuccess)
            continue;
        
        //サンプリングに成功したらワールド座標を求める。
        result.m_plantPos = g_worldMap[screenPos].xyz;
        
        //法線も求める。
        result.m_plantNormal = g_normalMap[screenPos].xyz;
        
        //草が近くにあるかを検索。
        bool isNearGrass = false;
        for (int grass = 0; grass < otherTransformData.m_grassCount; ++grass)
        {
            
            //データ取得
            PlantGrass grassData = aliveGrassBuffer[grass];
    
            //既定の距離より離れていたら問題ないので飛ばす。
            if (commonInfo.m_checkRange < distance(grassData.m_pos, result.m_plantPos))
                continue;
 
            //草が近くにある。
            isNearGrass = true;
            break;
            
        }
        
        //このFで生成された草も検索する。
        for (int grassIndex = 0; grassIndex < otherTransformData.m_plantOnceCount; ++grassIndex)
        {
            
            if (grassIndex == DTid)
                continue;
            
            //データ取得
            CheckResult grassData = checkResultBuffer[grassIndex];
    
            //既定の距離より離れていたら問題ないので飛ばす。
            if (commonInfo.m_checkRange < distance(grassData.m_plantPos, result.m_plantPos))
                continue;
 
            //草が近くにある。
            isNearGrass = true;
            break;
            
        }
        
        //草が近くにあったら次。なかったらこの値を返す。
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
    
    //ここまでくるということはサンプリングに失敗しているので草をはやさない。
    checkResultBuffer[DTid].m_isSuccess = false;
    return;
    
}