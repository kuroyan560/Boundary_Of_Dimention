#include "Transform2D.h"

std::list<KuroEngine::Transform2D*> KuroEngine::Transform2D::s_transform2DList;

void KuroEngine::Transform2D::CalculateMat()
{
	m_localMat = XMMatrixScaling(m_scale.x, m_scale.y, 1.0f) * m_rotate;
	m_localMat.r[3].m128_f32[0] = m_pos.x;
	m_localMat.r[3].m128_f32[1] = m_pos.y;

	m_worldMat = m_localMat;

	if (m_parent != nullptr)
	{
		m_worldMat *= m_parent->GetMatWorld();
	}
	m_dirty = false;
}

const KuroEngine::Matrix& KuroEngine::Transform2D::GetMatLocal()
{
	if (IsDirty())CalculateMat();
	return m_localMat;
}

const KuroEngine::Matrix& KuroEngine::Transform2D::GetMatWorld()
{
	if (IsDirty())CalculateMat();
	return m_worldMat;
}