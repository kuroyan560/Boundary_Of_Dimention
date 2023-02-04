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

	//���͂��ꂽ�ړ��ʂ��擾
	auto move = OperationConfig::Instance()->GetMove(0.1f);

	//�ړ��ʂɉ�]��K�p
	move = Math::TransformVec3(move, m_transform.GetRotate());
	//�ړ��ʉ��Z
	pos += move;
	
	//�g�����X�t�H�[���̕ω���K�p
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
