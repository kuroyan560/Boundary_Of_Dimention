#pragma once
#include<memory>
#include"Common/Vec.h"
#include<array>
#include<string>
#include"DirectX12/D3D12Data.h"

namespace KuroEngine
{
	enum NOISE_INTERPOLATION
	{
		WAVELET, BLOCK, NOISE_INTERPOLATION_NUM
	};

	struct NoiseInitializer
	{
	private:
		static int ID;
	public:
		int m_id = ID++;
		NOISE_INTERPOLATION m_interpolation = WAVELET;
		Vec2<int>m_split = { 16,16 };
		int m_contrast = 1;
		int m_octave = 1;
		float m_frequency = 1.0f;
		float m_persistance = 0.5f;
	};

	class PerlinNoise
	{
	private:
		static const int SPLIT_MAX = 256;

		//ノイズの補間方法名ゲッタ
		static const std::string& GetInterpolationName(const NOISE_INTERPOLATION& Interpolation)
		{
			static std::array<std::string, NOISE_INTERPOLATION_NUM>NAME =
			{
				"Wavelet","Block"
			};
			return NAME[Interpolation];
		}

		//引数の整数をランダムなハッシュ値に返還
		static unsigned xorshift32(unsigned v)
		{
			v = v ^ (v << 13);
			v = v ^ (v << 17);
			v = v ^ (v << 15);
			return v;
		}

		static float Wavelet(float t)
		{
			return static_cast<float>(1 - (6 * abs(pow(t, 5)) - 15 * pow(t, 4) + 10 * abs(pow(t, 3))));
		}

		//勾配を設定したウェーブレット関数
		static float GradWaveLet(Vec2<float>arg_uv, Vec2<float> arg_grad)
		{
			float l = arg_uv.x * arg_grad.x + arg_uv.y * arg_grad.y;
			float c = Wavelet(arg_uv.x) * Wavelet(arg_uv.y);
			return c * l;
		}

		/// <summary>
		/// 勾配成分を取得
		/// </summary>
		/// <param name="x">境界整数座標X</param>
		/// <param name="y">境界整数座標Y</param>
		/// <param name="elem">勾配の軸番号（x軸->0,y軸->1など）</param>
		/// <returns></returns>
		static float GetGrad(int arg_x, int arg_y, int arg_elem, int arg_seed = 0);

	public:
		//テクスチャにパーリンノイズ描画
		static void DrawToTex(std::shared_ptr<TextureBuffer>DestTex, const NoiseInitializer& Config);
		//パーリンノイズテクスチャ生成
		static std::shared_ptr<TextureBuffer>GenerateTex(const std::string& Name, const Vec2<int>& Size, const NoiseInitializer& Config, const DXGI_FORMAT& Format = DXGI_FORMAT_R32_FLOAT);
		//ノイズイニシャライザーのimguiデバッグ
		static bool ImguiDebugItem(NoiseInitializer& arg_initializer);

		//パーリンノイズから乱数取得
		static float GetRand(float arg_x, float arg_y, int arg_seed = 0);
	};
}