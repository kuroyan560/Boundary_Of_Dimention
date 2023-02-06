#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//�g�����X�t�H�[��
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetAngle();

		if (ImGui::DragFloat3("Position", (float*)&pos, 0.5f))
		{
			m_transform.SetPos(pos);
		}

		//���삵�₷���悤�ɃI�C���[�p�ɕϊ�
		KuroEngine::Vec3<float>eular = { angle.x.GetDegree(),angle.y.GetDegree(),angle.z.GetDegree() };
		if (ImGui::DragFloat3("Eular", (float*)&eular, 0.5f))
		{
			m_transform.SetRotate(Angle::ConvertToRadian(eular.x), Angle::ConvertToRadian(eular.y), Angle::ConvertToRadian(eular.z));
		}
		ImGui::TreePop();
	}

	//�J����
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		auto pos = m_cam->GetPos();
		auto target = m_cam->GetTarget();

		ImGui::DragFloat3("Position", (float*)&pos, 0.5f);
		ImGui::DragFloat3("Target", (float*)&target, 0.5f);

		ImGui::TreePop();
	}
}

Player::Player()
	:KuroEngine::Debugger("Player", true)
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
	auto angle = m_transform.GetAngle();

	//���͂��ꂽ�ړ��ʂ��擾
	auto move = OperationConfig::Instance()->GetMove(0.1f);
	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove(1);

	//�ړ��ʂɉ�]��K�p
	move = Math::TransformVec3(move, m_transform.GetRotate());
	//�ړ��ʉ��Z
	pos += move;

	//�����ړ��p�x�ʉ��Z
	/*angle += scopeMove;
	angle.x.Normalize();
	angle.y.Normalize();
	angle.z.Normalize();*/

	//�g�����X�t�H�[���̕ω���K�p
	m_transform.SetPos(pos);
	//m_transform.SetRotate(angle.x, angle.y, angle.z);
	m_cam->SetPos(pos);
	auto front = m_transform.GetFront();
	m_cam->SetTarget(pos + m_transform.GetFront());
}

void Player::Draw()
{
}

void Player::Finalize()
{
}
