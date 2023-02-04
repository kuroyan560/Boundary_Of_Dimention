#pragma once
#include<memory>
#include<vector>
#include"Light.h"
#include<wrl.h>
#include<d3d12.h>
#include<array>
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class ConstantBuffer;
	class StructuredBuffer;
	class DescriptorData;

	class LightManager : public Debugger
	{
	private:
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

		static const std::array<int, Light::TYPE_NUM>MAX_LIG_NUM;
	private:
		std::vector<Light::Direction*>dirLights;
		std::vector<Light::Point*>ptLights;
		std::vector<Light::Spot*>spotLights;
		std::vector<Light::HemiSphere*>hemiSphereLights;

		//アクティブ中のライトの数を送るための定数バッファデータ
		struct LightNum
		{
			int dirLigNum = 0;
			int ptLigNum = 0;
			int spotLigNum = 0;
			int hemiSphereNum = 0;
		};
		std::shared_ptr<ConstantBuffer>ligNumConstBuff;

		//ライト情報用構造化バッファ
		std::shared_ptr<StructuredBuffer>ligStructuredBuff[Light::TYPE_NUM];

		//アクティブ中のライトの数をマッピング
		void MappingLigNum();
		// (引数で指定したタイプの) ライト情報をマッピング、Dirtyフラグを確認するか
		void MappingLigInfo(const Light::TYPE& Type, const bool& CheckDirty = true);

		void OnImguiItems()override;

	public:
		LightManager();

		void RegisterDirLight(Light::Direction* DirLight);
		void RegisterPointLight(Light::Point* PtLight);
		void RegisterSpotLight(Light::Spot* SpotLight);
		void RegisterHemiSphereLight(Light::HemiSphere* HemiSphereLight);

		std::shared_ptr<DescriptorData>GetLigNumInfo();
		std::shared_ptr<DescriptorData>GetLigInfo(const Light::TYPE& Type);
	};
}