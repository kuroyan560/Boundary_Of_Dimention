#include"../../engine/Camera.hlsli"
#include"../../engine/LightInfo.hlsli"
#include"../../engine/ModelInfo.hlsli"

struct ToonCommonParameter
{
    float m_brightThresholdLow;
    float m_brightThresholdRange;
    float m_monochromeRate;
};

struct ToonIndividualParameter
{
    float4 m_fillColor;
    float4 m_brightMulColor;
    float4 m_darkMulColor;
    float4 m_edgeColor;
    int m_drawMask;
    float m_alpha;
};

struct PlayerInfo
{
    float3 m_worldPos;
    float2 m_screenPos;
};

//アクティブ中の植物繁殖ライトの数
struct LightInfo_Plant
{
    uint ptLigNum;
    uint spotLigNum;
};

//植物繁殖ポイントライト
struct PointLight_Plant
{
    float3 m_pos;
    float m_influenceRange;
    float m_defInfluenceRange;
    uint m_active;
};

//植物繁殖スポットライト
struct SpotLight_Plant
{
    float3 m_pos;
    float m_influenceRange;
    float3 m_vec;
    float m_angle;
    uint m_active;
};

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    LightInfo ligNum; //アクティブ中のライトの数の情報
}

StructuredBuffer<DirectionLight> dirLight : register(t0);
StructuredBuffer<PointLight> pointLight : register(t1);
StructuredBuffer<SpotLight> spotLight : register(t2);
StructuredBuffer<HemiSphereLight> hemiSphereLight : register(t3);


cbuffer cbuff3 : register(b3)
{
    matrix bones[256]; //ボーン行列
}

Texture2D<float4> baseTex : register(t4);
Texture2D<float4> emissiveTex : register(t5);
SamplerState smp : register(s0);

cbuffer cbuff4 : register(b4)
{
    Material material;
}

cbuffer cbuff5 : register(b5)
{
    ToonCommonParameter toonCommonParam;
}

cbuffer cbuff6 : register(b6)
{
    ToonIndividualParameter toonIndividualParam;
}

cbuffer cbuff7 : register(b7)
{
    PlayerInfo player;
}

cbuffer cbuff8 : register(b8)
{
    LightInfo_Plant ligNum_Plant;
}

StructuredBuffer<PointLight_Plant> pointLight_Plant : register(t6);
StructuredBuffer<SpotLight_Plant> spotLight_Plant : register(t7);

struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
    float4 bright : SV_Target4;
    float4 normal : SV_Target5;
    float4 normalGrass : SV_Target6;
    float4 worldPos : SV_Target7;
};