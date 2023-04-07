#include "PerlinNoise.h"
#include<vector>
#include"KuroEngine.h"
#include"DirectX12/D3D12App.h"
#include<magic_enum.h>

int KuroEngine::NoiseInitializer::ID = 0;

float KuroEngine::PerlinNoise::GetGrad(int arg_x, int arg_y, int arg_elem, int arg_seed)
{
	static std::array<int, 256> HASH;
	static bool HASH_INIT = false;
	if (!HASH_INIT)
	{
		int idx = 0;
		for (auto& key : HASH)key = idx++;

		for (int i = static_cast<int>(HASH.size()) - 1; 0 < i; --i)
		{
			int r = static_cast<int>(floor(KuroEngine::GetRand(1.0f) * (i + 1)));
			auto tmp = HASH[i];
			HASH[i] = HASH[r];
			HASH[r] = tmp;
		}
		HASH_INIT = true;
	}

	int hashX = HASH[arg_x % 256];
	int hashXY = HASH[(hashX + arg_y) % 256];
	int hashGrad = HASH[(hashXY + arg_elem) % 256];

	unsigned r = xorshift32(arg_x + arg_seed);
	r = xorshift32(r + arg_y);
	r = xorshift32(r + arg_elem) % 10000;
	return (float)r / 5000 - 1.0f;

	//-1~1の範囲を返す
	return (hashGrad / 255.0f) * 2.0f - 1.0f;
}

void KuroEngine::PerlinNoise::DrawToTex(std::shared_ptr<TextureBuffer> DestTex, const NoiseInitializer& Config)
{
	//最大分割数
	assert(0 < Config.m_split.x && Config.m_split.x <= SPLIT_MAX && 0 < Config.m_split.y && Config.m_split.y <= SPLIT_MAX);

	//補間方法の値が適切か
	assert(0 <= Config.m_interpolation && Config.m_interpolation < NOISE_INTERPOLATION_NUM);

	//コンピュートパイプライン
	static std::shared_ptr<ComputePipeline>PIPELINE;

	//定数バッファ
	static std::shared_ptr<ConstantBuffer>CONST_BUFF;
	//定数バッファ用データ
	struct ConstData
	{
		Vec2<float>rectLength;
		Vec2<int> split;
		int interpolation;
		int contrast;
		int octaveNum;
		float frequency;
		float persistance;
		ConstData(const int& Interpolation, const Vec2<float>& RectLength, const Vec2<int>& Split, const int& Contrast, const int& Octaves, const float& Frequency, const float& Persistance)
			:interpolation(Interpolation), rectLength(RectLength), split(Split), contrast(Contrast), octaveNum(Octaves), frequency(Frequency), persistance(Persistance) {}
	};

	//構造体バッファ
	static std::shared_ptr<StructuredBuffer>STRUCTURED_BUFF;

	if (!PIPELINE)
	{
		//シェーダ
		auto cs = D3D12App::Instance()->CompileShader("resource/engine/PerlinNoise2D.hlsl", "CSmain", "cs_6_4");
		//ルートパラメータ
		std::vector<RootParam>rootParams =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"フラクタルパーリンノイズ生成情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"勾配ベクトル配列"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"フラクタルパーリンノイズテクスチャ"),
		};
		//パイプライン生成
		PIPELINE = D3D12App::Instance()->GenerateComputePipeline(cs, rootParams, { WrappedSampler(false, false) });
	}

	//定数バッファ生成
	if (!CONST_BUFF)
	{
		CONST_BUFF = D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1, nullptr, "PerlinNoise - ConstantBuffer ");
	}

	//構造体バッファ生成
	if (!STRUCTURED_BUFF)
	{
		STRUCTURED_BUFF =
			D3D12App::Instance()->GenerateStructuredBuffer(
				sizeof(Vec2<float>),
				static_cast<int>(pow(SPLIT_MAX + 1, 2)),
				nullptr,
				"PerlinNoise2D - StructuredBuffer");
	}

	//定数バッファにデータ転送
	ConstData constData(Config.m_interpolation, DestTex->GetGraphSize().Float() / Config.m_split.Float(), Config.m_split, Config.m_contrast, Config.m_octave, Config.m_frequency, Config.m_persistance);
	CONST_BUFF->Mapping(&constData);

	//分割後の各頂点の勾配ベクトル格納先
	Vec2<float>grad[(SPLIT_MAX + 1) * (SPLIT_MAX + 1)];
	for (int y = 0; y <= Config.m_split.y; ++y)
	{
		for (int x = 0; x <= Config.m_split.x; ++x)
		{
			int idx = y * (Config.m_split.x + 1) + x;
			//ランダムな勾配ベクトル
			grad[idx].x = KuroEngine::GetRand(1.0f) * GetRandPlusMinus();
			grad[idx].y = KuroEngine::GetRand(1.0f) * GetRandPlusMinus();

			if (x == Config.m_split.x)grad[idx] = grad[y * (Config.m_split.x + 1)];
			if (y == Config.m_split.y)grad[idx] = grad[x];
		}
	}
	//構造化バッファに転送
	STRUCTURED_BUFF->Mapping(grad);


	std::vector<RegisterDescriptorData>descData =
	{
		{CONST_BUFF,CBV},
		{STRUCTURED_BUFF,SRV},
		{DestTex,UAV},
	};

	static const int THREAD_NUM = 16;
	Vec3<int>threadNum =
	{
		static_cast<int>((DestTex->GetGraphSize().x / THREAD_NUM) + 1),
		static_cast<int>((DestTex->GetGraphSize().y / THREAD_NUM) + 1),
		1
	};

	D3D12App::Instance()->DispathOneShot(PIPELINE, threadNum, descData);
}

std::shared_ptr<KuroEngine::TextureBuffer> KuroEngine::PerlinNoise::GenerateTex(const std::string& Name, const Vec2<int>& Size, const NoiseInitializer& Config, const DXGI_FORMAT& Format)
{
	//描き込み先用テクスチャバッファ生成
	auto result = D3D12App::Instance()->GenerateTextureBuffer(Size, Format, Name.c_str());
	DrawToTex(result, Config);
	return result;
}

#include"FrameWork/ImguiApp.h"
bool KuroEngine::PerlinNoise::ImguiDebugItem(NoiseInitializer& arg_initializer)
{
	bool changed = false;

	//補間方法
	std::string preview = std::string(magic_enum::enum_name(arg_initializer.m_interpolation));
	if (ImGui::BeginCombo("NOISE_INTERPOLATION", preview.c_str()))
	{
		for (int i = 0; i < NOISE_INTERPOLATION_NUM; ++i)
		{
			bool isSelected = arg_initializer.m_interpolation == i;
			NOISE_INTERPOLATION nowInterpolation = (NOISE_INTERPOLATION)i;
			std::string current = std::string(magic_enum::enum_name(nowInterpolation));
			if (ImGui::Selectable(current.c_str(), isSelected))
			{
				arg_initializer.m_interpolation = nowInterpolation;
				changed = true;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	//分割数
	if (ImGui::DragInt("SplitX", &arg_initializer.m_split.x, 1, 1,SPLIT_MAX))changed = true;
	if (ImGui::DragInt("SplitY", &arg_initializer.m_split.y, 1, 1,SPLIT_MAX))changed = true;

	if(ImGui::DragInt("Contrast", &arg_initializer.m_contrast))changed = true;
	if(ImGui::DragInt("Octave", &arg_initializer.m_octave))changed = true;
	if(ImGui::DragFloat("Frequency", &arg_initializer.m_frequency))changed = true;
	if(ImGui::DragFloat("Persistance", &arg_initializer.m_persistance))changed = true;

	return changed;
}

float KuroEngine::PerlinNoise::GetRand(float arg_x, float arg_y, int arg_seed)
{
	int floorX = static_cast<int>(floor(arg_x));
	int ceilX = static_cast<int>(ceil(arg_x));
	if (floorX == ceilX)ceilX++;
	int floorY = static_cast<int>(floor(arg_y));
	int ceilY = static_cast<int>(ceil(arg_y));
	if (floorY == ceilY)ceilY++;

	//境界整数座標
	Vec2<int>upL = { floorX,floorY };
	Vec2<int>upR = { ceilX,floorY };
	Vec2<int>bottomL = { floorX,ceilY };
	Vec2<int>bottomR = { ceilX,ceilY };

	//勾配ベクトル取得
	Vec2<float>gradUpL = { GetGrad(upL.x,upL.y,0,arg_seed),GetGrad(upL.x,upL.y,1,arg_seed) };
	Vec2<float>gradUpR = { GetGrad(upR.x,upR.y,0,arg_seed),GetGrad(upR.x,upR.y,1,arg_seed) };
	Vec2<float>gradBottomL = { GetGrad(bottomL.x,bottomL.y,0,arg_seed),GetGrad(bottomL.x,bottomL.y,1,arg_seed) };
	Vec2<float>gradBottomR = { GetGrad(bottomR.x,bottomR.y,0,arg_seed),GetGrad(bottomR.x,bottomR.y,1,arg_seed) };

	Vec2<float>uv = { arg_x - static_cast<int>(arg_x) ,arg_y - static_cast<int>(arg_y) };

	//左上と右上の対で補間
	float wUpL = GradWaveLet(uv, gradUpL);
	float wUpR = GradWaveLet(Vec2<float>(-1.0f + uv.x, uv.y), gradUpR);
	float wUp = KuroEngine::Math::Lerp(wUpL, wUpR, uv.x);

	//左下と右下の対で補間
	float wBottomL = GradWaveLet(Vec2<float>(uv.x, -1.0f + uv.y), gradBottomL);
	float wBottomR = GradWaveLet(Vec2<float>(-1.0f + uv.x, -1.0f + uv.y), gradBottomR);
	float wBottom = KuroEngine::Math::Lerp(wBottomL, wBottomR, uv.x);

	//Y軸方向に補間
	return KuroEngine::Math::Lerp(wUp, wBottom, uv.y);
}