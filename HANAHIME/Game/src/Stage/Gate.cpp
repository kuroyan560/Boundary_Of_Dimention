#include "Gate.h"

Gate::Gate(const KuroEngine::Transform &transform, int stage_num) :m_transform(transform), m_stageNum(stage_num)
{
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
}

void Gate::Update()
{
}

bool Gate::IsHit(const KuroEngine::Vec3<float> &player_pos)
{
	std::array<KuroEngine::Vec3<float>, 2> size = { m_transform.GetScale(),m_transform.GetScale()};
	KuroEngine::Vec3<float>distance = m_transform.GetPos() - player_pos;
	const int square1 = 0;
	const int square2 = 1;
	bool isHitFlag = fabs(distance.x) <= size[square1].x + size[square2].x && fabs(distance.y) <= size[square1].y + size[square2].y;
	return isHitFlag;
}

void Gate::DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform);
}
