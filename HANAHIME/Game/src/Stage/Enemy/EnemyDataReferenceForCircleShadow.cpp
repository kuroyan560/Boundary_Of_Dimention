#include "EnemyDataReferenceForCircleShadow.h"

void EnemyDataReferenceForCircleShadow::Setting()
{

	m_gpuResourceBuffer = KuroEngine::D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(EnemyDataReferenceForCircleShadow::GPUData),
		MAX_REFCOUNT,
		nullptr,
		"BasicDraw - EnemyCircleShadow");


	m_gpuResourceCountBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(int),
		1,
		nullptr,
		"BasicDraw - EnemyCircleShadowNum");

}

void EnemyDataReferenceForCircleShadow::Init()
{
	m_refData.clear();
	m_gpuData.clear();
}

void EnemyDataReferenceForCircleShadow::SetData(const KuroEngine::Transform* arg_refTransform, const float* arg_refShadowRadius, const bool* arg_refIsDead)
{

	RefData data;
	data.m_refIsDead = arg_refIsDead;
	data.m_refTransform = arg_refTransform;
	data.m_refShadowRadius = arg_refShadowRadius;
	m_refData.emplace_back(data);

}

void EnemyDataReferenceForCircleShadow::UpdateGPUData()
{

	//バッファが空だったら生成
	if (!m_gpuResourceBuffer) {
		Setting();
	}

	m_gpuData.clear();

	for (auto& index : m_refData) {

		if (*(index.m_refIsDead)) continue;

		GPUData data;
		data.m_pos = index.m_refTransform->GetPos();
		data.m_up = index.m_refTransform->GetUp();
		data.m_shadowRadius = *(index.m_refShadowRadius);

		m_gpuData.emplace_back(data);

	}

	//サイズが0だったら。
	if (static_cast<int>(m_gpuData.size()) <= 0) {

		GPUData data;
		data.m_pos = {};
		data.m_up = {0,1,0};
		data.m_shadowRadius = 1.0f;

		m_gpuData.emplace_back(data);

	}

	m_gpuResourceCount = static_cast<int>(m_gpuData.size());

	if (0 < m_gpuResourceCount) {

		//データを転送
		m_gpuResourceBuffer->Mapping(m_gpuData.data(), static_cast<int>(m_gpuData.size()));
		m_gpuResourceCountBuffer->Mapping(&m_gpuResourceCount, sizeof(int));

	}

}
