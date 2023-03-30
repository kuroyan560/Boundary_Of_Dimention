#include"../../engine/Camera.hlsli"
#include"Grass.hlsli"

//頂点データ
struct VSOutput
{
    float3 position : POSITION;
    uint instanceID : INSTANCE_ID;
};

//ジオメトリシェーダーを通したデータ
struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 toUV : TOUV;             //現在の角度から求められるUV
    float2 fromUV : FROMUV;         //前回使用されていたUV 補間させるために使用
    float uvLerpAmount : UVLERP;    //UVの補間量
    uint texID : TexID;             //使用するテクスチャのID
    float depthInView : CAM_Z;  //カメラまでの距離（深度）
};

//ピクセルシェーダーを通したデータ（レンダーターゲットに書き込むデータ）
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 normal : SV_Target3;
    float4 edgeColor : SV_Target4;
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
Texture2D<float4> tex_0 : register(t0);
Texture2D<float4> tex_1 : register(t1);
Texture2D<float4> tex_2 : register(t2);
Texture2D<float4> tex_3 : register(t3);
Texture2D<float4> tex_4 : register(t4);

//サンプラー
SamplerState smp : register(s0);
    
VSOutput VSmain(float3 pos : POSITION, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    output.position = pos;
    output.instanceID = instanceID;
    return output;
}

[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output)
{
    PlantGrass grass = aliveGrassBuffer[input[0].instanceID];
    
    GSOutput element;
    element.texID = grass.m_texIdx;
    element.normal = grass.m_normal;
    float3 position = grass.m_pos;

    //ビルボードのサイズ
    const float2 PolygonSize = float2(0.75f, 3.0f) * float2(0.75f, 0.75f);
    
    //デフォルトだと少し浮いてしまっているので1だけ沈める。
    position -= grass.m_normal;

    //カメラ方向ベクトル
    float3 cameraVec = normalize(otherTransformData.m_camPos - position);

    //右方向ベクトル
    float3 rightVec = normalize(cross(cameraVec, grass.m_normal));

    //正面ベクトル
    float3 forwardVec = normalize(cross(rightVec, grass.m_normal));

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

    //補間先のUVを求める。
    float invStep = 1.0f / textureSizeU;
    float toUVOffset =  floor(angle01 * invStep) / invStep;

    //補間元のUVを求める。
    float fromUVOFfset = toUVOffset - textureSizeU;
    if(fromUVOFfset < 0){
        fromUVOFfset = 1.0f - textureSizeU;
    }

    //uvOffsetの小数点第二位から補間の割合を求める。 Imposterに含まれている画像の数が20枚なので、0.05間隔で割合を求める。
    element.uvLerpAmount = abs(angle01 - toUVOffset) / textureSizeU;

    //風の強さ 出現度合いを風の大きさにかけることで出現初期は揺れないようにする。
    float windPower = grass.m_sineLength * grass.m_appearY;
    float4 windPos = float4(float3(0,0,1) * (sin(commonInfo.m_sineWave) * windPower), 0.0f);

    //草の高さ 出現度合いを風の高さにかけることでだんだん生えるようにする。
    float grassHeight = grass.m_appearY * PolygonSize.y;
    
    //法線を先に求めたいのでワールド座標を事前に求める
    float4 worldPos[4];
    //左下
    worldPos[0] = float4(position, 1.0f); //頂点を初期化
    worldPos[0] += float4(rightVec, 0) * -PolygonSize.x; //左方向に移動させる。
    //左上
    worldPos[1] = float4(position, 1.0f); //頂点を初期化。
    worldPos[1] += float4(rightVec, 0) * -PolygonSize.x; //左へ移動させる。
    worldPos[1] += float4(grass.m_normal, 0) * grassHeight; //上へ移動させる。
    worldPos[1] += windPos; //草を揺らす。
    //右下
    worldPos[2] = float4(position, 1.0f); //頂点を初期化
    worldPos[2] += float4(rightVec, 0) * PolygonSize.x; //右へ移動させる。
    //右上
    worldPos[3] = float4(position, 1.0f); //頂点を初期化。
    worldPos[3] += float4(rightVec, 0) * PolygonSize.x; //右へ移動させる。
    worldPos[3] += float4(grass.m_normal, 0) * grassHeight; //上へ移動させる。
    worldPos[3] += windPos; //草を揺らす。
    
    //法線を求める
    float3 vecA = worldPos[1].xyz - worldPos[0].xyz;
    float3 vecB = worldPos[2].xyz - worldPos[0].xyz;
    element.normal.xyz = normalize(cross(vecA, vecB));
    
    /*-- 左下 --*/
    //座標を求める。
    element.position = mul(cam.view, worldPos[0]); //カメラ座標へ
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UVを求める。
    element.toUV = float2(toUVOffset,1);                        //補間先のUV
    element.fromUV = float2(fromUVOFfset,1);                    //補間元のUV
    output.Append(element);
    
    /*-- 左上 --*/
    //座標を求める。
    element.position = mul(cam.view, worldPos[1]); //カメラ座標へ
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UVを求める。
    element.toUV = float2(toUVOffset,(1.0f - grass.m_appearY));        //補間先のUV
    element.fromUV = float2(fromUVOFfset,(1.0f - grass.m_appearY));    //補間元のUV
    output.Append(element);
    
    /*-- 右下 --*/
    //座標を求める。
    element.position = mul(cam.view, worldPos[2]); //カメラ座標へ
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UVを求める。
    element.toUV = float2(toUVOffset + textureSizeU,1);         //補間先のUV
    element.fromUV = float2(fromUVOFfset + textureSizeU,1);     //補間元のUV
    output.Append(element);
    
    /*-- 右上 --*/
    //座標を求める。
    element.position = mul(cam.view, worldPos[3]); //カメラ座標へ
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UVを求める。
    element.toUV = float2(toUVOffset + textureSizeU,(1.0f - grass.m_appearY));        //補間先のUV
    element.fromUV = float2(fromUVOFfset + textureSizeU,(1.0f - grass.m_appearY));    //補間元のUV
    output.Append(element);
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

    }else if(input.texID == 2){
        
        toColor = tex_2.Sample(smp, input.toUV);
        fromColor = tex_2.Sample(smp, input.fromUV);

    }else if(input.texID == 3){
        
        toColor = tex_3.Sample(smp, input.toUV);
        fromColor = tex_3.Sample(smp, input.fromUV);

    }else{

        toColor = tex_2.Sample(smp, input.toUV);
        fromColor = tex_2.Sample(smp, input.fromUV);

    }

    //色を補間する。
    output.color = fromColor * (1.0f - input.uvLerpAmount) + toColor * input.uvLerpAmount;

    //アルファ値によってクリップ
    clip(output.color.a - 0.9f);

    output.emissive = float4(0,0,0,0);
    output.depth = input.depthInView;
    output.normal.xyz = input.normal;
    output.edgeColor = float4(0.13, 0.53, 0.40,1);
 
    return output;
}