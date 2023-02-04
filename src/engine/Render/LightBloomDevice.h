#pragma once
#include"Common/Vec.h"
#include<memory>
#include<vector>
#include<array>

namespace KuroEngine
{
	class ComputePipeline;
	class RenderTarget;
	class TextureBuffer;
	class GaussianBlur;
	class ConstantBuffer;
	class ModelObject;

	struct LightBloomConfig
	{
		//出力するカラーに乗算
		Vec3<float>m_outputColorRate = { 1,1,1 };
		//明るさのしきい値
		float m_brightThreshold = 0.0f;
		//合成後の画像サイズ
		Vec2<int>m_imgSize;
	};

	class LightBloomDevice
	{
	private:
		static const int THREAD_DIV = 32;
		static std::shared_ptr<ComputePipeline>s_filterPipeline;
		static std::shared_ptr<ComputePipeline>s_combinePipeline;
		void GeneratePipeline();

	private:
		//設定
		LightBloomConfig m_config;

		//エミッシブマップ
		std::shared_ptr<TextureBuffer>m_processedEmissiveMap;
		//設定情報送信用
		std::shared_ptr<ConstantBuffer>m_constBuff;

		static const int BLUR_COUNT = 4;
		//ガウシアンブラー
		std::array<std::shared_ptr<GaussianBlur>, BLUR_COUNT>m_gaussianBlurs;

	public:
		LightBloomDevice();

		//レンダーターゲットに加算して描画
		void Draw(std::weak_ptr<RenderTarget>EmissiveMap, std::weak_ptr<RenderTarget>Target);


		//セッタ
		void SetOutputColorRate(const Vec3<float>& RGBrate = { 1,1,1 });
		void SetBrightThreshold(const float& Threshold = 0.0f);
		void SetBlurPower(const float& BlurPower = 8.0f);
	};
}