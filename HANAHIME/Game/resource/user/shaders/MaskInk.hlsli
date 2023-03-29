struct MaskInk
{
    float3 m_pos;
    float m_scale;
    float3 m_posOffset;
    int m_texIdx;
};

struct ConstData
{
    //マスクインクのスケール
    float m_initScale;
    //座標ズレ最大
    float m_posOffsetMax;
    //インクテクスチャ数
    int m_texMax;
};