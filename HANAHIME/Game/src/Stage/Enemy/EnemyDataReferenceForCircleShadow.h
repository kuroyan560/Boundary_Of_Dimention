#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"Common/Transform.h"
#include<vector>
#include<memory>
#include"DirectX12/D3D12Data.h"
#include"DirectX12/D3D12App.h"

class EnemyDataReferenceForCircleShadow : public KuroEngine::DesignPattern::Singleton<EnemyDataReferenceForCircleShadow>
{

public:

	//敵データ参照用構造体
	struct RefData {

		const KuroEngine::Transform* m_refTransform;
		const float* m_refShadowRadius;
		const bool* m_refIsDead;

	};

	//GPUに転送するデータ
	struct GPUData {

		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Vec3<float> m_up;
		float m_shadowRadius;

	};

	//最大参照保存数。
	const int MAX_REFCOUNT = 512;

private:
	
	std::vector<RefData> m_refData;		//保存されている敵データ
	std::vector<GPUData> m_gpuData;		//GPUに転送するデータ
	std::shared_ptr<KuroEngine::StructuredBuffer> m_gpuResourceBuffer;		//GPUに転送するデータを入れる構造化バッファ
	std::shared_ptr<KuroEngine::ConstantBuffer> m_gpuResourceCountBuffer;	//GPUに転送するデータのサイズを入れる定数バッファ
	int m_gpuResourceCount;

public:

	//構造化バッファを用意
	void Setting();

	//初期化 参照を全てリセット
	void Init();

	//参照をセット。
	void SetData(const KuroEngine::Transform* arg_refTransform, const float* arg_refShadowRadius, const bool* arg_refIsDead);

	//GPUに転送するデータを更新する。
	void UpdateGPUData();

	//GPUに転送するデータのサイズを取得。
	std::shared_ptr<KuroEngine::ConstantBuffer> GetGPUResourceCountBuffer() { return m_gpuResourceCountBuffer; }

	//GPUに転送するデータを取得。
	std::shared_ptr<KuroEngine::StructuredBuffer> GetGPUResourceBuffer() { return m_gpuResourceBuffer; }


};