#pragma once
#include"DirectX12/D3D12Data.h"

namespace KuroEngine
{
	class ScanLine
	{
		static const int THREAD_DIV = 16;

		//走査線を描く処理を施すコンピュートパイプライン
		static std::shared_ptr<ComputePipeline>s_pipeline;

		struct ConstData
		{
			//テクスチャの高さ
			int m_texHeight;
			//周波数
			int m_frequency = 10;
		}m_constData;
		//定数バッファ
		std::shared_ptr<ConstantBuffer>m_constBuff;

		//ポストエフェクトをかけた結果
		std::shared_ptr<TextureBuffer>m_resultTex;

	public:
		ScanLine(Vec2<int>arg_texSize, DXGI_FORMAT arg_texFormat, int arg_frequency = 10);
		void SetFrequency(int arg_frequency);

		//グラフィックスマネージャに登録
		void Register(const std::shared_ptr<TextureBuffer>& arg_srcTex);
		//結果のテクスチャ取得
		std::shared_ptr<TextureBuffer>& GetResultTex() { return m_resultTex; }
		//ウィンドウサイズで結果を描画
		void DrawResult(const AlphaBlendMode& arg_alphaBlend);
	};
}