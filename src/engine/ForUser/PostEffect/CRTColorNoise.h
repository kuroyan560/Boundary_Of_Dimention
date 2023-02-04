#pragma once
#include"Common/Color.h"
#include"DirectX12/D3D12Data.h"
#include<map>
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class SpriteMesh;

	class CRTColorNoise : public Debugger
	{
		struct Info
		{
			//画面サイズ
			Vec2<float>m_screenSize;
			//画面歪み強さ
			float m_screenDistort = 0.1f;
			//ガウシアンフィルタをかけてサンプリングする際の重み
			float gaussianSampleWeight = 16.0f;
			//オーバーレイ色
			Color m_overlayColor;
			//オーバーレイの明るさしきい値
			float m_overlayThreshold = 0.5f;
			//彩度を底上げ
			float m_saturationRate = 1.0f;
			float m_pad[2];
		}m_crtInfo;

		std::shared_ptr<SpriteMesh>m_spriteMesh;
		std::shared_ptr<ConstantBuffer>m_crtInfoBuff;
		std::shared_ptr<RenderTarget>m_resultTex;

		void OnImguiItems()override;

	public:
		CRTColorNoise();

		//グラフィックスマネージャに登録
		void Register(const std::shared_ptr<TextureBuffer>& arg_srcTex);
		//結果のテクスチャ取得
		std::shared_ptr<RenderTarget>& GetResultTex() { return m_resultTex; }
		//ウィンドウサイズで結果を描画
		void DrawResult(const AlphaBlendMode& arg_alphaBlend);
	};
}