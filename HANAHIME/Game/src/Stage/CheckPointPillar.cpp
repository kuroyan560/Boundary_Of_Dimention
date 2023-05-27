#include "CheckPointPillar.h"
#include "../../../../src/engine/FrameWork/Importer.h"
#include "../Stage/StageManager.h"
#include "../Graphics/BasicDraw.h"
#include "../TimeScaleMgr.h"

CheckPointPillar::CheckPointPillar()
{

	m_pillarModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/Stage/", "PillarLight.glb");
	m_isDraw = false;

}

void CheckPointPillar::Init()
{

	m_isDraw = false;

}

void CheckPointPillar::Update()
{

	//次のチェックポイントの情報を取得
	KuroEngine::Transform nextCheckPointTransform;
	if (StageManager::Instance()->GetNowMapPinTransform(&nextCheckPointTransform)) {

		//一旦そのまま保存。
		m_transform = nextCheckPointTransform;
		m_isDraw = true;

		//UVアニメーション

		for (auto& mesh : m_pillarModel->m_meshes)
		{
			for (auto& vertex : mesh.mesh->vertices)
			{
				vertex.uv.x += 0.01f * TimeScaleMgr::s_inGame.GetTimeScale();
				//if (1.0f < vertex.uv.y)vertex.uv.y -= 1.0f;
			}
			mesh.mesh->Mapping();
		}

	}
	else {

		//MapPinが表示されなかったら円柱も描画しない。
		m_isDraw = false;

	}

}

void CheckPointPillar::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	//描画する状態だったら描画
	if (m_isDraw) {

		auto param = IndividualDrawParameter::GetDefault();
		BasicDraw::Instance()->Draw_NoOutline(arg_cam, arg_ligMgr, m_pillarModel, m_transform, IndividualDrawParameter::GetDefault(), KuroEngine::AlphaBlendMode_Trans);

	}

}
