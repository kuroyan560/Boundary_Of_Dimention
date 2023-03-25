#include"../../engine/Camera.hlsli"

//最大インスタンス数　※CPU側と合わせる
static const int INSTANCE_MAX = 1024;

//頂点データ
struct VSOutput
{
    float3 position : POSITION;
    uint texID : TexID;
    float3 normal : NORMAL;
    uint isAlive : IsAlive;
};

//ジオメトリシェーダーを通したデータ
struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

//ピクセルシェーダーを通したデータ（レンダーターゲットに書き込むデータ）
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
};

//定数バッファ（カメラ情報）
cbuffer cbuff0 : register(b0)
{
    Camera cam;
};

//定数バッファ（好きなの入れてね）
cbuffer cbuff1 : register(b1)
{
    float3 cameraPos;
    uint pad;
}

//テクスチャ
Texture2D<float4> tex_0 : register(t0);
Texture2D<float4> tex_1 : register(t1);
Texture2D<float4> tex_2 : register(t2);
Texture2D<float4> tex_3 : register(t3);
Texture2D<float4> tex_4 : register(t4);

//サンプラー
SamplerState smp : register(s0);
    
VSOutput VSmain(VSOutput input)
{
    return input;
}

//ベクトルにスカラー値をかける。
float4 Scale(float4 In, float Scalar){
    return float4(In.x * Scalar,In.y * Scalar,In.z * Scalar,In.w * Scalar);
}
float4 Scale(float3 In, float Scalar){
    return float4(In.x * Scalar,In.y * Scalar,In.z * Scalar,1 * Scalar);
}

[maxvertexcount(6)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;

    //ビルボードのサイズ
    const float2 PolygonSize = float2(0.5f,2.0f);
    
    //ビュープロジェクション
    matrix viewproj = mul(cam.proj, cam.view);

    //デフォルトだと少し浮いてしまっているので1だけ沈める。
    input[0].position.xyz -= Scale(input[0].normal, 1);

    //カメラ方向ベクトル
    float3 cameraVec = normalize(cameraPos - input[0].position);

    //右方向ベクトル
    float3 rightVec = -normalize(cross(input[0].normal, cameraVec));

    //左下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //左上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //右下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);

    output.RestartStrip();
    
    //左上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //右上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //右下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);

    output.RestartStrip();
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    output.color = float4(1,0,0,1);
    output.emissive = float4(1,0,0,1);
    output.depth = float4(1,0,0,1);
    output.edgeColor = float4(1,0,0,1);
 
    return output;
}