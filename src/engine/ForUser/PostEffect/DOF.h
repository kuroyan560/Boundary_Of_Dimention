#pragma once
#include<memory>
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class RenderTarget;
	class ComputePipeline;
	class ConstantBuffer;
	class GaussianBlur;
	class TextureBuffer;

	//DOF設定情報
	struct DOFConfig
	{
		//手前
		float m_nearPint = 0.0f;
		//奥（手前～奥までボケない）
		float m_farPint = 10.0f;
		//手前以降、奥以降からボケ最大になるまでの距離
		float m_pintLength = 20.0f;
	};

	class DOF : public Debugger
	{
	private:
		static std::shared_ptr<ComputePipeline>s_pipeline;
		void GeneratePipeline();

	private:
		//DOF設定情報
		DOFConfig m_config;
		std::shared_ptr<ConstantBuffer>m_configBuff;

		//ガウシアンブラー
		std::shared_ptr<GaussianBlur>m_gaussianBlur;

		//深度マップをもとに生成した透過ボケ画像の格納先
		std::shared_ptr<TextureBuffer>m_processedTex;

		//ブラー強さ
		float m_blurPower = 8.0f;

		void OnImguiItems()override;

	public:
		DOF();
		void SetPintConfig(float NearPint = 0.0f, float FarPint = 10.0f, float PintLength = 20.0f);

		//グラフィックスマネージャに登録
		void Register(std::weak_ptr<TextureBuffer>Src, std::weak_ptr<TextureBuffer>DepthMap);

		//結果のテクスチャ取得
		std::shared_ptr<TextureBuffer>& GetResultTex() { return m_processedTex; }

		//結果の描画
		void Draw();

		//ボケ具合設定
		void SetBlurPower(float arg_blurPower);
	};
}