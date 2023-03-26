#pragma once
#include<memory>
#include"Common/PerlinNoise.h"
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class TextureBuffer;
	class ComputePipeline;
	class Camera;
	class DepthStencil;
}

//水彩画風に２つのテクスチャをブレンド
class WaterPaintBlend : public KuroEngine::Debugger
{
	static const int THREAD_PER_NUM = 32;
	
	//インクテクスチャ枚数
	static const int INK_TEX_NUM = 3;
	//インクテクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INK_TEX_NUM>m_inkTexArray;

	//マスクレイヤーにこぼすインク
	struct MaskInk
	{
		KuroEngine::Vec3<float>m_pos;
		int m_texIdx;
	};
	std::vector<MaskInk>m_maskInkArray;

	//パイプライン
	static std::shared_ptr<KuroEngine::ComputePipeline>s_pipeline;
	void GeneratePipeline();

	//パーリンノイズの設定
	KuroEngine::NoiseInitializer m_noiseInitializer;
	//境界ぼかしに使うノイズテクスチャ
	std::shared_ptr<KuroEngine::TextureBuffer>m_noiseTex;
	//結果の描画先
	std::shared_ptr<KuroEngine::TextureBuffer>m_resultTex;

	//マスクレイヤーレンダーターゲット
	std::shared_ptr<KuroEngine::RenderTarget>m_maskLayer;

	bool m_isDirty = false;

	void OnImguiItems()override;

public:
	WaterPaintBlend();

	//初期化処理
	void Init();

	//マスクレイヤーにインク垂らす
	void DropMaskInk(KuroEngine::Vec3<float>arg_pos);

	//グラフィックスマネージャに登録
	void Register(std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex, KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_depthStencil);

	//結果のテクスチャ取得
	std::shared_ptr<KuroEngine::TextureBuffer>& GetResultTex() { return m_resultTex; }
};