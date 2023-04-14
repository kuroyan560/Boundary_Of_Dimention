#include"../../engine/Math.hlsli"
#include"Grass.hlsli"
#include"../../engine/Camera.hlsli"

//植える草の初期化値
struct GrassInitializer
{
    float3 m_pos;
    float m_sineLength;
    float3 m_up;
    int m_texIdx;
    float3 m_pad;
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

RWStructuredBuffer<uint> sortAndDisappearNumBuffer : register(u1);

RWStructuredBuffer<CheckResult> checkResultBuffer : register(u2);

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
    Camera camera;
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
    newGrass.m_pad = 1;
    
    appendAliveGrassBuffer.Append(newGrass);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //データ取得
    PlantGrass grass = aliveGrassBuffer[DTid];
    
    //現在位置にライトが当たっているかを確認する。
    float4 viewPos = mul(camera.view, float4(grass.m_pos, 1.0f));
    float4 clipPos = mul(camera.proj, viewPos);
    float3 ndcPos = clipPos.xyz / clipPos.w;
    uint2 screenPos = round(float2((ndcPos.x * 0.5f + 0.5f) * 1280.0f, (1.0f - (ndcPos.y * 0.5f + 0.5f)) * 720.0f));
    float texColor = g_brightMap[screenPos].x;
  
    //光にあたっていたら草を生やし、 当たっていなかったらイージングタイマーを減らして草を枯らす。
    if (0.9f < texColor)
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

void SwapGrass(inout PlantGrass a, inout PlantGrass b)
{
    PlantGrass tmp = a;
    a = b;
    b = tmp;
}

[numthreads(1, 1, 1)]
void Sort(uint DTid : SV_DispatchThreadID)
{
    //sortAndDisappearNumBufferには生きている草のカウントが格納されている
    uint aliveGrassCount = sortAndDisappearNumBuffer[0];
    uint consumeCount = 0;
    
    for (int i = 0; i < aliveGrassCount; ++i)
    {
        PlantGrass grass = aliveGrassBuffer[i];
        
        //既に死んでいるものだった場合
        if (grass.m_isAlive == 0)
        {
            //死んでいるものが末尾に来るよう交換
            for (int j = aliveGrassCount - 1; 0 <= j; --j)
            {
                if (i == j)
                    break;
                
                if (aliveGrassBuffer[j].m_isAlive)
                {
                    SwapGrass(aliveGrassBuffer[i], aliveGrassBuffer[j]);
                    break;
                }
            }
            consumeCount++;
        }
    }
    sortAndDisappearNumBuffer[0] = consumeCount;
};

[numthreads(1, 1, 1)]
void Disappear(uint DTid : SV_DispatchThreadID)
{
    for (int i = 0; i < sortAndDisappearNumBuffer[0]; ++i)
    {
        consumeAliveGrassBuffer.Consume();
    }
};

[numthreads(1, 1, 1)]
void SearchPlantPos(uint DTid : SV_DispatchThreadID)
{
    
    //スクリーン座標からワールド座標へ変換。
    CheckResult result = checkResultBuffer[DTid];
    
    //探す回数。
    uint2 screenPos = uint2(0, 0);
    result.m_isSuccess = false;
    
    //乱数の種
    float seed = otherTransformData.m_seed + DTid * 103.0f;
    
    //サンプリングするスクリーン座標を求める。
    screenPos = uint2(RandomIntInRange(0, 1280, seed), RandomIntInRange(0, 720, seed * 2.0f));
        
    //サンプリングした座標がライトに当たっている位置かどうかを判断。
    result.m_isSuccess = step(0.9f, g_brightMap[screenPos].x);
        
    //サンプリングに失敗したら次へ。
    if (!result.m_isSuccess)
    {
        checkResultBuffer[DTid] = result;
        return;
    }
        
    //サンプリングに成功したらワールド座標を求める。
    result.m_plantPos = g_worldMap[screenPos].xyz;
        
    //法線も求める。
    result.m_plantNormal = g_normalMap[screenPos].xyz;
        
    checkResultBuffer[DTid] = result;
    
}