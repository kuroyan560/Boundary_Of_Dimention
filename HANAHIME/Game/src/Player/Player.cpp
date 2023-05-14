#include "Player.h"
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
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE)
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
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
	AddCustomParameter("Default_AccelSpeed", { "move","default","accelSpeed" }, PARAM_TYPE::FLOAT, &m_defaultAccelSpeed, "Move");
	AddCustomParameter("Default_MaxSpeed", { "move","default","maxSpeed" }, PARAM_TYPE::FLOAT, &m_defaultMaxSpeed, "Move");
	AddCustomParameter("Default_Brake", { "move","default","brake" }, PARAM_TYPE::FLOAT, &m_defaultBrake, "Move");
	AddCustomParameter("UnderGround_AccelSpeed", { "move","underGround","accelSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundAccelSpeed, "Move");
	AddCustomParameter("UnderGround_MaxSpeed", { "move","underGround","maxSpeed" }, PARAM_TYPE::FLOAT, &m_underGroundMaxSpeed, "Move");
	AddCustomParameter("UnderGround_Brake", { "move","underGround","brake" }, PARAM_TYPE::FLOAT, &m_underGroundBrake, "Move");
	AddCustomParameter("HpCenterPos", { "ui","hp","centerPos" }, PARAM_TYPE::FLOAT_VEC2, &m_hpCenterPos, "UI");
	LoadParameterLog();

	//���f���ǂݍ���
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//�J��������
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//�J�����̃R���g���[���[�ɃA�^�b�`
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

	m_collision.m_refPlayer = this;

	//���S�A�j���[�V������ǂݍ���
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_deathAnimSprite.data(), "resource/user/tex/Number.png", DEATH_SPRITE_ANIM_COUNT, KuroEngine::Vec2<int>(DEATH_SPRITE_ANIM_COUNT, 1));

	//�A�j���[�^�[����
	m_modelAnimator = std::make_shared<KuroEngine::ModelAnimator>(m_model);

	m_hpTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/hp_leaf.png");
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_initTransform = arg_initTransform;
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
	m_cameraMode = 1;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_canZip = false;
	m_isCameraInvX = false;
	m_canUnderGroundRelease = true;
	m_canOldUnderGroundRelease = true;
	m_onCeiling = false;
	m_isCameraUpInverse = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

	m_growPlantPtLig.Register();
	//���S���o�̃^�C�}�[���������B
	m_deathEffectTimer = 0;
	m_deathShakeAmount = 0;
	m_damageShakeAmount = 0;
	m_shake = KuroEngine::Vec3<float>();
	m_deathStatus = DEATH_STATUS::APPROACH;
	m_isFinishDeathAnimation = false;

	//�n���ɒ��ފ֘A
	m_isInputUnderGround = false;
	m_isUnderGround = false;
	m_underGroundEaseTimer = 1.0f;

	m_attackTimer = 0;

	m_deathSpriteAnimNumber = 0;
	m_deathSpriteAnimTimer = KuroEngine::Timer(DEATH_SPRITE_TIMER);

	m_hp = DEFAULT_HP;
	m_damageHitstopTimer = 0;
	m_nodamageTimer = 0;;

	m_modelAnimator->Play(m_animNames[ANIM_PATTERN_WAIT], true, false);
	m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;
	m_beforePlayerMoveStatus = m_playerMoveStatus;

	//�v���C���[�̋��̔���
	m_sphere.m_centerPos = &m_drawTransform.GetPos();
	m_sphere.m_radius = &m_radius;
	m_radius = 2.0f;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

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
	if (m_onCeiling) {
		scopeMove *= -1.0f;
	}

	//�W�����v���ł��邩�ǂ����B	��莞�Ԓn�`�Ɉ����|�����Ă���W�����v�ł���B
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//�J�������[�h��؂�ւ���B
	if (UsersInput::Instance()->KeyOffTrigger(DIK_RETURN) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::X)) {
		++m_cameraMode;
		if (static_cast<int>(CAMERA_MODE.size()) <= m_cameraMode) {
			m_cameraMode = 0;
		}

		//SE��炷�B
		SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, m_cameraMode);
	}

	//�ړ��X�e�[�^�X�ɂ���ď�����ς���B
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//�W�b�v���C��
		m_canZip = UsersInput::Instance()->KeyOnTrigger(DIK_LSHIFT);

		//�n���ɒ��ރt���O���X�V�B �C�[�W���O���I����Ă�����B
		if (1.0f <= m_underGroundEaseTimer) {

			bool prevInInputUnderGround = m_isInputUnderGround;
			m_isInputUnderGround = UsersInput::Instance()->KeyInput(DIK_SPACE) || UsersInput::Instance()->ControllerInput(0, KuroEngine::A);

			//���ރt���O�������ꂽ�g���K�[��������B
			if ((prevInInputUnderGround && !m_isInputUnderGround) || (!m_canOldUnderGroundRelease && m_canUnderGroundRelease)) {

				//�U������B
				m_attackTimer = ATTACK_TIMER;
			}

			//�C�[�W���O���I����Ă��鎞�̂ݒn���ɐ�������o���肷�锻�����������B
			bool isInputOnOff = UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || UsersInput::Instance()->KeyOffTrigger(DIK_SPACE) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::A) || UsersInput::Instance()->ControllerOffTrigger(0, KuroEngine::A);
			if ((isInputOnOff || (!m_isUnderGround && m_isInputUnderGround) || (m_isUnderGround && !m_isInputUnderGround)) && m_canUnderGroundRelease) {
				m_underGroundEaseTimer = 0;
			}

		}
		else {

			m_underGroundEaseTimer = std::clamp(m_underGroundEaseTimer + ADD_UNDERGROUND_EASE_TIMER, 0.0f, 1.0f);

			if (1.0f <= m_underGroundEaseTimer) {

				//�n���ɂ��锻����X�V�B
				m_isUnderGround = m_isInputUnderGround;
			}

		}

		//�v���C���[�̉�]���J������ɂ���B(�ړ������̊���J�����̊p�x�Ȃ���)
		m_transform.SetRotate(m_cameraQ);

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
			if (m_isCameraInvX || (m_isCameraUpInverse && fabs(m_transform.GetUp().y) < 0.1f)) {

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
	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

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

			//�ʈړ�SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_SURFACE_JUMP);

		}
		m_transform.SetPos(newPos);

		//�W�����v���͏펞��]��K�p������B
		m_drawTransform.SetRotate(m_transform.GetRotate());

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
	default:
		break;
	}

	//���W�ω��K�p
	m_ptLig.SetPos(newPos);

	//�M�~�b�N�̈ړ���ł������B
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();

	//�n���ɂ���Ƃ��̓��C�g��ς���B
	if (m_isInputUnderGround || !m_canUnderGroundRelease) {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange - SUB_INFLUENCE_RANGE, MIN_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);
	}
	else {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange + ADD_INFLUENCE_RANGE, ATTACK_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);
	}
	m_growPlantPtLig.m_defInfluenceRange = MAX_INFLUENCE_RANGE;

	//����ł����玀�S�̍X�V����������B
	if (!m_isDeath) {
		//�J��������	//����ł����玀��ł����Ƃ��̃J�����̏����ɕς���̂ŁA�����̏������ɓ����B
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, CAMERA_MODE[m_cameraMode], arg_nowStage, m_isCameraUpInverse);

		m_deathEffectCameraZ = CAMERA_MODE[m_cameraMode];
	}
	else {

		//�V�F�C�N�̕���߂��B
		m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() - m_shake);

		m_playerMoveStatus = PLAYER_MOVE_STATUS::DEATH;
		m_camController.Update(scopeMove, m_transform, m_cameraRotYStorage, m_deathEffectCameraZ, arg_nowStage, m_isCameraUpInverse);

	}
	//�V�F�C�N���v�Z�B
	float timeScaleShakeAmount = m_deathShakeAmount * TimeScaleMgr::s_inGame.GetTimeScale() + m_damageShakeAmount;
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
	m_nodamageTimer = std::clamp(m_nodamageTimer - 1.0f * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, NODAMAGE_TIMER);

	//�A�j���[�V�����w��
	AnimationSpecification(beforePos, newPos);

	//���f���̃A�j���[�^�[�X�V
	m_modelAnimator->Update(TimeScaleMgr::s_inGame.GetTimeScale());

	//�����̃X�e�[�^�X�L�^
	m_beforePlayerMoveStatus = m_playerMoveStatus;
	//�U�����^�C�}�[�����炷�B
	m_attackTimer = std::clamp(m_attackTimer - 1, 0, ATTACK_TIMER);

}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 1.0f, 0.0f);

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_drawTransform,
		edgeColor,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());


	//KuroEngine::DrawFunc3D::DrawNonShadingModel(
	//	m_axisModel,
	//	m_drawTransform,
	//	arg_cam);


	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			arg_cam);
	}
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

	//�ő�HP����z�u�̊p�x�I�t�Z�b�g�����߂�
	const Angle angleOffset = Angle::ROUND() / DEFAULT_HP;

	//�E�B���h�E�̃T�C�Y�擾
	const auto winSize = WinApp::Instance()->GetExpandWinSize();

	//HPUI�̒��S���W
	//const auto hpCenterPos = ConvertWorldToScreen(m_transform.GetPosWorld(), arg_cam.GetViewMat(), arg_cam.GetProjectionMat(), winSize);

	//HPUI�摜�̊g�嗦
	const Vec2<float>hpTexExpand = { 1.2f,1.2f };

	//HPUI�~�̔��a
	const auto hpRadius = m_hpTex->GetGraphSize().y * 0.5f * hpTexExpand.y;

	//�v���C���[�̂QD���W
	for (int hpIdx = m_hp - 1; 0 <= hpIdx; --hpIdx)
	{
		auto pos = m_hpCenterPos;
		Angle angle = angleOffset * hpIdx - Angle::ConvertToRadian(90);
		pos.x += cos(angle) * hpRadius;
		pos.y += sin(angle) * hpRadius;
		DrawFunc2D::DrawRotaGraph2D(pos, hpTexExpand, angle + Angle(90), m_hpTex);
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

void Player::Damage()
{

	//����ł����珈�����΂��B
	if (m_isDeath) return;

	//HP�����炷�B
	m_hp = std::clamp(m_hp - 1, 0, std::numeric_limits<int>().max());

	//���񂾂�
	if (m_hp <= 0) {

		m_isDeath = true;

	}
	else {

		//�e��^�C�}�[��ݒ�B
		m_nodamageTimer = NODAMAGE_TIMER;
		m_damageHitstopTimer = DAMAGE_HITSTOP_TIMER + DAMAGE_HITSTOP_RETURN_TIMER;

		//�V�F�C�N��������B
		m_damageShakeAmount = DAMAGE_SHAKE_AMOUNT;

		//�v���C���[�̏�Ԃ��_���[�W����
		m_beforeDamageStatus = m_playerMoveStatus;
		m_playerMoveStatus = PLAYER_MOVE_STATUS::DAMAGE;

	}


}

Player::CHECK_HIT_GRASS_STATUS Player::CheckHitGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize)
{

	//�U����Ԃ���Ȃ������珈����߂��B
	if (!GetIsAttack()) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�_���[�W���󂯂Ȃ���Ԃ������瓖���蔻����΂��B
	if (0 < m_damageHitstopTimer) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�܂��͋��̔���
	float distance = (arg_enemyPos - m_transform.GetPosWorld()).Length();
	bool isHit = distance < m_growPlantPtLig.m_influenceRange;

	//�������Ă��Ȃ������珈�����΂��B
	if (!isHit) {
		return Player::CHECK_HIT_GRASS_STATUS::NOHIT;
	}

	//�v���C���[����G�܂ł̃x�N�g���ƓG�̏�x�N�g������ς��āA���̌��ʂ�0�ȉ��������烉�C�g���������Ă��锻��(�����������Ă��锻��B)
	bool isLight = (arg_enemyPos - m_transform.GetPosWorld()).GetNormal().Dot(arg_enemyUp) < 0;

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
		}

	}
	break;
	case Player::DEATH_STATUS::LEAVE:
	{
		//�J�����𗣂��B
		m_deathEffectCameraZ += (CAMERA_MODE[1] - m_deathEffectCameraZ) / 5.0f;

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

	//�q�b�g�X�g�b�v�̃^�C�}�[���L����������
	if (0 < m_damageHitstopTimer) {


		//�q�b�g�X�g�b�v�^�C�}�[�̎c�莞�Ԃ�TimeScale��߂����߂̎��Ԃ��Z��������B
		if (m_damageHitstopTimer < DAMAGE_HITSTOP_RETURN_TIMER) {

			//�q�b�g�X�g�b�v�����ɖ߂��B
			TimeScaleMgr::s_inGame.Set(KuroEngine::Math::Lerp(TimeScaleMgr::s_inGame.GetTimeScale(), 1.0f, 0.9f));

		}
		else {

			//�q�b�g�X�g�b�v��������B
			TimeScaleMgr::s_inGame.Set(KuroEngine::Math::Lerp(TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 0.9f));

		}

		--m_damageHitstopTimer;

		//�V�F�C�N�ʂ��ւ炷�B
		m_damageShakeAmount = std::clamp(m_damageShakeAmount - SUB_DAMAGE_SHAKE_AMOUNT, 0.0f, 100.0f);

	}
	else {

		//�X�e�[�^�X�����ɖ߂��B
		m_playerMoveStatus = m_beforeDamageStatus;

		//�ꉞ�V�F�C�N�ʂ�0�ɂ��Ă����B
		m_damageShakeAmount = 0;

	}

}
