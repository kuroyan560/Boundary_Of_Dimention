#include "CheckPointPillar.h"
#include "../../../../src/engine/FrameWork/Importer.h"
#include "../Stage/StageManager.h"
#include "../Graphics/BasicDraw.h"
#include "../TimeScaleMgr.h"
#include "../Stage/CheckPointHitFlag.h"
#include "../../../../src/engine/FrameWork/UsersInput.h"

CheckPointPillar::CheckPointPillar()
{

	m_pillarModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/Stage/", "PillarLight.glb");
	m_isDraw = true;
	m_status = NORMAL;
	m_appearModeTimer.Reset(APPEAR_MODE_TIMER);
	m_exitModeTimer.Reset(EXIT_MODE_TIMER);
	m_isFirstFrame = false;

}

void CheckPointPillar::Init()
{

	m_isDraw = true;
	m_status = NORMAL;
	m_appearModeTimer.Reset(APPEAR_MODE_TIMER);
	m_exitModeTimer.Reset(EXIT_MODE_TIMER);
	m_isFirstFrame = false;

}

void CheckPointPillar::Update(const KuroEngine::Vec3<float>& arg_playerPos)
{

	//���̃`�F�b�N�|�C���g�̏����擾
	KuroEngine::Transform nextCheckPointTransform;
	StageManager::Instance()->GetNowMapPinTransform(&nextCheckPointTransform);

	//���̂܂܂̍��W��ۑ��B
	KuroEngine::Vec3<float> rawPos = nextCheckPointTransform.GetPosWorld();

	//���W��ۑ��B
	if (!m_isFirstFrame) {

		//���W���I�t�Z�b�g�����ĕۑ��B
		const float OFFSET = 20000.0f;
		m_transform.SetPos(nextCheckPointTransform.GetPosWorld() - nextCheckPointTransform.GetUp() * OFFSET);
		m_transform.SetRotate(nextCheckPointTransform.GetRotate());

	}

	//UV�A�j���[�V����

	for (auto& mesh : m_pillarModel->m_meshes)
	{
		for (auto& vertex : mesh.mesh->vertices)
		{
			vertex.uv.x += 0.01f * TimeScaleMgr::s_inGame.GetTimeScale();
		}
		mesh.mesh->Mapping();
	}

	//�ޏo�A�o������Ƃ��̍L�����Ă�X�P�[��
	const float SCALE_EXIT = 20.0f;
	const float SCALE_DEFAULT = 1.0f;

	switch (m_status)
	{
	case CheckPointPillar::NORMAL:
	{

		//�A���t�@�������鋗��
		const float ALPHA_DEADLINE = 100.0f;
		float distance = KuroEngine::Vec3<float>(arg_playerPos - rawPos).Length();

		//�߂Â��Ă���Ƃ��̓A���t�@��������B
		if (distance < ALPHA_DEADLINE) {

			m_alpha = KuroEngine::Math::Lerp(m_alpha, 0.0f, 0.2f);

		}
		else {

			m_alpha = KuroEngine::Math::Lerp(m_alpha, 1.0f, 0.08f);

		}

		//�X�P�[�����Z�b�g�B
		m_transform.SetScale(KuroEngine::Vec3<float>(1.0f, 100.0f, 1.0f));

		//�`�F�b�N�|�C���g�ɓ��������u�Ԃ�������APPEAR�̏������s���Z���B
		if (CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger || KuroEngine::UsersInput::Instance()->KeyInput(DIK_P)) {
			m_status = STATUS::EXIT;
			m_appearModeTimer.Reset();
			m_exitModeTimer.Reset();
		}

	}

	break;
	case CheckPointPillar::EXIT:
	{

		//�ޏo��Ԃł̍X�V����
		m_exitModeTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

		//�X�P�[���ƃA���t�@��ύX�B
		float timeRate = m_exitModeTimer.GetTimeRate(1.0f);
		float scaleRate = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, timeRate, 0.0f, 1.0f);

		m_alpha = 1.0f - KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Exp, timeRate, 0.0f, 1.0f);
		m_transform.SetScale(KuroEngine::Vec3<float>(SCALE_DEFAULT + scaleRate * (SCALE_EXIT - SCALE_DEFAULT), 100.0f, SCALE_DEFAULT + scaleRate * (SCALE_EXIT - SCALE_DEFAULT)));

		//�^�C�}�[����莞�Ԍo�߂����玟��
		if (m_exitModeTimer.IsTimeUp()) {

			m_exitModeTimer.Reset();
			m_appearModeTimer.Reset();
			m_status = CheckPointPillar::APPEAR;

			//���W���I�t�Z�b�g�����ĕۑ��B
			const KuroEngine::Vec3<float> OFFSET = KuroEngine::Vec3<float>(0, 20000.0f, 0);
			m_transform.SetPos(nextCheckPointTransform.GetPosWorld() - OFFSET);

		}

	}

	break;
	case CheckPointPillar::APPEAR:
	{

		//�o����Ԃł̍X�V����
		m_appearModeTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

		//�X�P�[���ƃA���t�@��ύX�B
		float timeRate = m_appearModeTimer.GetTimeRate(1.0f);
		float scaleRate = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, timeRate, 0.0f, 1.0f);

		m_alpha = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Sine, timeRate, 0.0f, 1.0f);
		m_transform.SetScale(KuroEngine::Vec3<float>(SCALE_EXIT - scaleRate * (SCALE_EXIT - SCALE_DEFAULT), 100.0f, SCALE_EXIT - scaleRate * (SCALE_EXIT - SCALE_DEFAULT)));

		//�^�C�}�[����莞�Ԍo�߂����玟��
		if (m_appearModeTimer.IsTimeUp()) {

			m_appearModeTimer.Reset();
			m_exitModeTimer.Reset();
			m_status = CheckPointPillar::NORMAL;

		}

	}

	break;
	default:
		break;
	}

	m_isFirstFrame = true;
}

void CheckPointPillar::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::DepthStencil>arg_ds)
{

	//�`�悷���Ԃ�������`��
	if (m_isDraw) {

		auto param = IndividualDrawParameter::GetDefault();
		param.m_alpha = m_alpha;
		BasicDraw::Instance()->Draw_NoOutline(arg_ds, arg_cam, arg_ligMgr, m_pillarModel, m_transform, param, KuroEngine::AlphaBlendMode_Trans);

	}

}
