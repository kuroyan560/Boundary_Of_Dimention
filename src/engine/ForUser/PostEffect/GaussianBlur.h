#pragma once
#include<memory>
#include"DirectX12/D3D12Data.h"
#include<array>
#include"Render/RenderObject/SpriteMesh.h"

namespace KuroEngine
{
	class GaussianBlur
	{
	private:
		enum PROCESS { X_BLUR, Y_BLUR, FINAL, PROCESS_NUM };
		static const int s_weightNum = 8;
		static void GeneratePipeline(std::array<std::shared_ptr<GraphicsPipeline>, PROCESS_NUM - 1>& arg_destPipeline, const DXGI_FORMAT& arg_format);

	private:
		//重みテーブル
		float m_weights[s_weightNum];

		//スプライトメッシュ
		std::array<std::unique_ptr<SpriteMesh>, PROCESS_NUM - 1>m_spriteMeshes;

		//重みテーブルの定数バッファ
		std::shared_ptr<ConstantBuffer>m_weightConstBuff;

		//ブラーの結果
		std::array<std::shared_ptr<RenderTarget>, PROCESS_NUM>m_blurResult;

		//パイプライン
		std::array<std::shared_ptr<GraphicsPipeline>, PROCESS_NUM - 1>m_gPipeline;

	public:
		//ソース画像 & 結果描画先設定、ぼかし強さ
		GaussianBlur(const Vec2<int>& arg_size, const DXGI_FORMAT& arg_format, const float& arg_blurPower = 8.0f);
		//ボケ具合
		void SetBlurPower(const float& arg_blurPower);
		//グラフィックスマネージャに登録
		void Register(const std::shared_ptr<TextureBuffer>& arg_srcTex);

		//結果のテクスチャ取得
		std::shared_ptr<RenderTarget>& GetResultTex() { return m_blurResult[FINAL]; }

		//ウィンドウサイズで結果を描画
		void DrawResult(const AlphaBlendMode& arg_alphaBlend);
	};
}