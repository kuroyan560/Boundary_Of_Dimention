//植えた草の情報
struct PlantGrass
{
    float3 m_pos;
    int m_texIdx;
    float3 m_normal;
    float m_sineLength;
    float m_appearY;
    float m_appearYTimer;
    int m_isAlive;
};

//全草むらで共通する定数バッファ用のパラメータ
struct CommonGrassInfo
{
    //ビュー行列
    matrix matView;
    //プロジェクション行列
    matrix matProjection;
    //カメラ座標（ワールド座標）
    float3 eye;
    //判定を飛ばす距離
    float m_checkClipOffset;
    //プレイヤーの座標
    float3 m_playerPos;
	//周辺に既に草が生えているか確認する際の範囲
    float m_checkRange;
    //草むら登場時のイージング速度
    float m_appearEaseSpeed;
    //草むら死亡時のイージング速度
    float m_deadEaseSpeed;
    //草を揺らす際のSine量 つまり風
    float m_sineWave;
    //草を枯らす距離
    float m_deathDistance;
};

//草むら以外の外部オブジェクトのトランスフォーム情報
struct TransformData
{
    float3 m_camPos;
    float m_seed;
    int m_grassCount;
    int m_plantOnceCount;
};

//ランダム
float RandomIntInRange(float arg_seed)
{   
    return frac(sin(dot(float2(arg_seed, arg_seed + 1.0f), float2(12.9898f, 78.233f))) * 43758.5453f);
}