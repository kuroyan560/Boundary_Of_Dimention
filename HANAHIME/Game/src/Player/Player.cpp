#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"

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

		//前ベクトル
		auto front = m_transform.GetFront();
		ImGui::Text("Front : %.2f ,%.2f , %.2f", front.x, front.y, front.z);

		//上ベクトル
		auto up = m_transform.GetUp();
		ImGui::Text("Up : %.2f ,%.2f , %.2f", up.x, up.y, up.z);
	}

	//カメラ
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		auto pos = m_cam->GetPos();
		auto target = m_cam->GetGazePointPos();

		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

bool Player::HitCheck(const KuroEngine::Vec3<float>arg_from, const KuroEngine::Vec3<float> arg_to, const std::vector<Terrian>& arg_terrianArray, KuroEngine::Vec3<float>* arg_terrianNormal)
{
	/*
	arg_from … 移動前の座標	
	arg_to … 移動後の座標
	arg_terrianArray … 地形の配列
	arg_terrianNormal … 当たった地形のメッシュの法線、格納先
	*/

	//当たり判定結果
	bool isHit = false;

	//地形配列走査
	for (auto& terrian : arg_terrianArray)
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//トランスフォーム情報
		auto& transform = terrian.m_transform;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//判定↓============================================

			//mesh->vertices
			


			//=================================================
		}
	}

	//当たり判定がtrueなら当たった地形の法線を格納
	if (isHit && arg_terrianNormal)
	{
		//*arg_terrianNormal = ;
	}
	return isHit;
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true)
{
	//モデル読み込み
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//カメラ生成
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");

	AddCustomParameter("MoveScalar", { "moveScalar" }, PARAM_TYPE::FLOAT, &m_moveScalar, "Player");
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;
	auto rotate = m_transform.GetRotate();

	//入力された移動量を取得
	auto moveVec = OperationConfig::Instance()->GetMoveVec(rotate);
	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//移動量加算
	newPos += moveVec * m_moveScalar;

	//視線移動角度量加算（Y軸：左右）
	auto yScopeSpin = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), scopeMove.x);
	rotate = XMQuaternionMultiply(yScopeSpin, rotate);

	//当たり判定
	Vec3<float>hitTerrianNormal;
	if (HitCheck(beforePos, newPos, arg_nowStage.lock()->GetTerrianArray(), &hitTerrianNormal))
	{
		//当たり判定に基づいて移動を修正
	}

	//トランスフォームの変化を適用
	m_transform.SetPos(newPos);
	m_transform.SetRotate(rotate);

	//カメラ操作
	auto front = m_transform.GetFront();
	m_camController.Update(m_cam, newPos, front);

}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform);

	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetWorldMat(),
			camTransform.GetPos().z,
			arg_cam);
	}
}

void Player::Finalize()
{
}
