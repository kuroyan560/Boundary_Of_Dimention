struct EdgeParameter
{
    //エッジ描画の判断をする深度差のしきい値
    float m_depthThreshold;
    //深度値を比べるテクセルへのUVオフセット
    float2 m_uvOffset[8];
};


struct VSOutput
{
    float4 m_pos : SV_POSITION;
    float2 m_uv : TEXCORRD;
};

#define FLT_EPSILON 0.001

Texture2D<float4> g_depthMap : register(t0);
Texture2D<float4> g_brightMap : register(t1);
Texture2D<float4> g_edgeColorMap : register(t2);
Texture2D<float4> g_normalMap : register(t3);
SamplerState g_sampler : register(s0);
cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat;
}

cbuffer cbuff0 : register(b1)
{
    EdgeParameter m_edgeParam;
}

VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //ピクセルシェーダに渡す値
    output.m_pos = mul(parallelProjMat, pos);
    pos.z = 0.0f;
    output.m_uv = uv;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    //このピクセルに光が当たっているか
    float myBright = g_brightMap.Sample(g_sampler, input.m_uv).x;
    
    // このピクセルの深度値を取得
    float depth = g_depthMap.Sample(g_sampler, input.m_uv).x;

    //一番近い（深度値が低い）ピクセルを記録
    float nearest = depth;
    float2 nearestUv = input.m_uv;

    //エッジの太さ
    float edgeThickness = 0.002f;
    float2 edgeOffsetUV[8];
    edgeOffsetUV[0] = float2(edgeThickness, 0.0f);
    edgeOffsetUV[1] = float2(-edgeThickness, 0.0f);
    edgeOffsetUV[2] = float2(0.0f, edgeThickness);
    edgeOffsetUV[3] = float2(0.0f, -edgeThickness);
    edgeOffsetUV[4] = float2(edgeThickness, edgeThickness);
    edgeOffsetUV[5] = float2(-edgeThickness, edgeThickness);
    edgeOffsetUV[6] = float2(edgeThickness, -edgeThickness);
    edgeOffsetUV[7] = float2(-edgeThickness, -edgeThickness);
    
    // 近傍8テクセルの深度値の差の平均値を計算する
    float depthDiffer = 0.0f;
    for( int i = 0; i < 8; i++)
    {

        //暗いところだったら
        if(myBright == 0){
        
            //明るさ取得
            float2 brihgtPickUv = input.m_uv + edgeOffsetUV[i];
            float pickBright = g_brightMap.Sample(g_sampler, brihgtPickUv).x;
            if(pickBright != myBright)
            {
                return g_edgeColorMap.Sample(g_sampler, brihgtPickUv);
            }

        }
        
        //深度取得
        float2 pickUv = input.m_uv + m_edgeParam.m_uvOffset[i];
        float pickDepth = g_depthMap.Sample(g_sampler, pickUv).x;
        depthDiffer += abs(depth - pickDepth);
        
        //深度値最小を更新
        if(pickDepth < nearest)
        {
            nearestUv = pickUv;
            nearest = pickDepth;
        }
    }
    depthDiffer /= 8.0f;
    
    //このピクセルの法線取得
    float3 normal = g_normalMap.Sample(g_sampler, input.m_uv).xyz;
    //一番近いピクセルの法線取得
    float3 nearestNormal = g_normalMap.Sample(g_sampler, nearestUv).xyz;
    //法線が一緒か
    bool sameNormal = (length(normal - nearestNormal) < FLT_EPSILON);
    
    // 自身の深度値と近傍8テクセルの深度値の差を調べる
    // 法線が異なる　かつ　深度値が結構違う場合はエッジ出力
    if (!sameNormal && m_edgeParam.m_depthThreshold <= depthDiffer)
    {
        //一番手前側のエッジカラーを採用する
        return g_edgeColorMap.Sample(g_sampler, nearestUv);
    }
    
    discard;
    return float4(0.0f,0.0f,0.0f,0.0f);
}