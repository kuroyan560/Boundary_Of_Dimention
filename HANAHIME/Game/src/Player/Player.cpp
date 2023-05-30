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
#include"../Stage/CheckPointHitFlag.h"
#include"CameraController.h"

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

		//�O�x�N�g��
		auto front = m_transform.GetFront();
		ImGui::Text("Front : %.2f ,%.2f , %.2f", front.x, front.y, front.z);

		//��x�N�g��
		auto up = m_transform.GetUp();
		ImGui::Text("Up : %.2f ,%.2f , %.2f", up.x, up.y, up.z);

		ImGui::Text("OnGround : %d", m_onGround);

	}
}

void Player::AnimationSpecification(const KuroEngine::Vec3<float>& arg_beforePos, const KuroEngine::Vec3<float>& arg_newPos)
{
	//�ړ��X�e�[�^�X
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE || m_playerMoveStatus == PLAYER_MOVE_STATUS::LOOK_AROUND)
	{
		//�W�����v�A�j���[�V������
		if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_JUMP]))return;

		//�����ɕϓ���������
		if (0.01f < arg_beforePos.Distance(arg_newPos))
		{
			//���ɕ����A�j���[�V�����Đ���
			if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WALK]))return;

			//�����A�j���[�V�����Đ�
			m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WALK], true, false);
		}
		//�����Ȃ�
		else
		{
			//���ɑҋ@�A�j���[�V�����܂��̓L�����L�����A�j���[�V�����Đ���
			if (m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WAIT]) || m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_INTEREST]))return;

			//�L�����L��������J�E���^�[�f�N�������g
			m_animInterestCycleCounter--;

			//�܂��L�����L�����������K��ĂȂ�
			if (0 <= m_animInterestCycleCounter)
			{
				//�ҋ@�A�j���[�V�����Đ�
				m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WAIT], false, false);
			}
			//����I�ɃL�����L��������
			else
			{
				//�L�����L�����A�j���[�V�����Đ�
				m_modelAnimator->Play(m_animNames[ANIM_PATTERN_INTEREST], false, false);
				//�L�����L���������J�E���^�[���Z�b�g
				m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
			}
		}
	}
	//�W�����v�X�e�[�^�X
	else if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP)
	{
		//�g���K�[���łȂ�
		if (m_playerMoveStatus == m_beforePlayerMoveStatus)return;

		//�W�����v�A�j���[�V�����Đ�
		m_modelAnimator->Play(m_animNames[ANIM_PATTERN_JUMP], false, false);
	}

	//�ҋ@�A�j���[�V�����łȂ���΃L�����L���������J�E���^�[�����邱�Ƃ͂Ȃ�
	if (!m_modelAnimator->IsPlay(m_animNames[ANIM_PATTERN_WAIT]))
	{
		//�L�����L���������J�E���^�[���Z�b�g
		m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
	}
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true), m_growPlantPtLig(8.0f, &m_transform)
{
	using namespace KuroEngine;

	AddCustomParameter("Default_AccelSpeed", { "move","default","accelSpeed" }, PARAM_TYPE::FLOAT, &m_defaultAccelSpeed, "Move");
	AddCustomParameter("Default_MaxSpeed", { "move","default","maxSpeed" }, PARAM_TYPE::FLOAT, &m_defaultMaxSpeed, "Move");
	AddCustomParameter("Default_Brake", { "move","default","brake" }, PARAM_TYPE::FLOAT, &m_defaultBrake, "Move");
	AddCustomParameter("UnderGround_AccelSpeed", { "move","underGround","accelSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundAccelSpeed, "Move");
	AddCustomParameter("UnderGround_MaxSpeed", { "move","underGround","maxSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundMaxSpeed, "Move");
	AddCustomParameter("UnderGround_Brake", { "move","underGround","brake" }, PARAM_TYPE::FLOAT, &m_underGroundBrake, "Move");
	LoadParameterLog();

	//���f���ǂݍ���
	m_model = Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_headLeafModel = Importer::Instance()->LoadModel("resource/user/model/", "Player_Head_Leaf.glb");
	m_axisModel = Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//�J��������
	m_cam = std::make_shared<Camera>("Player's Camera");
	//�J�����̃R���g���[���[�ɃA�^�b�`
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

	m_collision.m_refPlayer = this;

	//���S�A�j���[�V������ǂݍ���
	D3D12App::Instance()->GenerateTextureBuffer(m_deathAnimSprite.data(), "resource/user/tex/Number.png", DEATH_SPRITE_ANIM_COUNT, KuroEngine::Vec2<int>(DEATH_SPRITE_ANIM_COUNT, 1));

	//�A�j���[�^�[����
	m_modelAnimator = std::make_shared<ModelAnimator>(m_model);

	m_cameraReturnTimer.Reset(CAMERA_RETURN_TIMER);
	m_cameraReturnTimer.ForciblyTimeUp();
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	//�X�P�[���ł�����
	arg_initTransform.SetScale(1.0f);

	m_initTransform = arg_initTransform;
	m_prevTransform = arg_initTransform;
	m_transform = arg_initTransform;
	m_drawTransform = arg_initTransform;
	m_cameraRotY = 0;
	m_cameraRotYStorage = arg_initTransform.GetRotateAsEuler().x;
	m_cameraRotMove = 0;
	m_cameraJumpLerpAmount = 0;
	m_cameraJumpLerpStorage = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
	m_canJumpDelayTimer = 0;
	m_deathTimer = 0;

	m_checkPointRotY = {};
	m_isCheckPointUpInverse = false;

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_gimmickVel = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_onGround = false;
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
	m_isPlayerOverHeat = false;
	m_cameraNoCollisionTimer.Reset(10);
	m_cameraFar = CAMERA_FAR;
	m_baseCameraFar = CAMERA_FAR;
	m_cameraMode = CameraController::CAMERA_STATUS::NORMAL;

	m_growPlantPtLig.Register();
	//���S���o�̃^�C�}�[���������B
	m_deathEffectTimer = 0;
	m_deathShakeAmount = 0;
	m_damageShakeAmount = 0;
	m_shake = KuroEngine::Vec3<float>();
	m_deathStatus = DEATH_STATUS::APPROACH;
	m_isFinishDeathAnimation = false;

	//��_���[�W�̓_�ŏ�����
	m_damageFlashTimer.Reset(6.0f);
	m_damageFlash = false;

	//�n���ɒ��ފ֘A
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

	//�v���C���[�̋��̔���
	m_sphere.m_centerPos = &m_drawTransform.GetPos();
	m_sphere.m_radius = &m_radius;
	m_radius = 2.0f;

	//UI������
	m_hpUi.Init();
	m_camModeUI.Init();

	m_playerMoveParticle.Init();
	m_playerMoveParticleTimer.Reset(PLAYER_MOVE_PARTICLE_SPAN);
	m_playerIdleParticleTimer.Reset(PLAYER_IDLE_PARTICLE_SPAN);

	//�V�䂩��n�܂�����J�����𔽓]������B
	if (m_transform.GetUp().y <= -0.9f) {
		m_isCameraUpInverse = true;
		m_isCheckPointUpInverse = true;
	}

	m_cameraReturnTimer.Reset(CAMERA_RETURN_TIMER);
	m_cameraReturnTimer.ForciblyTimeUp();

	//�v���C���[��Y�������ŃJ�����𔽓]�����邩�����߂�B
	if (m_transform.GetUp().y <= -0.9f) {
		m_isCameraUpInverse = true;
	}
	else {
		m_isCameraUpInverse = false;
	}
	m_camController.Init(arg_initTransform, m_isCameraUpInverse);
	m_camController.Respawn(m_transform, m_isCameraUpInverse);
	m_camController.LerpForcedToEnd(m_cameraRotYStorage);

}

void Player::Respawn(KuroEngine::Transform arg_initTransform, const std::weak_ptr<Stage>arg_nowStage, int arg_nowStageIndex, int arg_nowCheckPointIndex)
{

	//�X�P�[���ł�����
	arg_initTransform.SetScale(1.0f);

	m_initTransform = arg_initTransform;
	m_prevTransform = arg_initTransform;
	m_transform = arg_initTransform;
	m_drawTransform = arg_initTransform;
	m_cameraRotY = m_checkPointRotY;
	m_cameraRotYStorage = m_checkPointRotY;
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
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_canZip = false;
	m_isCameraInvX = false;
	m_canUnderGroundRelease = true;
	m_canOldUnderGroundRelease = true;
	m_onCeiling = false;
	m_isCameraUpInverse = m_isCheckPointUpInverse;
	m_isHitUnderGroundCamera = false;
	m_isCameraDefault = false;
	m_isOldCameraDefault = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
	m_isWallFrontDir = false;
	m_cameraNoCollisionTimer.Reset(60);
	m_cameraMode = CameraController::CAMERA_STATUS::NORMAL;

	m_growPlantPtLig.Register();
	//���S���o�̃^�C�}�[���������B
	m_deathEffectTimer = 0;
	m_deathShakeAmount = 0;
	m_damageShakeAmount = 0;
	m_shake = KuroEngine::Vec3<float>();
	m_deathStatus = DEATH_STATUS::APPROACH;
	m_isFinishDeathAnimation = false;

	//��_���[�W�̓_�ŏ�����
	m_damageFlashTimer.Reset(6.0f);
	m_damageFlash = false;

	//�n���ɒ��ފ֘A
	m_isInputUnderGround = false;
	m_isUnderGround = false;
	m_underGroundEaseTimer = 1.0f;
	m_underGroundShake = 0;

	m_cameraFar = CAMERA_FAR;
	m_baseCameraFar = CAMERA_FAR;

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

	//�v���C���[�̋��̔���
	m_sphere.m_centerPos = &m_drawTransform.GetPos();
	m_sphere.m_radius = &m_radius;
	m_radius = 2.0f;

	//UI������
	m_hpUi.Init();
	m_camModeUI.Init();

	m_playerMoveParticle.Init();
	m_playerMoveParticleTimer.Reset(PLAYER_MOVE_PARTICLE_SPAN);
	m_playerIdleParticleTimer.Reset(PLAYER_IDLE_PARTICLE_SPAN);

	m_cameraReturnTimer.Reset(CAMERA_RETURN_TIMER);
	m_cameraReturnTimer.ForciblyTimeUp();

	//�v���C���[��Y�������ŃJ�����𔽓]�����邩�����߂�B
	if (m_transform.GetUp().y <= -0.9f) {
		m_isCameraUpInverse = true;
	}
	else {
		m_isCameraUpInverse = false;
	}
	m_camController.Init(arg_initTransform, m_isCameraUpInverse, true);

	//���X�|�[���ʒu�ɂ���ăJ�����̏����ݒ��ς���B
	if (arg_nowStageIndex == 0 && arg_nowCheckPointIndex == 0) {
		m_camController.SetParam(0.240949869f, 4.01524258f, false);
		m_cameraRotYStorage = 4.01524258f;
	}else if (arg_nowStageIndex == 0 && arg_nowCheckPointIndex == 1) {
		m_camController.SetParam(-0.662380993f, 4.65234709f, true);
		m_cameraRotYStorage = 4.65234709f;
	}
	else if (arg_nowStageIndex == 1 && arg_nowCheckPointIndex == 0) {
		m_camController.SetParam(0.603274345f, 1.61108458f, false);
		m_cameraRotYStorage = 1.61108458f;
	}
	else if (arg_nowStageIndex == 1 && arg_nowCheckPointIndex == 1) {
		m_camController.SetParam(0.404909104f, 1.61935687f, false);
		m_cameraRotYStorage = 1.61935687f;
	}
	else if (arg_nowStageIndex == 1 && arg_nowCheckPointIndex == 2) {
		m_camController.SetParam(0.698131680f, 0.0527084842f, false);
		m_cameraRotYStorage = 0.0527084842f;
	}
	else if (arg_nowStageIndex == 2 && arg_nowCheckPointIndex == 0) {
		m_camController.SetParam(0.443914831f, -1.54169261f, false);
		m_cameraRotYStorage = -1.54169261f;
	}
	else if (arg_nowStageIndex == 2 && arg_nowCheckPointIndex == 1) {
		m_cameraFar = -100.0f;
	}
	else if (arg_nowStageIndex == 2 && arg_nowCheckPointIndex == 2) {
		m_camController.SetParam(-0.349065840f, 0.755516231f, true);
		m_cameraRotYStorage = 0.755516231f;
	}
	else if (arg_nowStageIndex == 2 && arg_nowCheckPointIndex == 3) {
		m_camController.SetParam(-0.481665850f, 4.70555067f, true);
		m_cameraRotYStorage = 4.70555067f;
	}
	else if (arg_nowStageIndex == 2 && arg_nowCheckPointIndex == 4) {
		m_camController.SetParam(-0.578085601f, 4.65286255f, true);
		m_cameraRotYStorage = 4.65286255f;
	}

	m_camController.LerpForcedToEnd(m_cameraRotYStorage);

	m_isPlantLightExplosion = false;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	//�`�F�b�N�|�C���g�ɓ��B�����u�Ԃ�������A�g�����X�t�H�[����ۑ����Ă����B
	if (CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger) {
		m_checkPointRotY = m_cameraRotYStorage;
		m_isCheckPointUpInverse = m_isCameraUpInverse;
	}


	KuroEngine::Vec3<float>dir(GetOldPos() - GetNowPos());
	dir.Normalize();
	//m_dashEffect.Update(m_drawTransform.GetPos(), GetNowPos() != GetOldPos());


	//�g�����X�t�H�[����ۑ��B
	m_prevTransform = m_transform;

	//�X�e�[�W�̎Q�Ƃ�ۑ��B
	m_nowStage = arg_nowStage;

	//�X�e�[�W��ۑ��B
	m_stage = arg_nowStage;

	//�v���C���[���V��ɂ��邩�𔻒f�B
	m_onCeiling = 0.5f < m_transform.GetUp().Dot({ 0, -1, 0 });

	//�ʒu���֌W
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove() * TimeScaleMgr::s_inGame.GetTimeScale();

	//�v���C���[���V��ɂ����獶�E�̃J���������𔽓]�B
	if (m_onCeiling || m_isCameraUpInverse) {
		scopeMove *= -1.0f;
	}

	//���X�|�[�����Ă��琔�t���[���͓��͂��󂯕t���Ȃ��B(�J�������������ʒu�ɃZ�b�g�����܂ő҂B)
	if (!m_cameraNoCollisionTimer.IsTimeUp()) {
		scopeMove = KuroEngine::Vec3<float>();
	}

	//C���p�̃J��������������͂�؂�B
	if (m_cameraFunaMode == CameraController::CAMERA_FUNA_MODE::STAGE1_1_C) {
		scopeMove = KuroEngine::Vec3<float>();
	}
	if (m_cameraFunaMode == CameraController::CAMERA_FUNA_MODE::STAGE1_2_INV_U) {
		scopeMove = KuroEngine::Vec3<float>();
		m_isCameraUpInverse = true;
	}

	//�W�����v���ł��邩�ǂ����B	��莞�Ԓn�`�Ɉ����|�����Ă���W�����v�ł���B
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//�ړ��X�e�[�^�X�ɂ���ď�����ς���B
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		m_cameraMode = CameraController::CAMERA_STATUS::NORMAL;

		//���͂�����������͌��n�����[�h�ɐ؂�ւ��B
		if (OperationConfig::Instance()->GetOperationInput(OperationConfig::CAM_DIST_MODE_CHANGE_LOOK_AROUND, OperationConfig::ON_TRIGGER)) {

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, 0);

			m_cameraMode = CameraController::CAMERA_STATUS::LOOK_AROUND;
			m_playerMoveStatus = PLAYER_MOVE_STATUS::LOOK_AROUND;

			//�����������B
			m_moveSpeed = Vec3<float>();

			m_camModeUI.Appear();

			break;

		}

		//�W�b�v���C��
		m_canZip = OperationConfig::Instance()->GetOperationInput(OperationConfig::RIDE_ZIP_LINE, OperationConfig::ON_TRIGGER);

		//�n���ɒ��ރt���O���X�V�B �C�[�W���O���I����Ă�����B
		if (1.0f <= m_underGroundEaseTimer) {

			//�n���ɂ���Ƃ��̃t���O�̓��͂��X�V�B
			m_isInputUnderGround = OperationConfig::Instance()->GetOperationInput(OperationConfig::SINK_GROUND, OperationConfig::HOLD) && !m_isPlayerOverHeat;
			bool isInputUnderGroundRelease = OperationConfig::Instance()->GetOperationInput(OperationConfig::SINK_GROUND, OperationConfig::OFF_TRIGGER);
			bool isInputUnderGroundTrigger = OperationConfig::Instance()->GetOperationInput(OperationConfig::SINK_GROUND, OperationConfig::ON_TRIGGER);

			//�I�[�o�[�q�[�g���ɐ��낤�Ƃ�������ʉ���炷�B
			if (m_isPlayerOverHeat && isInputUnderGroundTrigger) {

				SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);

			}

			//�n���ɂ�����
			if (m_isUnderGround) {

				//���ރt���O�������ꂽ�g���K�[��������B
				if ((!m_isInputUnderGround && m_canUnderGroundRelease) || m_isPlayerOverHeat) {

					//�U������B
					m_attackTimer = ATTACK_TIMER;

					//����łȂ���Ԃɂ���B
					m_underGroundEaseTimer = 0;

				}

			}
			//�n���ɂ��Ȃ�������
			else {

				if (m_isInputUnderGround && !m_isPlayerOverHeat) {

					//����łȂ���Ԃɂ���B
					m_underGroundEaseTimer = 0;

				}

			}

		}
		else {

			m_underGroundEaseTimer = std::clamp(m_underGroundEaseTimer + ADD_UNDERGROUND_EASE_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

			if (1.0f <= m_underGroundEaseTimer) {

				//�n���ɂ��锻����X�V�B
				m_isUnderGround = m_isInputUnderGround;

				//�n���ɂ�����R���g���[���[��U��������B
				if (m_isUnderGround) {

					if (0 < TimeScaleMgr::s_inGame.GetTimeScale()) {
						UsersInput::Instance()->ShakeController(0, 1.0f * TimeScaleMgr::s_inGame.GetTimeScale(), 10);
					}

					//��ʂ������V�F�C�N�B
					m_underGroundShake = UNDER_GROUND_SHAKE * TimeScaleMgr::s_inGame.GetTimeScale();

					//�n������o���u�Ԃɑ�ʂɃp�[�e�B�N�����o���B
					for (int index = 0; index < 50.0f * TimeScaleMgr::s_inGame.GetTimeScale(); ++index) {

						//��x�N�g������Ɋe����90�x�ȓ��Ń����_���ɉ�]������B
						auto upVec = m_transform.GetUp();

						//�e������]������ʁB ���W�A�� ��]������̂̓��[�J����XZ���ʂ݂̂ŁAY���͍����̃p�����[�^�[�����B
						KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

						//XZ�̉�]�ʃN�H�[�^�j�I��
						auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
						auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

						//��x�N�g������]������B
						upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

						m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), upVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
					}

				}

			}

			//�C�[�W���O�^�C�}�[��0�Ńv���C���[���n���ɂ��Ȃ��Ƃ�(�n������o�鉉�o�̊J�n����)��������R���g���[���[���V�F�C�N������B
			if (m_underGroundEaseTimer <= ADD_UNDERGROUND_EASE_TIMER && m_isUnderGround) {

				//�I�[�o�[�q�[�g���͉����o���B
				if (m_isPlayerOverHeat) {

					for (int index = 0; index < 50.0f * TimeScaleMgr::s_inGame.GetTimeScale(); ++index) {

						//��x�N�g������Ɋe����90�x�ȓ��Ń����_���ɉ�]������B
						auto upVec = m_transform.GetUp();

						//�e������]������ʁB ���W�A�� ��]������̂̓��[�J����XZ���ʂ݂̂ŁAY���͍����̃p�����[�^�[�����B
						KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

						//XZ�̉�]�ʃN�H�[�^�j�I��
						auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
						auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

						//��x�N�g������]������B
						upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

						m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), upVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
					}

					//�n������o���u�Ԃɑ�ʂɃp�[�e�B�N�����o���B
					for (int index = 0; index < 50.0f * TimeScaleMgr::s_inGame.GetTimeScale(); ++index) {

						//��x�N�g������Ɋe����90�x�ȓ��Ń����_���ɉ�]������B
						auto upVec = m_transform.GetUp();

						//�e������]������ʁB ���W�A�� ��]������̂̓��[�J����XZ���ʂ݂̂ŁAY���͍����̃p�����[�^�[�����B
						KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

						//XZ�̉�]�ʃN�H�[�^�j�I��
						auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
						auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

						//��x�N�g������]������B
						upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

						//�v���C���[�܂ł̃x�N�g��
						KuroEngine::Vec3<float> playerDir = (m_transform.GetPos() - (m_transform.GetPos() + upVec.GetNormal() * m_growPlantPtLig.m_defInfluenceRange / 3.0f)).GetNormal();

						m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), upVec.GetNormal() * m_growPlantPtLig.m_defInfluenceRange / 3.0f, m_moveSpeed - playerDir * 0.75f);
					}

					if (0 < TimeScaleMgr::s_inGame.GetTimeScale()) {
						UsersInput::Instance()->ShakeController(0, 1.0f * TimeScaleMgr::s_inGame.GetTimeScale(), 60);
					}

					//��ʂ������V�F�C�N�B
					m_underGroundShake = (UNDER_GROUND_SHAKE * 5.0f) * TimeScaleMgr::s_inGame.GetTimeScale();

				}
				else {

					//�n������o���u�Ԃɑ�ʂɃp�[�e�B�N�����o���B
					for (int index = 0; index < 50.0f * TimeScaleMgr::s_inGame.GetTimeScale(); ++index) {

						//��x�N�g������Ɋe����90�x�ȓ��Ń����_���ɉ�]������B
						auto upVec = m_transform.GetUp();

						//�e������]������ʁB ���W�A�� ��]������̂̓��[�J����XZ���ʂ݂̂ŁAY���͍����̃p�����[�^�[�����B
						KuroEngine::Vec3<float> randomAngle = KuroEngine::GetRand(KuroEngine::Vec3<float>(-DirectX::XM_PIDIV2, -1.0f, -DirectX::XM_PIDIV2), KuroEngine::Vec3<float>(DirectX::XM_PIDIV2, 1.0f, DirectX::XM_PIDIV2));

						//XZ�̉�]�ʃN�H�[�^�j�I��
						auto xq = DirectX::XMQuaternionRotationAxis(m_transform.GetRight(), randomAngle.x);
						auto zq = DirectX::XMQuaternionRotationAxis(m_transform.GetFront(), randomAngle.z);

						//��x�N�g������]������B
						upVec = KuroEngine::Math::TransformVec3(upVec, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(xq, zq)));

						//�v���C���[�܂ł̃x�N�g��
						KuroEngine::Vec3<float> playerDir = (m_transform.GetPos() - (m_transform.GetPos() + upVec.GetNormal() * m_growPlantPtLig.m_defInfluenceRange / 3.0f)).GetNormal();

						m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), upVec.GetNormal() * m_growPlantPtLig.m_defInfluenceRange * 0.8f, m_moveSpeed - playerDir * 0.25f);
					}

					if (0 < TimeScaleMgr::s_inGame.GetTimeScale()) {
						UsersInput::Instance()->ShakeController(0, 1.0f * TimeScaleMgr::s_inGame.GetTimeScale(), 10);
					}

					//��ʂ������V�F�C�N�B
					m_underGroundShake = UNDER_GROUND_SHAKE * TimeScaleMgr::s_inGame.GetTimeScale();
				}

			}

		}

		//�v���C���[�̉�]���J������ɂ���B(�ړ������̊���J�����̊p�x�Ȃ���)
		if (m_onGround) {
			m_transform.SetRotate(m_cameraQ);
		}

		//���͂��ꂽ�ړ��ʂ��擾
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B


		//�J�����̉�]��ۑ��B
		m_cameraRotYStorage += scopeMove.x;

		//���͗ʂ����ȉ���������0�ɂ���B
		const float DEADLINE = 0.8f;
		if (m_rowMoveVec.Length() <= DEADLINE) {
			m_rowMoveVec = {};
		}

		//�~�܂��Ă�����B
		if (m_rowMoveVec.Length() <= 0.0f) {

			//�~�܂��Ă���Ƃ��Ɉړ������𔽓]�����邩�̃t���O������������B
			m_isCameraInvX = false;

		}
		else {

			//���͂𔽓]�����邩�H

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


		//�ړ�������B
		if (!m_isDeath) {
			Move(newPos);
		}

		//�J�����̉�]��ۑ��B
		m_cameraRotY = m_cameraRotYStorage;
		m_cameraRotMove = m_cameraRotYStorage;

		//�ړ�����������ۑ��B
		m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		//�����蔻��
		m_collision.CheckHit(beforePos, newPos, arg_nowStage);

		m_transform.SetPos(newPos);

		//�W�����v���I����Ă����莞�Ԉȓ���������A���͂�߂������������B
		m_jumpEndInvTimer.UpdateTimer();
		if (!m_jumpEndInvTimer.IsTimeUp() && 0 < m_rowMoveVec.Length() && m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE) {

			//�W�����v���ɃW�����v�������Ɣ��Ε����ɓ��͂��Ă����� and ���삪���]���Ă����� = ���]��ł������B
			if (0.0f < m_jumpRowMoveVec.Dot(m_rowMoveVec) && m_isCameraInvX) {

				m_isCameraInvX = false;

			}

		}

		//�J�������f�t�H���g�̈ʒu�ɖ߂����B
		m_isCameraDefault = OperationConfig::Instance()->GetOperationInput(OperationConfig::CAM_RESET, OperationConfig::ON_TRIGGER);
		if (m_isCameraDefault) {

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, 0);


		}

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		scopeMove = Vec3<float>();

		//���͂��ꂽ�ړ��ʂ��擾
		m_jumpRowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B

		//���͗ʂ����ȉ���������0�ɂ���B
		const float DEADLINE = 0.8f;
		if (m_jumpRowMoveVec.Length() <= DEADLINE) {
			m_jumpRowMoveVec = {};
		}

		//�^�C�}�[���X�V�B
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(InOut, Sine, m_jumpTimer, 0.0f, 1.0f);

		//�J�����̉�]���Ԃ���B
		m_cameraRotMove = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;

		//���W���Ԃ���B
		newPos = CalculateBezierPoint(easeAmount, m_jumpStartPos, m_jumpEndPos, m_bezierCurveControlPos);

		//��]��⊮����B
		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_jumpStartQ, m_jumpEndQ, easeAmount));

		//����ɒB���Ă�����W�����v���I����B
		if (1.0f <= m_jumpTimer) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraJumpLerpAmount = 0;

			//�ړ��ʂ�0�ɂ���B
			m_moveSpeed = {};

			//�ʈړ�SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_SURFACE_JUMP);

			//�W�����v���ɃW�����v�������Ɣ��Ε����ɓ��͂��Ă����� and ���삪���]���Ă����� = ���]��ł������B
			if (m_jumpRowMoveVec.Dot(m_jumpTrrigerRowMoveVec) < 0.0f && m_isCameraInvX) {

				m_isCameraInvX = false;

			}

		}
		m_transform.SetPos(newPos);

		//�W�����v���͏펞��]��K�p������B
		m_drawTransform.SetRotate(m_transform.GetRotate());

		m_jumpEndInvTimer.Reset();

	}
	break;
	case PLAYER_MOVE_STATUS::ZIP:
	{

		//�W�b�v���C���̍X�V����
		UpdateZipline();

	}
	break;
	case PLAYER_MOVE_STATUS::DEATH:
	{

		//���S�̍X�V����
		UpdateDeath();

		//�������Ȃ��B
		m_transform = m_prevTransform;

	}
	break;
	case PLAYER_MOVE_STATUS::DAMAGE:
	{

		//�_���[�W���󂯂����̍X�V����
		UpdateDamage();

	}
	break;
	case PLAYER_MOVE_STATUS::LOOK_AROUND:
	{

		m_cameraMode = CameraController::CAMERA_STATUS::LOOK_AROUND;

		//���͂�����������͌��n�����[�h�ɐ؂�ւ��B
		if (OperationConfig::Instance()->GetOperationInput(OperationConfig::CAM_DIST_MODE_CHANGE_LOOK_AROUND, OperationConfig::ON_TRIGGER)) {

			//SE��炷�B
			if (!m_camController.IsFinishLookAround()) {
				SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, 0);
			}

			m_camController.EndLookAround();

			m_camModeUI.Disappear();
		}

		if (m_camController.IsCompleteFinishLookAround()) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraMode = CameraController::CAMERA_STATUS::NORMAL;
		}

		m_moveSpeed = Vec3<float>();
		m_rowMoveVec = Vec3<float>();
		Move(newPos);
		m_transform.SetPos(newPos);

	}
	break;
	break;
	default:
		break;
	}

	//���W�ω��K�p
	m_ptLig.SetPos(newPos);

	//�M�~�b�N�̈ړ���ł������B
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();

	//�G�ɋ߂Â����Ƃ��Ƀf�t�H���g�̋����̖߂�܂ł̃^�C�}�[���X�V�B
	m_cameraReturnTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	//�n���ɂ���Ƃ��̓��C�g��ς���B
	if ((m_isInputUnderGround || !m_canUnderGroundRelease) && !m_isPlayerOverHeat) {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange - SUB_INFLUENCE_RANGE * TimeScaleMgr::s_inGame.GetTimeScale(), MIN_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);

		//�n���ɋ������ă��C�g���ŏ��ɂȂ��Ă��܂�����
		if (m_growPlantPtLig.m_influenceRange <= MIN_INFLUENCE_RANGE) {

			m_isPlayerOverHeat = true;

		}
	}
	else if (!(m_isPlayerOverHeat && m_isUnderGround)) {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange + ADD_INFLUENCE_RANGE * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, MAX_INFLUENCE_RANGE);


		//�n���ɋ������ă��C�g���ő�ɂȂ��Ă��܂�����
		if (MAX_INFLUENCE_RANGE <= m_growPlantPtLig.m_influenceRange) {

			m_isPlayerOverHeat = false;

		}
	}
	m_growPlantPtLig.m_defInfluenceRange = MAX_INFLUENCE_RANGE;

	//�v���C���[�������Ă��邩�B
	bool isMovePlayer = 0.1f < m_moveSpeed.Length();

	//�G���߂��ɂ��邩�H
	if (!m_cameraReturnTimer.IsTimeUp()) {
		m_baseCameraFar = KuroEngine::Math::Lerp(m_baseCameraFar, CAMERA_NEAR_ENEMY_FAR, 0.08f);
	}
	else if (m_cameraFunaMode == CameraController::CAMERA_FUNA_MODE::STAGE1_4_FAR) {
		m_baseCameraFar = KuroEngine::Math::Lerp(m_baseCameraFar, CAMERA_FUNAMODE_FAR, 0.08f);
	}
	else {
		m_baseCameraFar = KuroEngine::Math::Lerp(m_baseCameraFar, CAMERA_FAR, 0.08f);
	}


	//FunaSphere�Ƃ̓����蔻��
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE) {
		CheckHitFunaCamera(arg_nowStage);
	}
	else if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP && m_cameraFunaMode == CameraController::CAMERA_FUNA_MODE::STAGE1_2_INV_U) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::NORMAL;
	}


	//�f�o�b�O�p
#ifdef _DEBUG

	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_1)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::NORMAL;
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_2)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::STAGE1_1_C;
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_3)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::STAGE1_2_INV_U;
		m_cameraRotYStorage = fabs(fmod(m_cameraRotYStorage, DirectX::XM_2PI));
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_4)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::STAGE1_3_HEIGHT_LIMIT;
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_5)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::STAGE1_4_FAR;
	}
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_6)) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::STAGE1_6_NEAR_GOAL;
		m_cameraRotYStorage = fabs(fmod(m_cameraRotYStorage, DirectX::XM_2PI));
	}


#endif


	//����ł����玀�S�̍X�V����������B
	if (!m_isDeath) {
		//�J��������	//����ł����玀��ł����Ƃ��̃J�����̏����ɕς���̂ŁA�����̏������ɓ����B
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, m_baseCameraFar, arg_nowStage, m_isCameraUpInverse, m_isCameraDefault, m_isHitUnderGroundCamera, isMovePlayer, m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP, m_cameraQ, m_isWallFrontDir, m_drawTransform, m_frontWallNormal, !m_cameraNoCollisionTimer.IsTimeUp(), m_cameraMode, m_cameraFunaMode);

		m_deathEffectCameraZ = m_baseCameraFar;
	}
	else {

		//�V�F�C�N�̕���߂��B
		m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() - m_shake);

		m_playerMoveStatus = PLAYER_MOVE_STATUS::DEATH;
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, m_deathEffectCameraZ, arg_nowStage, m_isCameraUpInverse, m_isCameraDefault, m_isHitUnderGroundCamera, isMovePlayer, m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP, m_cameraQ, m_isWallFrontDir, m_drawTransform, m_frontWallNormal, !m_cameraNoCollisionTimer.IsTimeUp(), m_cameraMode, m_cameraFunaMode);

	}
	//�V�F�C�N���v�Z�B
	float timeScaleShakeAmount = m_deathShakeAmount * TimeScaleMgr::s_inGame.GetTimeScale() + m_damageShakeAmount + m_underGroundShake;
	m_shake.x = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);
	m_shake.y = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);
	m_shake.z = KuroEngine::GetRand(-timeScaleShakeAmount, timeScaleShakeAmount);

	//�V�F�C�N��������B
	m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() + m_shake);

	//�`��p�Ƀg�����X�t�H�[����K�p	
	//�n���ɐ����Ă��鎞�Ɩ߂��Ă���Ƃ��̃C�[�W���O���̃g�����X�t�H�[�����v�Z�B
	if (m_underGroundEaseTimer < 1.0f) {

		//�n���ɂ���Ƃ��Ƃ��Ȃ��Ƃ��ŏ�����ς���B
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
	//�n���ɐ����Ă��鎞�B
	else if (m_isUnderGround) {

		m_drawTransform.SetPos(m_transform.GetPos() - m_transform.GetUp() * UNDERGROUND_Y);

	}
	else {
		m_drawTransform.SetPos(m_transform.GetPos());
	}
	//��]�͓������Ƃ��̂ݓK�p������B
	if (0 < m_rowMoveVec.Length()) {
		m_drawTransform.SetRotate(m_transform.GetRotate());
	}

	//�_���[�W���󂯂Ȃ��^�C�}�[���X�V�B
	m_nodamageTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	//�A�j���[�V�����w��
	AnimationSpecification(beforePos, newPos);

	//���f���̃A�j���[�^�[�X�V
	m_modelAnimator->Update(TimeScaleMgr::s_inGame.GetTimeScale());

	//�����̃X�e�[�^�X�L�^
	m_beforePlayerMoveStatus = m_playerMoveStatus;
	//�U�����^�C�}�[�����炷�B
	m_attackTimer = std::clamp(m_attackTimer - 1, 0, ATTACK_TIMER);

	//��_���[�W�_�ōX�V
	{
		//���G��Ԓ��_��
		if (!m_nodamageTimer.IsTimeUp() && m_damageFlashTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale()))
		{
			m_damageFlash = !m_damageFlash;
			m_damageFlashTimer.Reset();
		}

		//���G��ԏI���Ɠ����ɒʏ�`���
		if (m_nodamageTimer.IsTimeUpOnTrigger())
		{
			m_damageFlash = false;
		}
	}

	//�n���ɐ������Ƃ��̃V�F�C�N�ʂ����炷�B
	m_underGroundShake = std::clamp(m_underGroundShake - SUB_UNDER_GROUND_SHAKE, 0.0f, 100.0f);

	//UI�X�V
	m_hpUi.Update(TimeScaleMgr::s_inGame.GetTimeScale(), DEFAULT_HP, m_hp, m_nodamageTimer);
	m_camModeUI.Update(TimeScaleMgr::s_inGame.GetTimeScale());

	//�v���C���[�����������̃p�[�e�B�N������
	m_playerMoveParticle.Update();

	m_cameraNoCollisionTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());


	//�W�����v��̃N�[���^�C�����C���B
	m_canJumpDelayTimer = std::clamp(m_canJumpDelayTimer - 1, 0, 100);

	if (m_isPlantLightExplosion)
	{
		m_plantLightExplosionTimer.UpdateTimer();

		float oldRange = m_growPlantPtLig.m_influenceRange;
		static const float PLANT_LIGHT_EXPLOSION_SCALE = 10.0f;
		m_growPlantPtLig.m_influenceRange = Math::Ease(Out, Circ,
			m_plantLightExplosionTimer.GetTimeRate(), m_plantLightExplosionStart, m_plantLightExplosionStart * PLANT_LIGHT_EXPLOSION_SCALE);

		m_growPlantPtLig.m_defInfluenceRange = Math::Ease(Out, Circ,
			m_plantLightExplosionTimer.GetTimeRate(), m_plantLightExplosionStart, m_plantLightExplosionStart * PLANT_LIGHT_EXPLOSION_SCALE);
	}

	////�e�X�g�p
	//FastTravel::Instance()->Update();
	//m_camController.GetCamera().lock()->GetTransform() = FastTravel::Instance()->GetCamera()->GetTransform();

}

void Player::Draw(KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_ds, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
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

	//�����Ă�����
	if ((m_isUnderGround && 1.0f <= m_underGroundEaseTimer)) {

		BasicDraw::Instance()->Draw_Player(
			arg_cam,
			arg_ds,
			arg_ligMgr,
			m_headLeafModel,
			m_drawTransform,
			drawParam,
			KuroEngine::AlphaBlendMode_None,
			m_modelAnimator->GetBoneMatBuff());

	}
	else {

		BasicDraw::Instance()->Draw_Player(
			arg_cam,
			arg_ds,
			arg_ligMgr,
			m_model,
			m_drawTransform,
			drawParam,
			KuroEngine::AlphaBlendMode_None,
			m_modelAnimator->GetBoneMatBuff());

	}

	//KuroEngine::DrawFunc3D::DrawNonShadingModel(
	//	m_axisModel,
	//	m_drawTransform,
	//	arg_cam);

	KuroEngine::Transform debugTrans = m_camController.GetDebugTransform();

	//KuroEngine::Vec3<float> pos = m_transform.GetPos();
	//KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec * 10.0f, KuroEngine::Color(255, 0, 0, 255), 1.0f);
	//KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec2 * 10.0f, KuroEngine::Color(0, 255, 0, 255), 1.0f);
	//KuroEngine::DrawFunc3D::DrawLine(arg_cam, pos, pos + m_debugVec3 * 10.0f, KuroEngine::Color(0, 0, 255, 255), 1.0f);

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			arg_cam);
	}
}

void Player::DrawParticle(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	//�v���C���[�����������̃p�[�e�B�N������
	m_playerMoveParticle.Draw(arg_cam, arg_ligMgr);
	//m_dashEffect.Draw(arg_cam);
}

void Player::DrawUI(KuroEngine::Camera& arg_cam)
{
	using namespace KuroEngine;

	//����ł��� ���� �A�j���[�V�������I����Ă��Ȃ�������
	bool isFinishAnimation = m_deathSpriteAnimNumber == DEATH_SPRITE_ANIM_COUNT && m_deathSpriteAnimTimer.IsTimeUp();
	if (m_deathStatus == Player::DEATH_STATUS::LEAVE && !isFinishAnimation) {

		Vec2<float> winCenter = WinApp::Instance()->GetExpandWinCenter();
		Vec2<float> spriteSize = Vec2<float>(512.0f, 512.0f);

		//KuroEngine::DrawFunc2D::DrawExtendGraph2D(winCenter - spriteSize, winCenter + spriteSize, m_deathAnimSprite[m_deathSpriteAnimNumber]);

	}

	//UI�`��
	m_hpUi.Draw(DEFAULT_HP, m_hp, !m_damageHitStopTimer.IsTimeUp());
	m_camModeUI.Draw();
}

void Player::Finalize()
{
}

void Player::CheckHitFunaCamera(const std::weak_ptr<Stage> arg_nowStage)
{
	using namespace KuroEngine;

	bool isHitFuna = false;
	for (auto& funaSphere : arg_nowStage.lock()->GetFunaCamPtArray()) {

		//�܂��͋����œ����蔻��
		float distance = Vec3<float>(funaSphere.m_pos - m_transform.GetPos()).Length();
		if (!(distance <= funaSphere.m_radius * 1.5f)) {
			continue;
		}

		switch (static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode))
		{
		case CameraController::CAMERA_FUNA_MODE::NORMAL:


			break;
		case CameraController::CAMERA_FUNA_MODE::STAGE1_1_C:

			//�v���C���[�̏�x�N�g����Y������k�������Ă����瓖�����Ă��Ȃ��B
			if (fabs(m_transform.GetUp().y) < 0.9f) {

				isHitFuna = true;

				//������ID������B
				m_cameraFunaMode = static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode);

			}

			break;
		case CameraController::CAMERA_FUNA_MODE::STAGE1_2_INV_U:
		{

			//�v���C���[��Z�����W��50��艺��Y�������̖@���������Ă������߂�B
			if (0.9f < fabs(m_transform.GetUp().y)) {

				if (m_transform.GetPos().z < 25.0f) {
					break;
				}

				if (m_transform.GetPos().y < -100.0f) {
					break;
				}

			}

			if (static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode) != CameraController::CAMERA_FUNA_MODE::STAGE1_2_INV_U) {
				m_cameraRotYStorage = fabs(fmod(m_cameraRotYStorage, DirectX::XM_2PI));
			}

			isHitFuna = true;

			//������ID������B
			m_cameraFunaMode = static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode);

		}
		break;
		case CameraController::CAMERA_FUNA_MODE::STAGE1_3_HEIGHT_LIMIT:

			isHitFuna = true;

			//������ID������B
			m_cameraFunaMode = static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode);

			break;
		case CameraController::CAMERA_FUNA_MODE::STAGE1_4_FAR:

			isHitFuna = true;

			//������ID������B
			m_cameraFunaMode = static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode);

			break;
		//case CameraController::CAMERA_FUNA_MODE::STAGE1_6_NEAR_GOAL:

		//	isHitFuna = true;

		//	if (static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode) != CameraController::CAMERA_FUNA_MODE::STAGE1_6_NEAR_GOAL) {
		//		m_cameraRotYStorage = fabs(fmod(m_cameraRotYStorage, DirectX::XM_2PI));
		//	}

		//	//������ID������B
		//	m_cameraFunaMode = static_cast<CameraController::CAMERA_FUNA_MODE>(funaSphere.m_camMode);

		//	break;
		default:
			break;
		}

		//�������Ă����珈�����I����B
		if (isHitFuna) {
			break;
		}

	}
	//�������Ă������Ԃ�؂�ւ���B
	if (!isHitFuna) {
		m_cameraFunaMode = CameraController::CAMERA_FUNA_MODE::NORMAL;
	}

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

	//����ł����珈�����΂��B
	if (m_isDeath) return;

	//���G���B
	if (!m_nodamageTimer.IsTimeUp()) return;

	//HP�����炷�B
	m_hp = std::clamp(m_hp - 1, 0, std::numeric_limits<int>().max());

	SoundConfig::Instance()->Play(SoundConfig::SE_PLAYER_DAMAGE_HIT);

	//HPUI���o
	m_hpUi.OnDamage();

	//���񂾂�
	if (m_hp <= 0) {

		m_isDeath = true;

	}
	else {

		//�e��^�C�}�[��ݒ�B
		m_nodamageTimer.Reset(NODAMAGE_TIMER);
		m_damageHitStopTimer.Reset(DAMAGE_HITSTOP_TIMER);

		//�V�F�C�N��������B
		m_damageShakeAmount = DAMAGE_SHAKE_AMOUNT;

		//�v���C���[�̏�Ԃ��_���[�W����
		m_beforeDamageStatus = m_playerMoveStatus;
		m_playerMoveStatus = PLAYER_MOVE_STATUS::DAMAGE;
	}

	//�q�b�g�X�g�b�v
	TimeScaleMgr::s_inGame.Set(0.0f);

	m_damageFlashTimer.Reset();

	//�R���g���[���[�U��
	KuroEngine::UsersInput::Instance()->ShakeController(0, 1.0f, 10);
}

Player::CHECK_HIT_GRASS_STATUS Player::CheckHitGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize)
{

	//�������v�Z�B
	float distance = (arg_enemyPos - m_transform.GetPosWorld()).Length();

	//��������肢�Ȃ���������G���߂��ɂ��锻��ɂ��ăJ�������߂Â���B
	if (distance <= CAMERA_NEAR_ENEMY_DISTANCE && 0.9f < m_transform.GetUp().Dot(arg_enemyUp)) {
		m_cameraReturnTimer.Reset(CAMERA_RETURN_TIMER);
	}

	//�U����Ԃ���Ȃ������珈����߂��B
	if (!GetIsAttack()) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�_���[�W���󂯂Ȃ���Ԃ������瓖���蔻����΂��B
	if (!m_damageHitStopTimer.IsTimeUp()) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�܂��͋��̔���
	bool isHit = distance < std::clamp(m_growPlantPtLig.m_influenceRange, PLAYER_HEAD_SIZE, m_growPlantPtLig.m_defInfluenceRange) + arg_enemySize;

	//�������Ă��Ȃ������珈�����΂��B
	if (!isHit) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�v���C���[����G�܂ł̃x�N�g���ƓG�̏�x�N�g������ς��āA���̌��ʂ�0�ȉ��������烉�C�g���������Ă��锻��(�����������Ă��锻��B)
	float dot = (arg_enemyPos - m_transform.GetPosWorld()).GetNormal().Dot(arg_enemyUp);
	bool isLight = dot < -0.1f;

	//���C�g�ɓ������Ă��锻��
	if (!isLight) {

		//�����ɂ���ē��ɓ������Ă��邩�𔻒f
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

void Player::Move(KuroEngine::Vec3<float>& arg_newPos) {

	//�������͓��͂𖳌����B
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

	//�ړ��ʂ���]������
	auto accel = KuroEngine::Math::TransformVec3(m_rowMoveVec, m_transform.GetRotate()) * accelSpeed;


	m_moveSpeed += accel;

	//�ړ����x����
	if (maxSpeed < m_moveSpeed.Length())
	{
		m_moveSpeed = m_moveSpeed.GetNormal() * maxSpeed;
	}

	//�ړ��ʉ��Z
	arg_newPos += m_moveSpeed * TimeScaleMgr::s_inGame.GetTimeScale();

	//�M�~�b�N�̈ړ��ʂ����Z�B
	arg_newPos += m_gimmickVel;

	//�n�ʂɒ���t����p�̏d�́B
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

	//�����Ă��āA�n����Ԑ؂�ւ�������Ȃ�������B
	const float PARTICLE_DEADLINE = 0.5f;
	if (PARTICLE_DEADLINE < m_moveSpeed.Length() && 1.0f <= m_underGroundEaseTimer) {

		//�v���C���[�����������̃p�[�e�B�N���𐶐��B
		m_playerMoveParticleTimer.UpdateTimer();
		if (m_playerMoveParticleTimer.IsTimeUpOnTrigger()) {
			//�n���ɂ��邩��������Ȃ����Ńp�[�e�B�N����ς���B
			if (m_isUnderGround) {
				//���p�[�e�B�N���B
				for (int index = 0; index < PLAYER_MOVE_PARTICLE_COUNT; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(SMOKE_SCATTER));
				}
				//�I�[�u��������Ƃ����o���B
				for (int index = 0; index < 2; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
				}
			}
			else {
				//�I�[�u���o���B
				for (int index = 0; index < PLAYER_MOVE_PARTICLE_COUNT; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateOrb(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_influenceRange));
				}
				//����������Ƃ����o���B
				for (int index = 0; index < 2; ++index) {
					KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

					const float SMOKE_SCATTER = 5.0f;
					m_playerMoveParticle.GenerateSmoke(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_defInfluenceRange));
				}
			}
			m_playerMoveParticleTimer.Reset();
		}

		//�ړ����Ă���Ƃ��̓V�F�C�N������B
		if (m_isUnderGround) {
			KuroEngine::UsersInput::Instance()->ShakeController(0, 0.2f, 10);
		}

	}
	//�����Ă��Ȃ��Ƃ����K�ʂ̃p�[�e�B�N�����o���B
	else {

		m_playerIdleParticleTimer.UpdateTimer();
		if (m_playerIdleParticleTimer.IsTimeUpOnTrigger()) {

			//�I�[�u���o���B
			for (int index = 0; index < 1; ++index) {
				KuroEngine::Vec3<float> scatterVec = KuroEngine::GetRand(KuroEngine::Vec3<float>(-1, -1, -1), KuroEngine::Vec3<float>(1, 1, 1));

				const float SMOKE_SCATTER = 5.0f;
				m_playerMoveParticle.GenerateIdle(m_transform.GetPos(), scatterVec.GetNormal() * KuroEngine::GetRand(m_growPlantPtLig.m_influenceRange));
			}

			m_playerIdleParticleTimer.Reset();

		}

	}

	//����
	if (m_rowMoveVec.IsZero())
		m_moveSpeed = KuroEngine::Math::Lerp(m_moveSpeed, KuroEngine::Vec3<float>(0.0f, 0.0f, 0.0f), brake);
}

void Player::UpdateZipline() {

	switch (m_gimmickStatus)
	{
	case Player::GIMMICK_STATUS::APPEAR:
	{

		//�W�b�v���C���̒��ɓ����Ă����^�C�}�[���X�V
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_START);

		//�C�[�W���O�̗ʂ����߂�B
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_START);

		//�ړ��ʂ̃C�[�W���O
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//�ړ�������B
		m_transform.SetPos(m_zipInOutPos + (m_refZipline.lock()->GetPoint(true) - m_zipInOutPos) * moveEaseRate);

		//�X�P�[���̃C�[�W���O
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//����������B
		m_transform.SetScale(1.0f - scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_START <= m_ziplineMoveTimer) {

			//�W�b�v���C���𓮂����B
			m_refZipline.lock()->CanMovePlayer();

			//NORMAL�ɂ��ăv���C���[�͉������Ȃ��悤�ɂ���B
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

		//�W�b�v���C���̒��ɓ����Ă����^�C�}�[���X�V
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_END);

		//�C�[�W���O�̗ʂ����߂�B
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_END);

		//�ړ��ʂ̃C�[�W���O
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//�ړ�������B
		m_transform.SetPos(m_refZipline.lock()->GetPoint(false) + (m_zipInOutPos - m_refZipline.lock()->GetPoint(false)) * moveEaseRate);

		//�X�P�[���̃C�[�W���O
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//����������B
		m_transform.SetScale(scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_END <= m_ziplineMoveTimer) {

			//�v���C���[�����ɖ߂��B
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

			m_ziplineMoveTimer = 0;

		}

	}
	break;
	default:
		break;
	}

}

bool Player::CheckInGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize)
{

	//�������v�Z�B
	float distance = (arg_enemyPos - m_transform.GetPosWorld()).Length();

	//�܂��͋��̔���
	bool isHit = distance < std::clamp(m_growPlantPtLig.m_influenceRange, PLAYER_HEAD_SIZE, m_growPlantPtLig.m_defInfluenceRange) + arg_enemySize;

	//�������Ă��Ȃ������珈�����΂��B
	if (!isHit) {
		return false;
	}

	//�v���C���[����G�܂ł̃x�N�g���ƓG�̏�x�N�g������ς��āA���̌��ʂ�0�ȉ��������烉�C�g���������Ă��锻��(�����������Ă��锻��B)
	bool isLight = (arg_enemyPos - m_transform.GetPosWorld()).GetNormal().Dot(arg_enemyUp) < 0.1f;

	//���C�g�ɓ������Ă��锻��
	if (!isLight) {

		return true;

	}
	else {

		return true;

	}

}

void Player::UpdateDeath() {

	switch (m_deathStatus)
	{
	case Player::DEATH_STATUS::APPROACH:
	{
		//�J�������߂Â���B
		m_deathEffectCameraZ += (DEATH_EFFECT_CAMERA_Z - m_deathEffectCameraZ) / 2.0f;

		//���x��x������B
		TimeScaleMgr::s_inGame.Set(DEATH_EFFECT_TIMER_SCALE);

		++m_deathEffectTimer;
		if (DEATH_EFFECT_APPROACH_TIMER <= m_deathEffectTimer) {
			m_deathStatus = DEATH_STATUS::STAY;

			//���S���o�̃^�C�}�[���������B
			m_deathEffectTimer = 0;
		}
	}
	break;
	case Player::DEATH_STATUS::STAY:
	{

		++m_deathEffectTimer;
		if (DEATH_EFFECT_STAY_TIMER <= m_deathEffectTimer) {
			m_deathStatus = DEATH_STATUS::LEAVE;

			//���S���o�̃^�C�}�[���������B
			m_deathEffectTimer = 0;

			//�V�F�C�N��������B
			m_deathShakeAmount = DEATH_SHAKE_AMOUNT;

			SoundConfig::Instance()->Play(SoundConfig::SE_GAME_OVER);
		}

	}
	break;
	case Player::DEATH_STATUS::LEAVE:
	{
		//�J�����𗣂��B
		m_deathEffectCameraZ += (CAMERA_FAR - m_deathEffectCameraZ) / 5.0f;

		//���x�����ɖ߂��B
		TimeScaleMgr::s_inGame.Set(1.0f);

		//�V�F�C�N�ʂ��ւ炷�B
		m_deathShakeAmount = std::clamp(m_deathShakeAmount - SUB_DEATH_SHAKE_AMOUNT, 0.0f, 100.0f);

		//���S���o�̃A�j���[�V�������X�V�B
		m_deathSpriteAnimTimer.UpdateTimer(1.0f);

		//�^�C�}�[���I�������B
		if (m_deathSpriteAnimTimer.IsTimeUpOnTrigger()) {

			//���̃A�j���[�V��������������
			if (m_deathSpriteAnimNumber < DEATH_SPRITE_ANIM_COUNT - 1) {

				++m_deathSpriteAnimNumber;
				m_deathSpriteAnimTimer = KuroEngine::Timer(DEATH_SPRITE_TIMER);

			}

		}

		//�X�P�[�������������Ă����B
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
	//�q�b�g�X�g�b�v�̃^�C�}�[�I��
	if (m_damageHitStopTimer.UpdateTimer())
	{
		//�ʏ�̃^�C���X�P�[���ɖ߂�
		TimeScaleMgr::s_inGame.Set(1.0f);

		//�X�e�[�^�X�����ɖ߂��B
		m_playerMoveStatus = m_beforeDamageStatus;

		//�ꉞ�V�F�C�N�ʂ�0�ɂ��Ă����B
		m_damageShakeAmount = 0;

		//�_���[�W�_�ŊJ�n
		m_damageFlash = true;
		m_damageFlashTimer.Reset();

		//�R���g���[���[�U��
		KuroEngine::UsersInput::Instance()->ShakeController(0, 1.0f, 20);

		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_PLAYER_DAMAGE);
	}
	else
	{
		//�V�F�C�N�ʂ��ւ炷�B
		m_damageShakeAmount = std::clamp(m_damageShakeAmount - SUB_DAMAGE_SHAKE_AMOUNT, 0.0f, 100.0f);
	}
}