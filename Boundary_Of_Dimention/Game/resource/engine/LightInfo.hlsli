//ディレクションライト
struct DirectionLight
{
    float4 color;
    float3 direction;
    uint active;
};

//ポイントライト
struct PointLight
{
    float4 color;
    float3 pos;
    float influenceRange;
    uint active;
};

//スポットライト
struct SpotLight
{
    float4 color;
    float3 pos;
    float influenceRange;
    float3 target;
    float angle;
    uint active;
};

//天球
struct HemiSphereLight
{
    float4 skyColor;
    float4 groundColor;
    float3 groundNormal;
    uint active;
};

//アクティブ中のライトの数
struct LightInfo
{
    int dirLigNum;
    int ptLigNum;
    int spotLigNum;
    int hemiSphereNum;
};

float3 CalcLambertDiffuse(float3 DirDirection, float3 DirColor, float3 Normal)
{
    float t = saturate(dot(normalize(Normal), -DirDirection));
	//拡散反射光
    return DirColor * t;
}

//正規化Lambert
float3 CalcNormalizeLambertDiffuse(float3 DirDirection, float3 DirColor, float3 Normal)
{
    float t = dot(Normal, DirDirection);
    t *= -1.0f;
    if (t < 0.0f)
        t = 0.0f;
	
	//拡散反射光
    return DirColor * t / 3.1415926f;
}

float3 CalcPhongSpecular(float3 DirDirection, float3 DirColor, float3 WorldNormal, float3 WorldPos, float3 EyePos)
{
	//反射ベクトル
    //float3 refVec = reflect(DirDirection, WorldNormal);

	//鏡面反射の強さ
    float3 toEye = normalize(EyePos - WorldPos);
    //float t = dot(refVec, toEye);
    float t = saturate(dot(normalize(WorldNormal), normalize(-DirDirection + toEye)));
    
    //if (t < 0.0f)
    //{
        //t = 0.0f;
    //}

	//鏡面反射の強さを絞る
    //t = pow(t, 5.0f);
    t = pow(t, 5.0f);

	//鏡面反射光
    return DirColor * t;
}

float3 CalcLimLight(float3 Dir, float3 Color, float3 WorldNormal, float3 ViewNormal)
{
    float power1 = 1.0f - max(0.0f, dot(Dir, WorldNormal));
    float power2 = 1.0f - max(0.0f, ViewNormal.z * -1.0f);
    float limPower = power1 * power2;
    limPower = pow(limPower, 1.3f);
    return limPower * Color;
}

float3 CalcLigDirection(DirectionLight DirLight, float3 WorldNormal, float3 WorldPos, float3 ViewNormal, float3 EyePos)
{
    return CalcLambertDiffuse(DirLight.direction, DirLight.color.xyz, WorldNormal)
		+ CalcPhongSpecular(DirLight.direction, DirLight.color.xyz, WorldNormal, WorldPos, EyePos)
        + CalcLimLight(DirLight.direction, DirLight.color.xyz, WorldNormal, ViewNormal);
}

float3 CalcLigPoint(PointLight PtLight, float3 WorldPos, float3 WorldNormal, float3 ViewNormal, float3 EyePos)
{
    float3 ligDir = WorldPos - PtLight.pos;
    ligDir = normalize(ligDir);
		//減衰なし
    float3 diffPoint = CalcLambertDiffuse(ligDir, PtLight.color.xyz, WorldNormal);
    float3 specPoint = CalcPhongSpecular(ligDir, PtLight.color.xyz, WorldNormal, WorldPos, EyePos);
	//距離による減衰
    float3 distance = length(WorldPos - PtLight.pos);
		//影響率は距離に比例して小さくなっていく
    float affect = 1.0f - 1.0f / PtLight.influenceRange * distance;
		//影響力がマイナスにならないように補正をかける
    if (affect < 0.0f)
        affect = 0.0f;
		//影響を指数関数的にする
    affect = pow(affect, 3.0f);
    diffPoint *= affect;
    specPoint *= affect;
    
    return diffPoint + specPoint + CalcLimLight(ligDir, PtLight.color.xyz, WorldNormal, ViewNormal) * affect;
}

float3 CalcLigSpot(SpotLight SpotLight, float3 WorldPos, float3 WorldNormal, float3 ViewNormal, float3 EyePos)
{
    float3 ligDir = WorldPos - SpotLight.pos;
    ligDir = normalize(ligDir);
    float3 diffSpotLight = CalcLambertDiffuse(ligDir, SpotLight.color.xyz, WorldNormal);
    float3 specSpotLight = CalcPhongSpecular(ligDir, SpotLight.color.xyz, WorldNormal, WorldPos, EyePos);
        //スポットライトとの距離を計算
    float3 distance = length(WorldPos - SpotLight.pos);
       	//影響率は距離に比例して小さくなっていく
    float affect = 1.0f - 1.0f / SpotLight.influenceRange * distance;
        //影響力がマイナスにならないように補正をかける
    if (affect < 0.0f)
        affect = 0.0f;
    //影響を指数関数的にする
    affect = pow(affect, 3.0f);
    diffSpotLight *= affect;
    specSpotLight *= affect;
    
    float3 lim = CalcLimLight(ligDir, SpotLight.color.xyz, WorldNormal, ViewNormal) * affect;
        
    float3 dir = normalize(SpotLight.target - SpotLight.pos);
    float angle = dot(ligDir, dir);
    angle = abs(acos(angle));
    affect = 1.0f - 1.0f / SpotLight.angle * angle;
    if (affect < 0.0f)
        affect = 0.0f;
    affect = pow(affect, 0.5f);
    diffSpotLight *= affect;
    specSpotLight *= affect;
    lim *= affect;
    return diffSpotLight + specSpotLight + lim;
}