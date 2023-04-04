#include "Gate.h"

Gate::Gate(const KuroEngine::Transform &transform, int stage_num) :m_transform(transform), m_stageNum(stage_num)
{
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
}

void Gate::Update()
{
}

void Gate::DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform);
}
