#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//トランスフォーム
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetAngle();

		if (ImGui::DragFloat3("Position", (float*)&pos, 0.5f))
		{
			m_transform.SetPos(pos);
		}

		//操作しやすいようにオイラー角に変換
		KuroEngine::Vec3<float>eular = { angle.x.GetDegree(),angle.y.GetDegree(),angle.z.GetDegree() };
		if (ImGui::DragFloat3("Eular", (float*)&eular, 0.5f))
		{
			m_transform.SetRotate(Angle::ConvertToRadian(eular.x), Angle::ConvertToRadian(eular.y), Angle::ConvertToRadian(eular.z));
		}
		ImGui::TreePop();
	}

	//カメラ
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

	//入力された移動量を取得
	auto move = OperationConfig::Instance()->GetMove(0.1f);
	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove(1);

	//移動量に回転を適用
	move = Math::TransformVec3(move, m_transform.GetRotate());
	//移動量加算
	pos += move;

	//視線移動角度量加算
	/*angle += scopeMove;
	angle.x.Normalize();
	angle.y.Normalize();
	angle.z.Normalize();*/

	//トランスフォームの変化を適用
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
