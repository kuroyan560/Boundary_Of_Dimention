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
		auto angle = m_transform.GetRotateAsEuler();

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
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

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
	auto rotate = m_transform.GetRotate();

	//���͂��ꂽ�ړ��ʂ��擾
	auto move = OperationConfig::Instance()->GetMove(0.1f);
	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove(m_camSensitivity);

	//�ړ��ʂ�Y����]��K�p
	auto ySpin = XMVectorSet(0.0f, rotate.m128_f32[1], 0.0f, rotate.m128_f32[3]);
	move = Math::TransformVec3(move, ySpin);
	//�ړ��ʉ��Z
	pos += move;

	//�����ړ��p�x�ʉ��Z
	auto yScopeSpin = XMQuaternionRotationAxis(XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f), scopeMove.y);
	auto xScopeSpin = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), -scopeMove.x);
	rotate = XMQuaternionMultiply(yScopeSpin, rotate);
	rotate = XMQuaternionMultiply(xScopeSpin, rotate);

	//�g�����X�t�H�[���̕ω���K�p
	m_transform.SetPos(pos);
	m_transform.SetRotate(rotate);
	m_cam->SetPos(pos);
	auto front = m_transform.GetFront();
	m_cam->SetTarget(pos + m_transform.GetFront() * 6.0f);
}

void Player::Draw()
{
}

void Player::Finalize()
{
}
