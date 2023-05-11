#include"../../engine/Camera.hlsli"
#include"../../engine/ModelInfo.hlsli"
#include"Grass.hlsli"

//頂点データ
struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
};

//ピクセルシェーダーを通したデータ（レンダーターゲットに書き込むデータ）
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
};

RWStructuredBuffer<PlantGrass> aliveGrassBuffer : register(u0);

//定数バッファ（カメラ情報）
cbuffer cbuff0 : register(b0)
{
    Camera cam;
};

//定数バッファ（好きなの入れてね）
cbuffer cbuff1 : register(b1)
{
    CommonGrassInfo commonInfo;
}

cbuffer cbuff2 : register(b2)
{
    TransformData otherTransformData;
}

//テクスチャ
Texture2D<float4> baseTex : register(t0);

cbuffer cbuff3 : register(b3)
{
    Material material;
}

StructuredBuffer<int> grassIndiciesBuffer : register(t1);

//サンプラー
SamplerState smp : register(s0);
    
VSOutput VSmain(Vertex input, uint instanceID : SV_InstanceID)
{
    PlantGrass grass = aliveGrassBuffer[grassIndiciesBuffer[instanceID]];
    
    VSOutput output;
    float4 wpos = input.pos;
    wpos.xyz += grass.m_worldPos;
    output.svpos = mul(cam.view, wpos);
    output.depthInView = output.svpos.z;
    output.svpos = mul(cam.proj, output.svpos);
    output.worldpos = wpos;
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

PSOutput PSmain(VSOutput input)
{
    PSOutput output;
    
    //法線を取得する。
    float3 normal = input.normal;
    
    //プレイヤーと描画する座標のベクトル
    //float3 playerDir = normalize(input.worldPosition.xyz - commonInfo.m_playerPos);
    float3 lightDir = normalize(commonInfo.m_playerPos - cam.eyePos);
    
    //距離を求める。
    float distance = length(input.worldpos.xyz - commonInfo.m_playerPos);
    
    //距離によって明るさの割合を変える。
    const float DISTANCE = 8.0f;
    const float OFFSET_DISTANCE_LUMI = 0.2f; //距離が遠く離れていてもある程度明るさを出すためのオフセット。
    float distanceRate = clamp(step(distance, DISTANCE), OFFSET_DISTANCE_LUMI, 1.0f);
    
    //明るさを求める。
    float lumi = dot(normal, lightDir) * distanceRate;
    
    //明るさのオフセット
    const float OFFSET_LUMI = 0.3f;
    lumi = clamp(lumi + OFFSET_LUMI, OFFSET_LUMI, 1.0f);
    
    //距離によって最終的な明るさをクランプする。
    const float IN_CIRCLE_LUMI = 0.5f;
    lumi = clamp(lumi, step(distance, DISTANCE) * IN_CIRCLE_LUMI, 1.0f);
    
    //float bright = distance;

    float4 color = baseTex.Sample(smp, input.uv);
    //色を保存する。
    color.xyz *= lumi;
    output.color = color;

    output.emissive = float4(0, 0, 0, 0);
    output.depth = input.depthInView;
    output.edgeColor = float4(0.13, 0.53, 0.40, 1);
 
    return output;
}