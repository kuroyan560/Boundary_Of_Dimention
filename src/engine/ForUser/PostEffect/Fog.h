#pragma once
#include"Common/Color.h"
#include"Common/Vec.h"
#include"ForUser/Debugger.h"
#include"DirectX12/D3D12Data.h"
#include<memory>
namespace KuroEngine
{
	class ComputePipeline;
	class ConstantBuffer;
	class TextureBuffer;

	//フォグポストエフェクト
	class Fog : public Debugger
	{
		static const int THREAD_PER_NUM = 32;

		//フォグの設定
		struct Config
		{
			//強さ
			float m_intensity = 1.0f;
			//不透明度最大
			float m_opacityMax = 1.0f;
			//フォグがかかる最大深度
			float m_distMax = 20.0f;
			//フォグがかかる最小深度
			float m_distMin = 0.0f;
		}m_config;
		std::shared_ptr<ConstantBuffer>m_configBuffer;

		//色に関して
		struct ColorParameter
		{
			//テクスチャを使わない場合のフォグの色
			Color m_fogColorNear = Color(1.0f, 1.0f, 1.0f, 1.0f);
			Color m_fogColorFar = Color(1.0f, 1.0f, 1.0f, 1.0f);
			//テクスチャを使うか
			unsigned int m_useTex = 0;
			int m_isGameScene;
			KuroEngine::Vec2<float> m_pad;
		}m_colorConfig;
		std::shared_ptr<ConstantBuffer>m_colorConfigBuffer;

		//フォグをかけるパイプライン
		std::shared_ptr<ComputePipeline>m_cPipeline;

		//フォグの色を決めるテクスチャ（横長のグラデーション画像、深度値が小さいほど UV の U値 が小さくなる）
		std::shared_ptr<TextureBuffer>m_gradationTex;

		//結果の描画先
		std::shared_ptr<TextureBuffer>m_resultTex;

		void OnImguiItems()override;

	public:
		Fog(Vec2<int>arg_size, DXGI_FORMAT arg_mainFormat);

		//グラフィックスマネージャに登録
		void Register(const std::shared_ptr<TextureBuffer>& arg_main, const std::shared_ptr<TextureBuffer>& arg_depthMap, std::shared_ptr<TextureBuffer>arg_maskTex = nullptr, bool arg_isGameScene = false);

		//結果のテクスチャ取得
		std::shared_ptr<TextureBuffer>& GetResultTex() { return m_resultTex; }
		//ウィンドウサイズで結果を描画
		void DrawResult(const AlphaBlendMode& arg_alphaBlend);

		//フォグの色を決めるグラデーションテクスチャ（横長）をアタッチ
		void AttachGradationTex(std::shared_ptr<TextureBuffer>arg_tex);
	};
}