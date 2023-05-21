#include"Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/UsersInput.h"
#include"../SoundConfig.h"
#include"PlayerCollision.h"
#include"../TimeScaleMgr.h"
#include"DirectX12/D3D12App.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"FrameWork/WinApp.h"

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//トランスフォーム
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetRotateAsEuler();

		if (ImGui::DragFloat3("Position", (float *)&pos, 0.5f))
		{
			m_transform.SetPos(pos);
		}

		//操作しやすいようにオイラー角に変換
		KuroEngine::Vec3<float>eular = { angle.x.GetDegree(),angle.y.GetDegree(),angle.z.GetDegree() };
		if (ImGui::DragFloat3("Eular", (float *)&eular, 0.5f))
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
}

void Player::AnimationSpecification(const KuroEngine::Vec3<float> &arg_beforePos, const KuroEngine::Vec3<float> &arg_newPos)
{
	//移動ステータス
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE)
	{
		//ジャンプアニメーション中
		if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_JUMP]))return;

		//動きに変動があった
		if (0.01f < arg_beforePos.Distance(arg_newPos))
		{
			//既に歩きアニメーション再生中
			if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WALK]))return;

			//歩きアニメーション再生
			m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WALK], true, false);
		}
		//動きなし
		else
		{
			//既に待機アニメーションまたはキョロキョロアニメーション再生中
			if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WAIT]) || m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_INTEREST]))return;

			//キョロキョロするカウンターデクリメント
			m_animInterestCycleCounter--;

			//まだキョロキョロ周期が訪れてない
			if (0 <= m_animInterestCycleCounter)
			{
				//待機アニメーション再生
				m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WAIT], false, false);
			}
			//定期的にキョロキョロする
			else
			{
				//キョロキョロアニメーション再生
				m_modelAnimator->Play(m_animNames[ANIM_PATTERN_INTEREST], false, false);
				//キョロキョロ周期カウンターリセット
				m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
			}
		}
	}
	//ジャンプステータス
	else if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP)
	{
		//トリガー時でない
		if (m_playerMoveStatus == m_beforePlayerMoveStatus)return;

		//ジャンプアニメーション再生
		m_modelAnimator->Play(m_animNames[ANIM_PATTERN_JUMP], false, false);
	}

	//待機アニメーションでなければキョロキョロ周期カウンターが減ることはない
	if (!m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WAIT]))
	{
		//キョロキョロ周期カウンターリセット
		m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
	}
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true), m_growPlantPtLig(8.0f, &m_transform)
{
	using namespace KuroEngine;

	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
	AddCustomParameter("Default_AccelSpeed", { "move","default","accelSpeed" }, PARAM_TYPE::FLOAT, &m_defaultAccelSpeed, "Move");
	AddCustomParameter("Default_MaxSpeed", { "move","default","maxSpeed" }, PARAM_TYPE::FLOAT, &m_defaultMaxSpeed, "Move");
	AddCustomParameter("Default_Brake", { "move","default","brake" }, PARAM_TYPE::FLOAT, &m_defaultBrake, "Move");
	AddCustomParameter("UnderGround_AccelSpeed", { "move","underGround","accelSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundAccelSpeed, "Move");
	AddCustomParameter("UnderGround_MaxSpeed", { "move","underGround","maxSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundMaxSpeed, "Move");
	AddCustomParameter("UnderGround_Brake", { "move","underGround","brake" }, PARAM_TYPE::FLOAT, &m_underGroundBrake, "Move");
	LoadParameterLog();

	//モデル読み込み
	m_model = Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//カメラ生成
	m_cam = std::make_shared<Camera>("Player's Camera");
	//カメラのコントローラーにアタッチ
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

	m_collision.m_refPlayer = this;

	//死亡アニメーションを読み込み
	D3D12App::Instance()->GenerateTextureBuffer(m_deathAnimSprite.data(), "resource/user/tex/Number.png", DEATH_SPRITE_ANIM_COUNT, KuroEngine::Vec2<int>(DEATH_SPRITE_ANIM_COUNT, 1));

	//アニメーター生成
	m_modelAnimator = std::make_shared<ModelAnimator>(m_model);

	m_tex.resize(MiniBug::MAX);
	m_tex[MiniBug::FIND] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Find.png");
	m_tex[MiniBug::HIT] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Attack.png");
	m_tex[MiniBug::LOOK] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
	m_tex[MiniBug::FAR_AWAY] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
	m_tex[MiniBug::DEAD] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/dead.png");
	m_reaction = std::make_unique<MiniBug::Reaction>(m_tex);
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	//スケール打ち消し
	arg_initTransform.SetScale(1.0f);

	m_initTransform = arg_initTransform;
	m_prevTransform = arg_initTransform;
	m_transform = arg_initTransform;
	m_drawTransform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraRotYStorage = arg_initTransform.GetRotateAsEuler().x;
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
	m_onGround = false;
	m_cameraMode = 1;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_canZip = false;
	m_isCameraInvX = false;
	m_canUnderGroundRelease = true;
	m_canOldUnderGroundRelease = true;
	m_onCeiling = false;
	m_isCameraUpInverse = false;
	m_isHitUnderGroundCamera = false;
	m_isCameraDefault = false;
	m_isOldCameraDefault = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
	m_isWallFrontDir = false;

	m_growPlantPtLig.Register();
	//死亡演出のタイマーを初期化。
	m_deathEffectTimer = 0;
	m_deathShakeAmount = 0;
	m_damageShakeAmount = 0;
	m_shake = KuroEngine::Vec3<float>();
	m_deathStatus = DEATH_STATUS::APPROACH;
	m_isFinishDeathAnimation = false;

	//被ダメージの点滅初期化
	m_damageFlashTimer.Reset(6.0f);
	m_damageFlash = false;

	//地中に沈む関連
	m_isInputUnderGround = false;
	m_isUnderGround = false;
	m_underGroundEaseTimer = 1.0f;
	m_underGroundShake = 0;

	m_attackTimer = 0;

	m_jumpEndInvTimer.Reset(JUMP_END_INV_TIMER);

	m_deathSpriteAnimNumber = 0;
	m_deathSpriteAnimTimer = KuroEngine::Timer(DEATH_SPRITE_TIMER);

	m_hp = DEFAULT_HP;
	m_damageHitStopTimer.Reset(0.0f);
	m_nodamageTimer.Reset(0.0f);

	m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WAIT], true, false);
	m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
	m_beforePlayerMoveStatus = m_playerMoveStatus;

	//プレイヤーの球の判定
	m_sphere.m_centerPos = &m_drawTransform.GetPos();
	m_sphere.m_radius = &m_radius;
	m_radius = 2.0f;

	//HPのUI初期化
	m_hpUi.Init();

	m_playerMoveParticle.Init();
	m_playerMoveParticleTimer.Reset(PLAYER_MOVE_PARTICLE_SPAN);
	m_playerIdleParticleTimer.Reset(PLAYER_IDLE_PARTICLE_SPAN);

	//天井から始まったらカメラを反転させる。
	if (m_transform.GetUp().y <= -0.9f) {
		m_isCameraUpInverse = true;
	}

}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_0))
	{
		m_reaction->Init(MiniBug::FIND);
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_1))
	{
		m_reaction->Init(MiniBug::LOOK);
	}

	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_J))
	{
		Damage();
	}


	m_reaction->Update(m_drawTransform.GetPos());

	KuroEngine::Vec3<float>dir(GetOldPos() - GetNowPos());
	dir.Normalize();
	//m_dashEffect.Update(m_drawTransform.GetPos(), GetNowPos() != GetOldPos());


	//トランスフォームを保存。
	m_prevTransform = m_transform;

	//ステージの参照を保存。
	m_nowStage = arg_nowStage;

	//ステージを保存。
	m_stage = arg_nowStage;

	//プレイヤーが天井にいるかを判断。
	m_onCeiling = 0.5f < m_transform.GetUp().Dot({ 0, -1, 0 });

	//位置情報関係
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove() * TimeScaleMgr::s_inGame.GetTimeScale();

	//プレイヤーが天井にいたら左右のカメラ走査を反転。
	if (m_onCeiling || m_isCameraUpInverse) {
		scopeMove *= -1.0f;
	}

	//ジャンプができるかどうか。	一定時間地形に引っ掛かってたらジャンプできる。
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//カメラモードを切り替える。
	if (OperationConfig::Instance()->InputCamDistModeChange()) {
		++m_cameraMode;
		if (static_cast<int>(CAMERA_MODE.size()) <= m_cameraMode) {
			m_cameraMode = 0;
		}

		//SEを鳴らす。
		SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, m_cameraMode);
	}

	//移動ステータスによって処理を変える。
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//ジップライン
		m_canZip = OperationConfig::Instance()->InputRideZipLine();

		//地中に沈むフラグを更新。 イージングが終わっていたら。
		if (1.0f <= m_underGroundEaseTimer) {

			bool prevInInputUnderGround = m_isInputUnderGround;
			m_isInputUnderGround = OperationConfig::Instance()->InputSink();

			//沈むフラグが離されたトリガーだったら。
			if ((prevInInputUnderGround && !m_isInputUnderGround) || (!m_canOldUnderGroundRelease && m_canUnderGroundRelease)) {

				//攻撃する。
				m_attackTimer = ATTACK_TIMER;
			}

			//イージングが終わっている時のみ地中に潜ったり出たりする判定を持たせる。
			bool isInputOnOff = OperationConfig::Instance()->InputSinkOnOffTrigger();
			if ((isInputOnOff || (!m_isUnderGround && m_isInputUnderGround) || (m_isUnderGround && !m_isInputUnderGround)) && m_canUnderGroundRelease) {
				m_underGroundEaseTimer = 0;
			}

		}
		else {

			m_underGroundEaseTimer = std::clamp(m_underGroundEaseTimer + ADD_UNDERGROUND_EASE_TIMER, 0.0f, 1.0f);

			if (1.0f <= m_underGroundEaseTimer) {

				//地中にいる判定を更新。
				m_isUnderGround = m_isInputUnderGround;

				//地中にいたらコントローラーを振動させる。
				if (m_isUnderGround) {
					UsersInput::Instance()->ShakeController(0, 1.0f, 10);

					//画面を少しシェイク。
					m_underGroundShake = UNDER_GROUND_SHAKE;

					//地中から出た瞬間に大量にパーティクルを出す。
					for (int index = 0; index < 50; ++index) {

						//上ベクトルを基準に各軸を90度以内でランダムに回転させる。
						auto upVec = m_transform.GetUp();

						//各軸を回転させる量。 ラジアン 回転させるのはローカルのXZ平面のみで、Y軸は高さのパラメーターを持つ。
						KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

						//XZの回転量クォータニオン
						auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
						auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

						//上ベクトルを回転させる。
						upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

						m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), upVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
					}

				}

			}

			//イージングタイマーが0でプレイヤーが地中にいないとき(地中から出る演出の開始直後)だったらコントローラーをシェイクさせる。
			if (m_underGroundEaseTimer <= ADD_UNDERGROUND_EASE_TIMER && m_isUnderGround) {

				UsersInput::Instance()->ShakeController(0, 1.0f, 10);

				//画面を少しシェイク。
				m_underGroundShake = UNDER_GROUND_SHAKE;

				//地中から出た瞬間に大量にパーティクルを出す。
				for (int index = 0; index < 50; ++index) {

					//上ベクトルを基準に各軸を90度以内でランダムに回転させる。
					auto upVec = m_transform.GetUp();

					//各軸を回転させる量。 ラジアン 回転させるのはローカルのXZ平面のみで、Y軸は高さのパラメーターを持つ。
					KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

					//XZの回転量クォータニオン
					auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
					auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

					//上ベクトルを回転させる。
					upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

					m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), upVec.GetNormal() * m_growPlantPtLig.m_defInfluenceRange, m_moveSpeed);
				}

			}

		}

		//プレイヤーの回転をカメラ基準にする。(移動方向の基準がカメラの角度なため)
		if (m_onGround) {
			m_transform.SetRotate(m_cameraQ);
		}

		//入力された移動量を取得
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。


		//カメラの回転を保存。
		m_cameraRotYStorage += scopeMove.x;

		//入力量が一定以下だったら0にする。
		const float DEADLINE = 0.8f;
		if (m_rowMoveVec.Length() <= DEADLINE) {
			m_rowMoveVec = {};
		}

		//止まっていたら。
		if (m_rowMoveVec.Length() <= 0.0f) {

			//止まっているときに移動方向を反転させるかのフラグを初期化する。
			m_isCameraInvX = false;

		}
		else {

			//入力を反転させるか？

			if (m_isCameraInvX && (m_isCameraUpInverse && (0.9f < m_transform.GetUp().y || m_transform.GetUp().y < -0.9f))) {

				//m_rowMoveVec.x *= -1.0f;
				m_rowMoveVec.z *= -1.0f;

			}
			else if (m_isCameraInvX && (!m_isCameraUpInverse && (0.9f < m_transform.GetUp().y || m_transform.GetUp().y < -0.9f))) {

				//m_rowMoveVec.x *= -1.0f;
				m_rowMoveVec.z *= -1.0f;

			}
			else if (m_isCameraInvX || (m_isCameraUpInverse && (0.9f < m_transform.GetUp().x || m_transform.GetUp().x < -0.9f))) {

				m_rowMoveVec.x *= -1.0f;
				m_rowMoveVec.z *= -1.0f;

			}

		}


		//移動させる。
		if (!m_isDeath) {
			Move(newPos);
		}

		//カメラの回転を保存。
		m_cameraRotY = m_cameraRotYStorage;
		m_cameraRotMove = m_cameraRotYStorage;

		//移動した方向を保存。
		m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		//当たり判定
		m_collision.CheckHit(beforePos, newPos, arg_nowStage);

		m_transform.SetPos(newPos);

		//ジャンプが終わってから一定時間以内だったら、入力を戻す処理を書く。
		m_jumpEndInvTimer.UpdateTimer();
		if (!m_jumpEndInvTimer.IsTimeUp() && 0 < m_rowMoveVec.Length() && m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE) {

			//ジャンプ中にジャンプした時と反対方向に入力していたら and 操作が反転していたら = 反転を打ち消す。
			if (0.0f < m_jumpRowMoveVec.Dot(m_rowMoveVec) && m_isCameraInvX) {

				m_isCameraInvX = false;

			}

		}

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		//入力された移動量を取得
		m_jumpRowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。

		//入力量が一定以下だったら0にする。
		const float DEADLINE = 0.8f;
		if (m_jumpRowMoveVec.Length() <= DEADLINE) {
			m_jumpRowMoveVec = {};
		}

		//タイマーを更新。
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(InOut, Sine, m_jumpTimer, 0.0f, 1.0f);

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

			//移動量を0にする。
			m_moveSpeed = {};

			//面移動SEを鳴らす。
			SoundConfig::Instance()->Play(SoundConfig::SE_SURFACE_JUMP);

			//ジャンプ中にジャンプした時と反対方向に入力していたら and 操作が反転していたら = 反転を打ち消す。
			if (m_jumpRowMoveVec.Dot(m_jumpTrrigerRowMoveVec) < 0.0f && m_isCameraInvX) {

				m_isCameraInvX = false;

			}

		}
		m_transform.SetPos(newPos);

		//ジャンプ中は常時回転を適用させる。
		m_drawTransform.SetRotate(m_transform.GetRotate());

		m_jumpEndInvTimer.Reset();

	}
	break;
	case PLAYER_MOVE_STATUS::ZIP:
	{

		//ジップラインの更新処理
		UpdateZipline();

	}
	break;
	case PLAYER_MOVE_STATUS::DEATH:
	{

		//死亡の更新処理
		UpdateDeath();

		//動かさない。
		m_transform = m_prevTransform;

	}
	break;
	case PLAYER_MOVE_STATUS::DAMAGE:
	{

		//ダメージを受けた時の更新処理
		UpdateDamage();

	}
	break;
	default:
		break;
	}

	//座標変化適用
	m_ptLig.SetPos(newPos);

	//ギミックの移動を打ち消す。
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();

	//地中にいるときはライトを変える。
	if (m_isInputUnderGround || !m_canUnderGroundRelease) {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange - SUB_INFLUENCE_RANGE, MIN_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);
	}
	else {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange + ADD_INFLUENCE_RANGE, 0.0f, MAX_INFLUENCE_RANGE);
	}
	m_growPlantPtLig.m_defInfluenceRange = MAX_INFLUENCE_RANGE;

	//カメラをデフォルトの位置に戻すか。
	m_isCameraDefault = OperationConfig::Instance()->InputCamReset();
	if (m_isCameraDefault) {

		//SEを鳴らす。
		SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, 0);

	}

	//プレイヤーが動いているか。
	bool isMovePlayer = 0.1f < m_moveSpeed.Length();

	//死んでいたら死亡の更新処理を入れる。
	if (!m_isDeath) {
		//カメラ操作	//死んでいたら死んでいたときのカメラの処理に変えるので、ここの条件式に入れる。
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, CAMERA_MODE[m_cameraMode], arg_nowStage, m_isCameraUpInverse, m_isCameraDefault, m_isHitUnderGroundCamera, isMovePlayer, m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP, m_cameraQ, m_isWallFrontDir, m_drawTransform, m_frontWallNormal);

		m_deathEffectCameraZ = CAMERA_MODE[m_cameraMode];
	}
	else {

		//シェイクの分を戻す。
		m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() - m_shake);

		m_playerMoveStatus = PLAYER_MOVE_STATUS::DEATH;
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, m_deathEffectCameraZ, arg_nowStage, m_isCameraUpInverse, m_isCameraDefault, m_isHitUnderGroundCamera, isMovePlayer, m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP, m_cameraQ, m_isWallFrontDir, m_drawTransform, m_frontWallNormal);

	}
	//シェイクを計算。
	float timeScaleShakeAmount = m_deathShakeAmount * TimeScaleMgr::s_inGame.GetTimeScale() + m_damageShakeAmount + m_underGroundShake;
	m_shake.x = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);
	m_shake.y = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);
	m_shake.z = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);

	//シェイクをかける。
	m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() + m_shake);

	//描画用にトランスフォームを適用	
	//地中に潜っている時と戻っているときのイージング中のトランスフォームを計算。
	if (m_underGroundEaseTimer < 1.0f) {

		//地中にいるときといないときで処理を変える。
		if (m_isUnderGround) {

			float easeAmount = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_underGroundEaseTimer, 0.0f, 1.0f) * UNDERGROUND_Y;

			auto underPos = m_transform.GetPos() - m_transform.GetUp() * UNDERGROUND_Y;
			m_drawTransform.SetPos(underPos + m_transform.GetUp() * easeAmount);

		}
		else {

			float easeAmount = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, m_underGroundEaseTimer, 0.0f, 1.0f) * UNDERGROUND_Y;

			m_drawTransform.SetPos(m_transform.GetPos() - m_transform.GetUp() * easeAmount);

		}

	}
	//地中に潜っている時。
	else if (m_isUnderGround) {

		m_drawTransform.SetPos(m_transform.GetPos() - m_transform.GetUp() * UNDERGROUND_Y);

	}
	else {
		m_drawTransform.SetPos(m_transform.GetPos());
	}
	//回転は動いたときのみ適用させる。
	if (0 < m_rowMoveVec.Length()) {
		m_drawTransform.SetRotate(m_transform.GetRotate());
	}

	//ダメージを受けないタイマーを更新。
	m_nodamageTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	//アニメーション指定
	AnimationSpecification(beforePos, newPos);

	//モデルのアニメーター更新
	m_modelAnimator->Update(TimeScaleMgr::s_inGame.GetTimeScale());

	//動きのステータス記録
	m_beforePlayerMoveStatus = m_playerMoveStatus;
	//攻撃中タイマーを減らす。
	m_attackTimer = std::clamp(m_attackTimer - 1, 0, ATTACK_TIMER);

	//被ダメージ点滅更新
	{
		//無敵状態中点滅
		if (!m_nodamageTimer.IsTimeUp() && m_damageFlashTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale()))
		{
			m_damageFlash = !m_damageFlash;
			m_damageFlashTimer.Reset();
		}

		//無敵状態終了と同時に通常描画に
		if (m_nodamageTimer.IsTimeUpOnTrigger())
		{
			m_damageFlash = false;
		}
	}

	//地中に潜ったときのシェイク量を減らす。
	m_underGroundShake = std::clamp(m_underGroundShake - SUB_UNDER_GROUND_SHAKE, 0.0f, 100.0f);

	//HPUI更新
	m_hpUi.Update(TimeScaleMgr::s_inGame.GetTimeScale(), DEFAULT_HP, m_hp, m_nodamageTimer);

	//プレイヤーが動いた時のパーティクル挙動
	m_playerMoveParticle.Update();

}

void Player::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr, bool arg_cameraDraw)
{

	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	if (m_damageFlash)return;

	IndividualDrawParameter drawParam = IndividualDrawParameter::GetDefault();
	drawParam.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 1.0f, 0.0f);

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_drawTransform,
		drawParam,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());

	//KuroEngine::DrawFunc3D::DrawNonShadingModel(
	//	m_axisModel,
	//	m_drawTransform,
	//	arg_cam);

	KuroEngine::Transform debugTrans = m_camController.GetDebugTransform();

	KuroEngine::Vec3<float> pos = m_transform.GetPos();
	KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec * 10.0f, KuroEngine::Color(255, 0, 0, 255), 1.0f);
	KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec2 * 10.0f, KuroEngine::Color(0, 255, 0, 255), 1.0f);
	KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec3 * 10.0f, KuroEngine::Color(0, 0, 255, 255), 1.0f);

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			arg_cam);
	}
}

void Player::DrawParticle(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	//プレイヤーが動いた時のパーティクル挙動
	m_playerMoveParticle.Draw(arg_cam, arg_ligMgr);

	m_reaction->Draw(arg_cam);
	//m_dashEffect.Draw(arg_cam);
}

void Player::DrawUI(KuroEngine::Camera &arg_cam)
{
	using namespace KuroEngine;

	//死んでいる かつ アニメーションが終わっていなかったら
	bool isFinishAnimation = m_deathSpriteAnimNumber == DEATH_SPRITE_ANIM_COUNT && m_deathSpriteAnimTimer.IsTimeUp();
	if (m_deathStatus == Player::DEATH_STATUS::LEAVE && !isFinishAnimation) {

		Vec2<float> winCenter = WinApp::Instance()->GetExpandWinCenter();
		Vec2<float> spriteSize = Vec2<float>(512.0f, 512.0f);

		//KuroEngine::DrawFunc2D::DrawExtendGraph2D(winCenter - spriteSize, winCenter + spriteSize, m_deathAnimSprite[m_deathSpriteAnimNumber]);

	}

	//ダメージのヒットストップが効いていないときHPUI描画
	m_hpUi.Draw(DEFAULT_HP, m_hp, !m_damageHitStopTimer.IsTimeUp());
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

void Player::Damage()
{

	//死んでいたら処理を飛ばす。
	if (m_isDeath) return;

	//無敵中。
	if (!m_nodamageTimer.IsTimeUp()) return;

	//HPを減らす。
	m_hp = std::clamp(m_hp - 1, 0, std::numeric_limits<int>().max());

	//HPUI演出
	m_hpUi.OnDamage();

	//死んだら
	if (m_hp <= 0) {

		m_isDeath = true;

	}
	else {

		//各種タイマーを設定。
		m_nodamageTimer.Reset(NODAMAGE_TIMER);
		m_damageHitStopTimer.Reset(DAMAGE_HITSTOP_TIMER);

		//シェイクをかける。
		m_damageShakeAmount = DAMAGE_SHAKE_AMOUNT;

		//プレイヤーの状態をダメージ中に
		m_beforeDamageStatus = m_playerMoveStatus;
		m_playerMoveStatus = PLAYER_MOVE_STATUS::DAMAGE;
	}

	//ヒットストップ
	TimeScaleMgr::s_inGame.Set(0.0f);

	m_damageFlashTimer.Reset();

	//コントローラー振動
	KuroEngine::UsersInput::Instance()->ShakeController(0, 1.0f, 10);
}

Player::CHECK_HIT_GRASS_STATUS Player::CheckHitGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize)
{

	//攻撃状態じゃなかったら処理を戻す。
	if (!GetIsAttack()) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//ダメージを受けない状態だったら当たり判定を飛ばす。
	if (!m_damageHitStopTimer.IsTimeUp()) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//まずは球の判定
	float distance = (arg_enemyPos - m_transform.GetPosWorld()).Length();
	bool isHit = distance < m_growPlantPtLig.m_influenceRange;

	//当たっていなかったら処理を飛ばす。
	if (!isHit) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//プレイヤーから敵までのベクトルと敵の上ベクトルを内積して、その結果が0以下だったらライトが当たっている判定(草が当たっている判定。)
	bool isLight = (arg_enemyPos - m_transform.GetPosWorld()).GetNormal().Dot(arg_enemyUp) < 0;

	//ライトに当たっている判定
	if (!isLight) {

		//距離によって頭に当たっているかを判断
		if (distance < PLAYER_HEAD_SIZE) {

			return Player::CHECK_HIT_GRASS_STATUS::HEAD;

		}
		else {

			return Player::CHECK_HIT_GRASS_STATUS::AROUND;

		}

	}
	else {

		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;

	}

}

void Player::Move(KuroEngine::Vec3<float> &arg_newPos) {

	//落下中は入力を無効化。
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}

	float accelSpeed = m_defaultAccelSpeed;
	float maxSpeed = m_defaultMaxSpeed;
	float brake = m_defaultBrake;

	if (m_isUnderGround)
	{
		accelSpeed = m_underGroundAccelSpeed;
		maxSpeed = m_underGroundMaxSpeed;
		brake = m_underGroundBrake;
	}

	//移動量を回転させる
	auto accel = KuroEngine::Math::TransformVec3(m_rowMoveVec, m_transform.GetRotate()) * accelSpeed;


	m_moveSpeed += accel;

	//移動速度制限
	if (maxSpeed < m_moveSpeed.Length())
	{
		m_moveSpeed = m_moveSpeed.GetNormal() * maxSpeed;
	}

	//移動量加算
	arg_newPos += m_moveSpeed * TimeScaleMgr::s_inGame.GetTimeScale();

	//ギミックの移動量も加算。
	arg_newPos += m_gimmickVel;

	//地面に張り付ける用の重力。
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

	//動いていて、地中状態切り替え中じゃなかったら。
	const float PARTICLE_DEADLINE = 0.5f;
	if (PARTICLE_DEADLINE < m_moveSpeed.Length() && 1.0f <= m_underGroundEaseTimer) {

		//プレイヤーが動いた時のパーティクルを生成。
		m_playerMoveParticleTimer.UpdateTimer();
		if (m_playerMoveParticleTimer.IsTimeUpOnTrigger()) {
			//地中にいるかそうじゃないかでパーティクルを変える。
			if (m_isUnderGround) {
				//煙パーティクル。
				for (int index = 0; index < PLAYER_MOVE_PARTICLE_COUNT; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(SMOKE_SCATTER));
				}
				//オーブもちょっとだけ出す。
				for (int index = 0; index < 2; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
				}
			}
			else {
				//オーブを出す。
				for (int index = 0; index < PLAYER_MOVE_PARTICLE_COUNT; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_influenceRange));
				}
				//煙もちょっとだけ出す。
				for (int index = 0; index < 2; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
				}
			}
			m_playerMoveParticleTimer.Reset();
		}

		//移動しているときはシェイクさせる。
		if (m_isUnderGround) {
			KuroEngine::UsersInput::Instance()->ShakeController(0, 0.2f, 10);
		}

	}
	//動いていないときも適量のパーティクルを出す。
	else {

		m_playerIdleParticleTimer.UpdateTimer();
		if (m_playerIdleParticleTimer.IsTimeUpOnTrigger()) {

			//オーブを出す。
			for (int index = 0; index < 1; ++index) {
				KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

				const float SMOKE_SCATTER = 5.0f;
				m_playerMoveParticle.GenerateIdle(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_influenceRange));
			}

			m_playerIdleParticleTimer.Reset();

		}

	}

	//減速
	if (m_rowMoveVec.IsZero())
		m_moveSpeed = KuroEngine::Math::Lerp(m_moveSpeed, KuroEngine::Vec3<float>(0.0f, 0.0f, 0.0f), brake);
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

void Player::UpdateDeath() {

	switch (m_deathStatus)
	{
	case Player::DEATH_STATUS::APPROACH:
	{
		//カメラを近づける。
		m_deathEffectCameraZ += (DEATH_EFFECT_CAMERA_Z - m_deathEffectCameraZ) / 2.0f;

		//速度を遅くする。
		TimeScaleMgr::s_inGame.Set(DEATH_EFFECT_TIMER_SCALE);

		++m_deathEffectTimer;
		if (DEATH_EFFECT_APPROACH_TIMER <= m_deathEffectTimer) {
			m_deathStatus = DEATH_STATUS::STAY;

			//死亡演出のタイマーを初期化。
			m_deathEffectTimer = 0;
		}
	}
	break;
	case Player::DEATH_STATUS::STAY:
	{

		++m_deathEffectTimer;
		if (DEATH_EFFECT_STAY_TIMER <= m_deathEffectTimer) {
			m_deathStatus = DEATH_STATUS::LEAVE;

			//死亡演出のタイマーを初期化。
			m_deathEffectTimer = 0;

			//シェイクをかける。
			m_deathShakeAmount = DEATH_SHAKE_AMOUNT;
		}

	}
	break;
	case Player::DEATH_STATUS::LEAVE:
	{
		//カメラを離す。
		m_deathEffectCameraZ += (CAMERA_MODE[1] - m_deathEffectCameraZ) / 5.0f;

		//速度を元に戻す。
		TimeScaleMgr::s_inGame.Set(1.0f);

		//シェイク量をへらす。
		m_deathShakeAmount = std::clamp(m_deathShakeAmount - SUB_DEATH_SHAKE_AMOUNT, 0.0f, 100.0f);

		//死亡演出のアニメーションを更新。
		m_deathSpriteAnimTimer.UpdateTimer(1.0f);

		//タイマーが終わったら。
		if (m_deathSpriteAnimTimer.IsTimeUpOnTrigger()) {

			//次のアニメーションがあったら
			if (m_deathSpriteAnimNumber < DEATH_SPRITE_ANIM_COUNT - 1) {

				++m_deathSpriteAnimNumber;
				m_deathSpriteAnimTimer = KuroEngine::Timer(DEATH_SPRITE_TIMER);

			}

		}

		//スケールを小さくしていく。
		m_drawTransform.SetScale(m_drawTransform.GetScale() * 0.9f);

		++m_deathEffectTimer;
		if (DEATH_EFFECT_FINISH_TIMER <= m_deathEffectTimer) {

			m_isFinishDeathAnimation = true;
		}

	}
	break;
	default:
		break;
	}

}

void Player::UpdateDamage()
{
	//ヒットストップのタイマー終了
	if (m_damageHitStopTimer.UpdateTimer())
	{
		//通常のタイムスケールに戻す
		TimeScaleMgr::s_inGame.Set(1.0f);

		//ステータスを元に戻す。
		m_playerMoveStatus = m_beforeDamageStatus;

		//一応シェイク量を0にしておく。
		m_damageShakeAmount = 0;

		//ダメージ点滅開始
		m_damageFlash = true;
		m_damageFlashTimer.Reset();

		//コントローラー振動
		KuroEngine::UsersInput::Instance()->ShakeController(0, 1.0f, 20);

		//SE再生
		SoundConfig::Instance()->Play(SoundConfig::SE_PLAYER_DAMAGE);
	}
	else
	{
		//シェイク量をへらす。
		m_damageShakeAmount = std::clamp(m_damageShakeAmount - SUB_DAMAGE_SHAKE_AMOUNT, 0.0f, 100.0f);
	}
}
