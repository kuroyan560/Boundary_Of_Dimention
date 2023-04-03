//植えた草の情報
struct PlantGrass
{
    float3 m_pos;
    int m_texIdx;
    float3 m_normal;
    float m_sineLength;
    float m_appearY;
    float m_appearYTimer;
};

//全草むらで共通する定数バッファ用のパラメータ
struct CommonGrassInfo
{
    //判定を飛ばす距離
    float m_checkClipOffset;
	//周辺に既に草が生えているか確認する際の範囲
    float m_checkRange;
    //草むら登場時のイージング速度
    float m_appearEaseSpeed;
    //草を揺らす際のSine量 つまり風
    float m_sineWave;
};

//草むら以外の外部オブジェクトのトランスフォーム情報
struct TransformData
{
    float3 m_camPos;
    float3 m_checkPlantPos;
};
