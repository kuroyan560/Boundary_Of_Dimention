#include "GameItem.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"

void ItemOnGame::ItemData::Update()
{
	if (m_enbaleToGetFlag)
	{
	}
}

void ItemOnGame::ItemData::Draw(KuroEngine::Camera &camera)
{
	if (m_enbaleToGetFlag)
	{
		KuroEngine::DrawFuncBillBoard::Graph(camera, m_transform.GetPos(), KuroEngine::Vec2<float>(m_transform.GetScale().x, m_transform.GetScale().y), m_itemTex);
	}
}
