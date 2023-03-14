#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//トランスフォーム
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetRotateAsEuler();

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

		ImGui::DragFloat3("PositionOffset", (float*)&m_camPosOffset, 0.5f);
		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

Player::Player()
	:KuroEngine::Debugger("Player", true)
{
	//モデル読み込み
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");

	//カメラ生成
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");

	Debugger::ParameterLogInfo param;
	param.m_dataPtr = &m_camSensitivity;
	param.m_key = "sensitivity";
	param.m_type = LOG_TYPE::FLOAT;
	m_parameterLogArray.emplace_back(param);
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camPosOffset = m_camPosOffsetDefault;
}

void Player::Update()
{
	using namespace KuroEngine;

	auto pos = m_transform.GetPos();
	auto rotate = m_transform.GetRotate();

	//入力された移動量を取得
	auto move = OperationConfig::Instance()->GetMove(0.1f);
	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove(m_camSensitivity);

	//移動量にY軸回転を適用
	auto ySpin = XMVectorSet(0.0f, rotate.m128_f32[1], 0.0f, rotate.m128_f32[3]);
	move = Math::TransformVec3(move, ySpin);
	//移動量加算
	pos += move;

	//視線移動角度量加算（Y軸：左右）
	auto yScopeSpin = XMQuaternionRotationAxis(XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f), scopeMove.y);
	rotate = XMQuaternionMultiply(yScopeSpin, rotate);
	m_transform.SetRotate(rotate);

	//視線移動角度量加算（X軸：上下）
	auto xScopeSpin = XMQuaternionRotationAxis(m_transform.GetRight(), -scopeMove.x);
	rotate = XMQuaternionMultiply(xScopeSpin, rotate);

	//トランスフォームの変化を適用
	m_transform.SetPos(pos);
	m_transform.SetRotate(rotate);
	m_cam->SetPos(pos + m_camPosOffset);
	auto front = m_transform.GetFront();
	m_cam->SetTarget(pos + m_transform.GetFront() * 6.0f);
}

void Player::Draw(KuroEngine::Camera& arg_cam)
{
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
}

void Player::Finalize()
{
}
