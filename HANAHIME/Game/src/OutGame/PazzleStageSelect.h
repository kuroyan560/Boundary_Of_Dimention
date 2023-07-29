#pragma once
#include"Common/Vec.h"
#include"Framework/UsersInput.h"
#include<vector>
#include<array>
#include"Select.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/Timer.h"
#include"../Movie/MovieCamera.h"
#include"../SoundConfig.h"
#include"../OperationConfig.h"

class GameScene;

/// <summary>
/// �^�C�g�����璼�ڃX�e�[�W��I������
/// </summary>
class PazzleStageSelect
{
public:
	PazzleStageSelect();

	void Init();
	void Update(std::shared_ptr<KuroEngine::Camera> arg_cam, GameScene* arg_gameScene);
	void Draw(KuroEngine::Camera& arg_cam);

	int GetNumber();
	int GetMaxNumber();
	bool IsEnableToSelect()
	{
		return m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].enableFlag;
	}
	bool IsEnableToDone()
	{
		bool flag = !m_previweFlag && !stop1FlameFlag;
		stop1FlameFlag = false;
		return flag;
	}
	void Stop()
	{
		m_stopFlag = true;
	}
	void Clear()
	{
		m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag = true;
	};
	bool Done()
	{
		//bool inputFlag = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::XBOX_BUTTON::A);
		bool inputFlag = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
		bool tmpDoneFlag = m_doneFlag;
		if (inputFlag && !m_doneFlag)
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
			m_doneFlag = true;
		}
		return inputFlag && !tmpDoneFlag && !m_stopFlag;
	};

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		if (!m_previweFlag)
		{
			return m_camera.GetCamera();
		}
		else
		{
			return m_previewCamera;
		}
	}

	void SetSelectStageNum(int arg_stageNum)
	{
		m_nowStageNum.x = arg_stageNum;
	}

	MovieCamera m_camera;
private:

	struct StageData
	{
		KuroEngine::Vec2<float> m_pos;
		bool m_isClearFlag;
		bool enableFlag;	//�V�ׂ�X�e�[�W���ǂ���
		StageData() :m_isClearFlag(false)
		{};
	};
	std::array<std::array<StageData, 30>, 1>m_stageSelectArray;
	std::array<std::array<KuroEngine::Vec2<float>, 30>, 1>m_basePos;
	std::array<std::array<KuroEngine::Vec2<float>, 30>, 1>m_nowPos;
	std::array<std::array<float, 30>, 1>m_baseAlpha;
	std::array<std::array<float, 30>, 1>m_nowAlpha;
	KuroEngine::Vec2<int> m_nowStageNum;
	int m_preStageNum;

	enum {
		LEFT = 0,
		RIGHT = 1,
	};

	//�����T�C���g�œ������p�̕ϐ�
	std::array<float, 2> m_arrowAlpha;
	std::array<float, 2> m_arrowSinTimer;						//�T�C���g��h�炷�ۂ̃^�C�}�[
	std::array<float, 2> m_arrowSinTimerAddNow;				//�T�C���g��h�炷���Z��
	std::array<float, 2> m_arrowSinTimerAddBase;				//�T�C���g��h�炷���Z�ʂ̕�Ԑ�
	const float ARROW_SINE_TIMER = 0.138f / 2.0f;	//�T�C���g��h�炷�^�C�}�[�ɉ��Z����� ���y�ɍ��킹�Ă���̂Ŕ����Ȓl�ɂȂ��Ă���B
	const float ARROW_SINE_TIMER_ADD = 0.5f;	//�T�C���g��h�炷�^�C�}�[�ɉ��Z����� ���y�ɍ��킹�Ă���̂Ŕ����Ȓl�ɂȂ��Ă���B
	std::array<float, 2> m_arrowSineLengthNow;
	std::array<float, 2> m_arrowSineLengthBase;
	const float ARROW_SINE_INIT_LENGTH = 10.0f;	//�T�C���g�̗h��̕��̏����l
	const float ARROW_SINE_INIT_LENGTH_ADD = 30.0f;	//�T�C���g�̗h��̕��̏����l

	//�J�����̃p�����[�^�[
	float m_cameraAngle;
	const float CAMERA_ANGLE_ADD = 0.005f;
	float m_cameraLength;
	const float DEF_CAMERA_LENGTH = 120.0f;
	const float FAR_CAMERA_LENGTH = 200.0f;


	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_numMainTexArray;
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_numSubTexArray;

	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_nowNumTexArray;


	std::vector<int> CountNumber(int TIME)
	{
		std::vector<int> Number(2);
		Number[0] = -1;
		Number[1] = -1;

		int tmp = TIME;
		//�X�R�A�v�Z
		for (int i = 0; tmp > 0; i++)
		{
			int result = tmp % 10;
			//Number.push_back(result);
			Number[i] = result;
			tmp /= 10;
		}
		//0����
		for (int i = 0; i < Number.size(); i++)
		{
			if (Number[i] == -1)
			{
				Number[i] = 0;
			}
		}
		std::reverse(Number.begin(), Number.end());
		return Number;
	}

	//�X�e�[�W�I����ʂ������ɕ`�悷�邩(������W)
	KuroEngine::Vec2<float> m_baseStageSelectPos;
	bool m_stopFlag;


	//�v���r���[���[�h
	bool m_previweFlag, m_triggerPreviewFlag;
	bool stop1FlameFlag;

	bool m_doneFlag;
	KuroEngine::Vec2<float>m_hideVel;
	KuroEngine::Timer m_hideTiemr;
	KuroEngine::Timer m_appearTimer;


	//�I�𒆂̐����̌��ɕ`�悷��
	std::shared_ptr<KuroEngine::TextureBuffer>m_selectTex;
	//���
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, 2>m_dirTex;


	//�N���AUI----------------------------------------

	std::shared_ptr<KuroEngine::TextureBuffer>m_clearTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_clearFlameTex;
	float m_flameAngle;
	KuroEngine::Timer m_beatTimer;
	std::array<KuroEngine::Vec2<float>, 2>m_hexaSize;
	KuroEngine::Vec2<float>m_clearSize;
	//�N���AUI----------------------------------------


	//�X�e�[�WUI----------------------------------------

	struct StageUI
	{
		std::shared_ptr<KuroEngine::TextureBuffer>m_stageTex;
		std::shared_ptr<KuroEngine::TextureBuffer>m_subStageTex;
		StageUI(std::shared_ptr<KuroEngine::TextureBuffer>stageUITex, std::shared_ptr<KuroEngine::TextureBuffer>subStageUITex) :
			m_stageTex(stageUITex), m_subStageTex(subStageUITex)
		{};
	};
	std::vector<StageUI>m_stageTex;

	//�X�e�[�WUI----------------------------------------

	//�㉺�̑�-----------------------------------------------------------------------
	class Band
	{
	public:

		Band(const KuroEngine::Vec2<float>& pos, const KuroEngine::Vec2<float>& size, const KuroEngine::Vec2<float>& easeVel, float flame) :
			m_pos(pos), m_size(size), m_easeVel(easeVel), m_hideVel({}), m_appearTimer(flame), m_disappearTimer(flame), m_appearFlag(true)
		{
			m_appearTimer.ForciblyTimeUp();
			m_disappearTimer.ForciblyTimeUp();
		};

		void Update()
		{
			if (m_appearFlag)
			{
				m_disappearTimer.Reset();
				m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_appearTimer.GetTimeRate(), KuroEngine::Vec2<float>(0.0f, 0.0f), m_easeVel);
				m_appearTimer.UpdateTimer();
			}
			else
			{
				m_appearTimer.Reset();
				m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_disappearTimer.GetTimeRate(), m_easeVel, KuroEngine::Vec2<float>(0.0f, 0.0f));
				m_disappearTimer.UpdateTimer();
			}
			m_appearFlag = false;
		};

		void Draw()
		{
			KuroEngine::DrawFunc2D::DrawBox2D(m_pos + m_hideVel, m_pos + m_size, KuroEngine::Color(18, 43, 38, 255), true);
		};

		void Appear()
		{
			m_appearFlag = true;
		};

	private:
		KuroEngine::Vec2<float> m_pos;
		KuroEngine::Vec2<float> m_size;
		KuroEngine::Vec2<float> m_easeVel;
		KuroEngine::Vec2<float> m_hideVel;
		KuroEngine::Timer m_appearTimer;
		KuroEngine::Timer m_disappearTimer;

		bool m_appearFlag;
	};
	std::array<std::unique_ptr<Band>, 2>m_bandArray;

	//�㉺�̑�-----------------------------------------------------------------------

	int m_prevStageIndex;


	//�v���r���[-----------------------------------------------------------------------

	std::shared_ptr<KuroEngine::Camera> m_previewCamera;
	KuroEngine::Transform m_cameraTransform;
	KuroEngine::Vec3<float>m_cameraPos;
	KuroEngine::Vec2<float> m_angle, m_larpAngle;
	float m_hitBoxRadius;

	KuroEngine::Vec3<float> m_preMouseVel;


	//�v���r���[-----------------------------------------------------------------------


};