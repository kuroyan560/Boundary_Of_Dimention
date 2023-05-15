#include "PlayerMoveParticle.h"
#include "FrameWork/Importer.h"
#include "../Graphics/BasicDraw.h"
#include "../../../../src/engine/KuroEngine.h"
#include "../../../../src/engine/Render/RenderObject/Camera.h"
#include "../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"

PlayerMoveParticle::PlayerMoveParticle()
{

}

void PlayerMoveParticle::Init()
{

	m_orb.Init();
	m_smoke.Init();

}

void PlayerMoveParticle::Update()
{

	m_orb.Update();
	m_smoke.Update();

}

void PlayerMoveParticle::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	
	m_orb.Draw(arg_cam, arg_ligMgr);
	m_smoke.Draw(arg_cam, arg_ligMgr);

}

void PlayerMoveParticle::GenerateOrb(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter, const KuroEngine::Vec3<float>& arg_vel)
{

	m_orb.Generate(arg_playerPos, arg_scatter, arg_vel);

}

void PlayerMoveParticle::GenerateSmoke(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter)
{

	m_smoke.Generate(arg_playerPos, arg_scatter);

}
