#pragma once
#include<memory>
#include"DirectX12/D3D12Data.h"
#include"ForUser/Debugger.h"
namespace KuroEngine
{
	class SpriteMesh;

	//ビネットポストエフェクト（周辺減光）
	class Vignette : public Debugger
	{
	public:
		struct Info
		{
			//周辺減光の色
			Color m_color = Color(0.0f, 0.0f, 0.0f, 1.0f);
			//周辺減光の中心（UV座標）
			Vec2<float>m_center = { 0.5f,0.5f };
			//ビネット量
			float m_intensity = 1.0f;
			//ビネットの滑らかさ
			float m_smoothness = 0.0f;
		};
	private:
		std::shared_ptr<SpriteMesh>m_spriteMesh;
		std::shared_ptr<ConstantBuffer>m_vignetteInfoBuff;
		std::shared_ptr<RenderTarget>m_resultTex;

		Info m_vignetteInfo;

		void OnImguiItems()override;

	public:
		Vignette();

		//グラフィックスマネージャに登録
		void Register(const std::shared_ptr<TextureBuffer>& arg_srcTex);
		//結果のテクスチャ取得
		std::shared_ptr<RenderTarget>& GetResultTex() { return m_resultTex; }
		//ウィンドウサイズで結果を描画
		void DrawResult(const AlphaBlendMode& arg_alphaBlend);

		//情報のゲッタ・セッタ
		const Info& GetInfo()const { return m_vignetteInfo; }
		void SetInfo(const Info& arg_info);
	};
}