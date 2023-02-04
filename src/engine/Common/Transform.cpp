#include "Transform.h"

std::list<KuroEngine::Transform*> KuroEngine::Transform::s_transformList;

void KuroEngine::Transform::CalculateMat()
{
	m_localMat = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) * m_rotate;
	m_localMat *= DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	m_worldMat = m_localMat;

	if (m_parent != nullptr)
	{
		m_worldMat *= m_parent->GetWorldMat();
	}
	m_dirty = false;
}

const KuroEngine::Matrix& KuroEngine::Transform::GetLocalMat()
{
	if (IsDirty())CalculateMat();
	return m_localMat;
}

const KuroEngine::Matrix& KuroEngine::Transform::GetWorldMat()
{
	if (IsDirty())CalculateMat();
	return m_worldMat;
}

KuroEngine::Matrix KuroEngine::Transform::GetWorldMat(const Matrix& arg_billBoardMat)
{
	Matrix result = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) * m_rotate;
	result *= arg_billBoardMat;
	result *= DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);

	if (m_parent != nullptr)
	{
		result *= (m_parent->GetWorldMat());
	}
	return result;
}