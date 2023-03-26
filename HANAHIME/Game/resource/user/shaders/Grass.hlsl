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
    float2 toUV : TOUV;         //現在の角度から求められるUV
    float2 fromUV : FROMUV;         //前回使用されていたUV 補間させるために使用
    float uvLerpAmount : UVLERP;    //UVの補間量
    uint texID : NEXTTexID;
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

float3 ProjectOnPlane(float3 Vector, float3 PlaneNormal)
{
    //平面の法線を正規化する
    PlaneNormal = normalize(PlaneNormal);

    //射影平面の方程式を作成する
    float d = dot(Vector, PlaneNormal);
    float3 projectionPlane = Vector - d * PlaneNormal;

    //射影されたベクトルを計算する
    float3 projectedVector = Vector - projectionPlane;

    return projectedVector;
}
float Angle(float3 From, float3 To)
{
    //ベクトルの大きさを計算する
    float fromMagnitude = length(From);
    float toMagnitude = length(To);

    //ベクトルの内積を計算する
    float dotProduct = dot(From, To);

    //ベクトルのなす角度のcosineを計算する
    float cosine = dotProduct / (fromMagnitude * toMagnitude);

    //ベクトルのなす角度を計算する
    float angle = acos(cosine);

    //結果を度数法に変換する
    angle = degrees(angle);

    return angle;
}


[maxvertexcount(6)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
    element.texID = input[0].texID;

    //ビルボードのサイズ
    const float2 PolygonSize = float2(0.5f,2.0f);
    
    //ビュープロジェクション
    matrix viewproj = mul(cam.proj, cam.view);

    //デフォルトだと少し浮いてしまっているので1だけ沈める。
    input[0].position.xyz -= Scale(input[0].normal, 1);

    //カメラ方向ベクトル
    float3 cameraVec = normalize(cameraPos - input[0].position);

    //右方向ベクトル
    float3 rightVec = normalize(cross(cameraVec, input[0].normal));

    //正面ベクトル
    float3 forwardVec = normalize(cross(rightVec, input[0].normal));

    //デフォルトの正面ベクトルと現在の正面ベクトルの角度を求める。
    float3 defForwardVec = float3(0,0,1);
    float cosTheta = dot(defForwardVec, forwardVec) / (length(defForwardVec) * length(forwardVec));
    float angle = acos(cosTheta) * (180 / 3.14159265);
    //2つのベクトルの位置関係によって、角度を0~360度の範囲に修正する
    if (cross(defForwardVec, forwardVec).y < 0) {
        angle = 360 - angle;
    }
    float angle01 = angle / 360.0f; //0~1の角度

    //1枚あたりのU軸サイズ
    float textureSizeU = 1.0f / 20.0f;

    //uvoffsetをtextureSizeU区切りにする。
    float invStep = 1.0f / textureSizeU;
    float uvOffset =  floor(angle01 * invStep) / invStep;

    //補間元のUVを求める。
    float fromUVOFfset = uvOffset - textureSizeU;
    if(fromUVOFfset < 0){
        fromUVOFfset = 1.0f - textureSizeU;
    }

    //uvOffsetの小数点第二位から補間の割合を求める。 Imposterに含まれている画像の数が20枚なので、0.05間隔で割合を求める。
    element.uvLerpAmount = abs(angle01 - uvOffset) / textureSizeU;

    //左下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset,1);
    element.fromUV = float2(fromUVOFfset,1);
    output.Append(element);
    
    //左上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset,0);
    element.fromUV = float2(fromUVOFfset,0);
    output.Append(element);
    
    //右下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset + textureSizeU,1);
    element.fromUV = float2(fromUVOFfset + textureSizeU,1);
    output.Append(element);

    output.RestartStrip();
    
    //左上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset,0);
    element.fromUV = float2(fromUVOFfset,0);
    output.Append(element);
    
    //右上
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset + textureSizeU,0);
    element.fromUV = float2(fromUVOFfset + textureSizeU,0);
    output.Append(element);
    
    //右下
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    element.toUV = float2(uvOffset + textureSizeU,1);
    element.fromUV = float2(fromUVOFfset + textureSizeU,1);
    output.Append(element);

    output.RestartStrip();
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    //補間元と補間先の色を取得。
    float4 fromColor;
    float4 toColor;

    if(input.texID == 0){

        toColor = tex_0.Sample(smp, input.toUV);
        fromColor = tex_0.Sample(smp, input.fromUV);

    }else if(input.texID == 1){
        
        toColor = tex_1.Sample(smp, input.toUV);
        fromColor = tex_1.Sample(smp, input.fromUV);

    }else{

        toColor = tex_2.Sample(smp, input.toUV);
        fromColor = tex_2.Sample(smp, input.fromUV);

    }

    //色を補間する。
    output.color = fromColor * (1.0f - input.uvLerpAmount) + toColor * input.uvLerpAmount;

    //アルファ値によってクリップ
    clip(output.color.a - 0.9f);

    output.emissive = float4(0,0,0,0);
    output.depth = float4(0,0,0,0);
    output.edgeColor = float4(0,0,0,0);
 
    return output;
}