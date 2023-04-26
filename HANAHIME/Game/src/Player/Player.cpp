#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/UsersInput.h"
#include"../SoundConfig.h"
#include"PlayerCollision.h"
#include"../TimeScaleMgr.h"

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

		ImGui::Text("OnGround : %d", m_onGround);

	}

	//移動
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Move")) {

		ImGui::DragFloat("MoveAccel", &m_moveAccel, 0.01f);
		ImGui::DragFloat("MaxSpeed", &m_maxSpeed, 0.01f);
		ImGui::DragFloat("Brake", &m_brake, 0.01f);

		ImGui::TreePop();
	}

	//カメラ
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true), m_growPlantPtLig(8.0f, &m_transform)
{
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
	LoadParameterLog();

	//モデル読み込み
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//カメラ生成
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//カメラのコントローラーにアタッチ
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

	m_collision.m_refPlayer = this;

}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_initTransform = arg_initTransform;
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraRotYStorage = 0;
	m_cameraRotMove = 0;
	m_cameraJumpLerpAmount = 0;
	m_cameraJumpLerpStorage = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
	m_canJumpDelayTimer = 0;
	m_deathTimer = 0;

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_gimmickVel = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_cameraMode = 1;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_canZip = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

	m_growPlantPtLig.Register();
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	//トランスフォームを保存。
	m_prevTransform = m_transform;

	//ステージを保存。
	m_stage = arg_nowStage;

	//位置情報関係
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//ジャンプができるかどうか。	一定時間地形に引っ掛かってたらジャンプできる。
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//カメラモードを切り替える。
	if (UsersInput::Instance()->KeyOffTrigger(DIK_RETURN) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::X)) {
		++m_cameraMode;
		if (static_cast<int>(CAMERA_MODE.size()) <= m_cameraMode) {
			m_cameraMode = 0;
		}

		//SEを鳴らす。
		SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, m_cameraMode);
	}

	//ジップライン
	m_canZip = UsersInput::Instance()->KeyOnTrigger(DIK_SPACE);

	//移動ステータスによって処理を変える。
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//プレイヤーの回転をカメラ基準にする。(移動方向の基準がカメラの角度なため)
		m_transform.SetRotate(m_cameraQ);

		//入力された移動量を取得
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。

		//カメラの回転を保存。
		m_cameraRotYStorage += scopeMove.x;

		//入力量が一定以下だったら0にする。
		const float DEADLINE = 0.8f;
		if (m_rowMoveVec.Length() <= DEADLINE) {
			m_rowMoveVec = {};
		}

		//天井にいたら
		if (m_transform.GetUp().y < -0.9f) {
			//Xの移動方向を反転
			m_rowMoveVec.x *= -1.0f;
		}

		//移動させる。
		Move(newPos);

		//入力がなかったら
		if (m_rowMoveVec.Length() <= 0) {

			//カメラの回転を保存。
			m_cameraRotY = m_cameraRotYStorage;
			m_cameraRotMove = m_cameraRotYStorage;

		}
		else {

			//移動した方向を保存。
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		}

		//当たり判定
		m_collision.CheckHit(beforePos, newPos, arg_nowStage);

		m_transform.SetPos(newPos);

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		//タイマーを更新。
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(Out, Sine, m_jumpTimer, 0.0f, 1.0f);

		//カメラの回転を補間する。
		m_cameraRotMove = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;

		//座標を補間する。
		newPos = CalculateBezierPoint(easeAmount, m_jumpStartPos, m_jumpEndPos, m_bezierCurveControlPos);

		//回転を補完する。
		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_jumpStartQ, m_jumpEndQ, easeAmount));

		//上限に達していたらジャンプを終える。
		if (1.0f <= m_jumpTimer) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraJumpLerpAmount = 0;

			//面移動SEを鳴らす。
			SoundConfig::Instance()->Play(SoundConfig::SE_SURFACE_JUMP);

		}
		m_transform.SetPos(newPos);

	}
	break;
	case PLAYER_MOVE_STATUS::ZIP:
	{

		//ジップラインの更新処理
		UpdateZipline();

	}
	break;
	default:
		break;
	}


	//座標変化適用
	m_ptLig.SetPos(newPos);

	//カメラ操作
	m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, CAMERA_MODE[m_cameraMode]);

	//ギミックの移動を打ち消す。
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();
}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		IndividualDrawParameter::GetDefault());

	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);
	*/

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			camTransform.GetPos().z,
			arg_cam);
	}
}

void Player::Finalize()
{
}

KuroEngine::Vec3<float> Player::CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint) {

	float oneMinusT = 1.0f - arg_time;
	float oneMinusTSquared = oneMinusT * oneMinusT;
	float tSquared = arg_time * arg_time;

	float x = oneMinusTSquared * arg_startPoint.x + 2 * oneMinusT * arg_time * arg_controlPoint.x + tSquared * arg_endPoint.x;
	float y = oneMinusTSquared * arg_startPoint.y + 2 * oneMinusT * arg_time * arg_controlPoint.y + tSquared * arg_endPoint.y;
	float z = oneMinusTSquared * arg_startPoint.z + 2 * oneMinusT * arg_time * arg_controlPoint.z + tSquared * arg_endPoint.z;

	return KuroEngine::Vec3<float>(x, y, z);

}

void Player::Move(KuroEngine::Vec3<float>& arg_newPos) {

	//落下中は入力を無効化。
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed = m_rowMoveVec * m_maxSpeed;

	//移動速度をクランプ。
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//入力された値が無かったら移動速度を減らす。
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = 0;

	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = 0;

	}

	//ローカル軸の移動方向をプレイヤーの回転に合わせて動かす。
	auto moveAmount = KuroEngine::Math::TransformVec3(m_moveSpeed, m_transform.GetRotate());

	//移動量加算
	arg_newPos += moveAmount * TimeScaleMgr::s_inGame.GetTimeScale();

	//ギミックの移動量も加算。
	arg_newPos += m_gimmickVel;

	//地面に張り付ける用の重力。
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

}

void Player::UpdateZipline() {

	switch (m_gimmickStatus)
	{
	case Player::GIMMICK_STATUS::APPEAR:
	{

		//ジップラインの中に入っていくタイマーを更新
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_START);

		//イージングの量を求める。
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_START);

		//移動量のイージング
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//移動させる。
		m_transform.SetPos(m_zipInOutPos + (m_refZipline.lock()->GetPoint(true) - m_zipInOutPos) * moveEaseRate);

		//スケールのイージング
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//小さくする。
		m_transform.SetScale(1.0f - scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_START <= m_ziplineMoveTimer) {

			//ジップラインを動かす。
			m_refZipline.lock()->CanMovePlayer();

			//NORMALにしてプレイヤーは何もしないようにする。
			m_gimmickStatus = GIMMICK_STATUS::NORMAL;

			m_ziplineMoveTimer = 0;

		}

	}
	break;
	case Player::GIMMICK_STATUS::NORMAL:
	{
		//m_zipInOutPos = m_transform.GetPosWorld();
	}
	break;
	case Player::GIMMICK_STATUS::EXIT:
	{

		//ジップラインの中に入っていくタイマーを更新
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_END);

		//イージングの量を求める。
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_END);

		//移動量のイージング
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//移動させる。
		m_transform.SetPos(m_refZipline.lock()->GetPoint(false) + (m_zipInOutPos - m_refZipline.lock()->GetPoint(false)) * moveEaseRate);

		//スケールのイージング
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//小さくする。
		m_transform.SetScale(scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_END <= m_ziplineMoveTimer) {

			//プレイヤーを元に戻す。
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

			m_ziplineMoveTimer = 0;

		}

	}
	break;
	default:
		break;
	}

}