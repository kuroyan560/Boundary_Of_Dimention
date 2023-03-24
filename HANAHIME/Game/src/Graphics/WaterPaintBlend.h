#pragma once
#include<memory>
#include"Common/PerlinNoise.h"

namespace KuroEngine
{
	class TextureBuffer;
	class RenderTarget;
}

//水彩画風に２つのテクスチャをブレンド
class WaterPaintBlend
{
	//パーリンノイズの設定
	KuroEngine::NoiseInitializer m_noiseInitializer;
	//境界ぼかしに使うノイズテクスチャ
	std::shared_ptr<KuroEngine::TextureBuffer>m_noiseTex;
	//結果の描画先
	std::shared_ptr<KuroEngine::RenderTarget>m_resultTex;

public:
	WaterPaintBlend();

	//グラフィックスマネージャに登録
	void Register(
		std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex,
		std::shared_ptr<KuroEngine::TextureBuffer>arg_blendTex,
		std::shared_ptr<KuroEngine::TextureBuffer>arg_maskTex);

	//結果のテクスチャ取得
};

