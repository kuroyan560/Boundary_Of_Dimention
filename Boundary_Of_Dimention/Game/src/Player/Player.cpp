#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"

Player::Player()
{
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
}

void Player::Update()
{
	using namespace KuroEngine;

	auto pos = m_transform.GetPos();

	//入力された移動量を取得
	auto move = OperationConfig::Instance()->GetMove(0.1f);

	//移動量に回転を適用
	move = Math::TransformVec3(move, m_transform.GetRotate());
	//移動量加算
	pos += move;
	
	//トランスフォームの変化を適用
	m_transform.SetPos(pos);
	m_cam->SetPos(pos);
	m_cam->SetTarget(pos + m_transform.GetFront());
}

void Player::Draw()
{
}

void Player::Finalize()
{
}
