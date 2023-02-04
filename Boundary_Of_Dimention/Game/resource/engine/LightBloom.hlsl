cbuffer cbuff0 : register(b0)
{
    float3 outputColor;
    float brightThreshold;
    int2 imgSize;
}

Texture2D<float4> img_0 : register(t0);
Texture2D<float4> img_1 : register(t1);
Texture2D<float4> img_2 : register(t2);
Texture2D<float4> img_3 : register(t3);
RWTexture2D<float4> outputPixels : register(u0);

static const int DIV = 32;

[numthreads(DIV, DIV, 1)]
void Filter( uint2 DTid : SV_DispatchThreadID )
{
    float4 col = img_0[DTid];
    float bright = dot(col.xyz, float3(0.2125f, 0.7154f, 0.0721f)); //明るさ計算
    col.w *= step(brightThreshold, bright);
    
    //出力カラー割合乗算
    col.xyz *= outputColor;
    
    outputPixels[DTid] = col;
}

[numthreads(DIV, DIV, 1)]
void Combine(uint2 DTid : SV_DispatchThreadID)
{
    float4 combineCol = img_0[DTid];
    combineCol += img_1[DTid];
    combineCol += img_2[DTid];
    combineCol += img_3[DTid];
    combineCol.a = 1.0f;
    
    outputPixels[DTid] = combineCol;
}