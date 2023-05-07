#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/UsersInput.h"
#include"../SoundConfig.h"
#include"PlayerCollision.h"
#include"../TimeScaleMgr.h"
#include"DirectX12/D3D12App.h"

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

	//�ړ�
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Move")) {

		ImGui::DragFloat("MoveAccel", &m_moveAccel, 0.01f);
		ImGui::DragFloat("MaxSpeed", &m_maxSpeed, 0.01f);
		ImGui::DragFloat("Brake", &m_brake, 0.01f);

		ImGui::TreePop();
	}

	//�J����
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
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

	m_growPlantPtLig.Register();
	//���S���o�̃^�C�}�[���������B
	m_deathEffectTimer = 0;
	m_deathShakeAmount = 0;
	m_deathShake = KuroEngine::Vec3<float>();
	m_deathStatus = DEATH_STATUS::APPROACH;
	m_isFinishDeathAnimation = false;

	//�n���ɒ��ފ֘A
	m_isInputUnderGround = false;
	m_isUnderGround = false;
	m_underGroundEaseTimer = 1.0f;

	m_deathSpriteAnimNumber = 0;
	m_deathSpriteAnimTimer = KuroEngine::Timer(DEATH_SPRITE_TIMER);

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

	//�X�e�[�W��ۑ��B
	m_stage = arg_nowStage;

	//�ʒu���֌W
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove() * TimeScaleMgr::s_inGame.GetTimeScale();

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

			m_isInputUnderGround = UsersInput::Instance()->KeyInput(DIK_SPACE) || UsersInput::Instance()->ControllerInput(0, KuroEngine::A);

			//�C�[�W���O���I����Ă��鎞�̂ݒn���ɐ�������o���肷�锻�����������B
			bool isInputOnOff = UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || UsersInput::Instance()->KeyOffTrigger(DIK_SPACE) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::A) || UsersInput::Instance()->ControllerOffTrigger(0, KuroEngine::A);
			if (isInputOnOff || (!m_isUnderGround && m_isInputUnderGround) || (m_isUnderGround && !m_isInputUnderGround)) {
				m_underGroundEaseTimer = 0;
			}

		}
		else {

			m_underGroundEaseTimer = std::clamp(m_underGroundEaseTimer + ADD_UNDERGROUND_EASE_TIMER, 0.0f, 1.0f);

			if (1.0f <= m_underGroundEaseTimer) {
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

		//�ړ����Ă��Ȃ�������������]�t���O��؂�ւ���B
		if (m_rowMoveVec.Length() <= 0) {

			//�~�܂��Ă���Ƃ��͔��]���邩�ǂ����̏��������A���^�C���Ōv�Z����B
			if (0.9f < m_transform.GetUp().y) {

				//�v���C���[�̖@���ƃJ�����܂ł̋�������AZ������(���[�J��)�̈ړ������𔽓]�����邩�����߂�B
				KuroEngine::Vec3<float> cameraDir = m_camController.GetCamera().lock()->GetTransform().GetPosWorld() - m_transform.GetPos();
				cameraDir.Normalize();
				if (m_transform.GetUp().Dot(cameraDir) < 0.1f) {
					m_isCameraInvX = true;
					m_rowMoveVec.z *= -1.0f;
				}
				else {
					m_isCameraInvX = false;
				}

			}
			//�V��ɂ�����
			else if (m_transform.GetUp().y < -0.9f) {
				//X�̈ړ������𔽓]
				m_rowMoveVec.x *= -1.0f;

				//�v���C���[�̖@���ƃJ�����܂ł̋�������AZ������(���[�J��)�̈ړ������𔽓]�����邩�����߂�B
				KuroEngine::Vec3<float> cameraDir = m_camController.GetCamera().lock()->GetTransform().GetPosWorld() - m_transform.GetPos();
				cameraDir.Normalize();
				if (-0.1f < m_transform.GetUp().Dot(cameraDir)) {
					m_isCameraInvX = true;
					m_rowMoveVec.z *= -1.0f;
				}
				else {
					m_isCameraInvX = false;
				}

			}
			else {
				m_isCameraInvX = false;
			}
		}
		else {

			if (0.9f < m_transform.GetUp().y) {

				//�v���C���[�̖@���ƃJ�����܂ł̋�������AZ������(���[�J��)�̈ړ������𔽓]�����邩�����߂�B
				if (m_isCameraInvX) {
					m_rowMoveVec.z *= -1.0f;
				}

			}
			//�V��ɂ�����
			else if (m_transform.GetUp().y < -0.9f) {
				//X�̈ړ������𔽓]
				m_rowMoveVec.x *= -1.0f;

				if (m_isCameraInvX) {
					m_rowMoveVec.z *= -1.0f;
				}

			}
			else if (m_isCameraInvX) {
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
	default:
		break;
	}


	//���W�ω��K�p
	m_ptLig.SetPos(newPos);

	//�M�~�b�N�̈ړ���ł������B
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();

	//�n���ɂ���Ƃ��̓��C�g��ς���B
	if (m_isInputUnderGround) {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange - SUB_INFLUENCE_RANGE, MIN_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);
	}
	else {
		m_growPlantPtLig.m_influenceRange = std::clamp(m_growPlantPtLig.m_influenceRange + ADD_INFLUENCE_RANGE, ATTACK_INFLUENCE_RANGE, MAX_INFLUENCE_RANGE);
	}
	m_growPlantPtLig.m_defInfluenceRange = MAX_INFLUENCE_RANGE;

	//����ł����玀�S�̍X�V����������B
	if (!m_isDeath) {
		//�J��������	//����ł����玀��ł����Ƃ��̃J�����̏����ɕς���̂ŁA�����̏������ɓ����B
		m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, CAMERA_MODE[m_cameraMode]);
		m_deathEffectCameraZ = CAMERA_MODE[m_cameraMode];
		TimeScaleMgr::s_inGame.Set(1.0f);
	}
	else {

		//�V�F�C�N�̕���߂��B
		m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() - m_deathShake);

		m_playerMoveStatus = PLAYER_MOVE_STATUS::DEATH;
		m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, m_deathEffectCameraZ);

		//�V�F�C�N���v�Z�B
		m_deathShake.x = KuroEngine::GetRand(-m_deathShakeAmount, m_deathShakeAmount);
		m_deathShake.y = KuroEngine::GetRand(-m_deathShakeAmount, m_deathShakeAmount);
		m_deathShake.z = KuroEngine::GetRand(-m_deathShakeAmount, m_deathShakeAmount);

		//�V�F�C�N��������B
		m_camController.GetCamera().lock()->GetTransform().SetPos(m_camController.GetCamera().lock()->GetTransform().GetPos() + m_deathShake);
	}

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
	m_drawTransform.SetScale(m_transform.GetScale());
	//��]�͓������Ƃ��̂ݓK�p������B
	if (0 < m_rowMoveVec.Length()) {
		m_drawTransform.SetRotate(m_transform.GetRotate());
	}

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
		m_drawTransform,
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
			arg_cam);
	}
}

void Player::DrawUI()
{
	//����ł��� ���� �A�j���[�V�������I����Ă��Ȃ�������
	bool isFinishAnimation = m_deathSpriteAnimNumber == DEATH_SPRITE_ANIM_COUNT && m_deathSpriteAnimTimer.IsTimeUp();
	if (m_deathStatus == Player::DEATH_STATUS::LEAVE && !isFinishAnimation) {

		KuroEngine::Vec2<float> winCenter = KuroEngine::Vec2<float>(1280.0f / 2.0f, 720.0f / 2.0f);
		KuroEngine::Vec2<float> spriteSize = KuroEngine::Vec2<float>(512.0f, 512.0f);

		//KuroEngine::DrawFunc2D::DrawExtendGraph2D(winCenter - spriteSize, winCenter + spriteSize, m_deathAnimSprite[m_deathSpriteAnimNumber]);

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

	//�������͓��͂𖳌����B
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed = m_rowMoveVec * m_maxSpeed;

	//�ړ����x���N�����v�B
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//���͂��ꂽ�l������������ړ����x�����炷�B
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = 0;

	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = 0;

	}

	//���[�J�����̈ړ��������v���C���[�̉�]�ɍ��킹�ē������B
	auto moveAmount = KuroEngine::Math::TransformVec3(m_moveSpeed, m_transform.GetRotate());

	//�ړ��ʉ��Z
	arg_newPos += moveAmount * TimeScaleMgr::s_inGame.GetTimeScale();

	//�M�~�b�N�̈ړ��ʂ����Z�B
	arg_newPos += m_gimmickVel;

	//�n�ʂɒ���t����p�̏d�́B
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

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