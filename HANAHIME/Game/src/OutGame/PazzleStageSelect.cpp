#include"PazzleStageSelect.h"
#include"FrameWork/WinApp.h"
#include"../Stage/StageManager.h"


PazzleStageSelect::PazzleStageSelect()
{
	const int STAGE_MAX_NUM = StageManager::Instance()->GetAllStageNum();
	int yNum = 0;
	for (int y = 0; y < m_stageSelectArray.size(); ++y)
	{
		for (int x = 0; x < m_stageSelectArray[y].size(); ++x)
		{
			int stageNumber = yNum + x;
			if (stageNumber < STAGE_MAX_NUM)
			{
				m_stageSelectArray[y][x].enableFlag = true;
			}
		}
		yNum += static_cast<int>(m_stageSelectArray[y].size());
	}


	m_numTexArray.resize(10);
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_numTexArray.data(), "resource/user/tex/Number.png", 10, { 10,1 });

	m_selectTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(KuroEngine::Color(255, 255, 255, 255));

	float bandSize = 100.0f;

	m_bandArray[0] = std::make_unique<Band>(KuroEngine::Vec2<float>(0.0f, bandSize), KuroEngine::WinApp::Instance()->GetExpandWinSize() * KuroEngine::Vec2<float>(1.0f, -1.0f), KuroEngine::Vec2<float>(0.0f, -bandSize), 60.0f);
	m_bandArray[1] = std::make_unique<Band>(KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinSize().y - bandSize), KuroEngine::WinApp::Instance()->GetExpandWinSize(), KuroEngine::Vec2<float>(0.0f, bandSize), 60.0f);
}

void PazzleStageSelect::Init()
{
	m_stopFlag = false;
}

void PazzleStageSelect::Update()
{
	if (m_stopFlag)
	{
		return;
	}

	//�R���g���[���[�̓��͂�ۑ��B
	KuroEngine::Vec2<float> contollerLeftStickInput = KuroEngine::UsersInput::Instance()->GetLeftStickVecFuna(0);

	const float DEADLINE = 0.8f;
	bool isInputRightController = m_prevContollerLeftStick.x < DEADLINE &&DEADLINE < contollerLeftStickInput.x;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT) || isInputRightController)
	{
		++m_nowStageNum.x;
	}
	bool isInputLeftController = -DEADLINE < m_prevContollerLeftStick.x &&contollerLeftStickInput.x < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_LEFT) || isInputLeftController)
	{
		--m_nowStageNum.x;
	}
	bool isInputUpController = -DEADLINE < m_prevContollerLeftStick.y &&contollerLeftStickInput.y < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_UP) || isInputUpController)
	{
		--m_nowStageNum.y;
	}
	bool isInputDownController = m_prevContollerLeftStick.y < DEADLINE &&DEADLINE < contollerLeftStickInput.y;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_DOWN) || isInputDownController)
	{
		++m_nowStageNum.y;
	}

	//�R���g���[���[�̓��͂�ۑ��B
	m_prevContollerLeftStick = contollerLeftStickInput;

	int stageYMaxNum = static_cast<int>(m_stageSelectArray.size());
	if (m_nowStageNum.y < 0)
	{
		m_nowStageNum.y = stageYMaxNum - 1;
	}
	if (stageYMaxNum <= m_nowStageNum.y)
	{
		m_nowStageNum.y = 0;
	}


	int stageXMaxNum = static_cast<int>(m_stageSelectArray[m_nowStageNum.y].size());
	//����[���獶�ɍs�����Ƃ�����E���[�Ɍ�����
	if (m_nowStageNum.x < 0 && m_nowStageNum.y == 0)
	{
		m_nowStageNum.y = stageYMaxNum - 1;
		m_nowStageNum.x = stageXMaxNum - 1;
	}
	//�E���[����E�ɍs�����Ƃ����獶��[�Ɍ�����
	if (stageXMaxNum <= m_nowStageNum.x && m_nowStageNum.y == stageYMaxNum - 1)
	{
		m_nowStageNum.y = 0;
		m_nowStageNum.x = 0;
	}
	//���̍ő�l�s�����Ƃ������ɍs��
	if (m_nowStageNum.x < 0)
	{
		--m_nowStageNum.y;
		m_nowStageNum.x = stageXMaxNum - 1;
	}
	//�E�̍ő�l�s�����Ƃ����牺�ɍs��
	if (stageXMaxNum <= m_nowStageNum.x)
	{
		++m_nowStageNum.y;
		m_nowStageNum.x = 0;
	}

	//�N���A�������ǂ���
	if (m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
		bool debug = false;
	}

	for (auto &obj : m_bandArray)
	{
		if (KuroEngine::UsersInput::Instance()->KeyInput(DIK_L))
		{
			obj->Appear();
		}
		obj->Update();
	}
}

void PazzleStageSelect::Draw()
{
	m_baseStageSelectPos = { 200.0f,64.0f };


	for (auto &obj : m_bandArray)
	{
		obj->Draw();
	}


	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_baseStageSelectPos + KuroEngine::Vec2<float>(10.0f, 25.0f), KuroEngine::Vec2<float>(128.0f, 128.0f), 0.0f, m_selectTex, 1.0f);

	//Y�����l�����������̃J�E���g�ɕK�v
	int yNum = 0;

	KuroEngine::Vec2<float>texSize(64.0f * 2.0f, 64.0f);
	//�����̕`��
	for (int y = 0; y < m_stageSelectArray.size(); ++y)
	{
		for (int x = 0; x < m_stageSelectArray[y].size(); ++x)
		{
			KuroEngine::Vec2<float>pos(static_cast<float>(x), static_cast<float>(y));
			int stageNumber = yNum + x;

			float numberAlpha = 1.0f;
			if (!m_stageSelectArray[y][x].enableFlag)
			{
				numberAlpha = 0.5f;
			}


			//�I�𒆂̔ԍ�����݂ĉ�����Ή����قǓ����ɂ��Ă���
			if (GetNumber() <= stageNumber|| stageNumber < GetNumber() + 10)
			{
				numberAlpha = 1.0f - abs((stageNumber - GetNumber()) / 10.0f);
			}


			KuroEngine::Vec2<float>basePos(pos * texSize + m_baseStageSelectPos);
			KuroEngine::Vec2<float>size(1.0f, 1.0f);
			//���̊Ԃ̃X�y�[�X
			KuroEngine::Vec2<float>digitsBetween(30.0f, 0.0f);
			//�I�𒆂̐����͋���������
			bool isSelectingFlag = GetNumber() == stageNumber;
			if (isSelectingFlag)
			{
				size = { 2.0f,2.0f };
				digitsBetween = { 50.0f,0.0f };
				basePos.y += 30.0f;
			}
			//���̊Ԃ�^�񒆂Ɏ����Ă�������
			basePos -= { 15.0f, 0.0f };
			//�I�𒆂̐����̗��ɕ`�悷��ׂ̍��W��ۑ�����
			KuroEngine::Vec2<float>selectingTexPos = basePos;

			//�I�𒆂̐�������ɑS�Ă̐��������炷�B
			basePos.x -= GetNumber() * 128.0f;

			//���p��
			std::vector<int>timeArray = CountNumber(stageNumber + 1);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos, size, 0.0f, m_numTexArray[timeArray[0]], numberAlpha);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos + digitsBetween, size, 0.0f, m_numTexArray[timeArray[1]], numberAlpha);
		}
		yNum += static_cast<int>(m_stageSelectArray[y].size());
	}


}

int PazzleStageSelect::GetNumber()
{
	size_t num = 0;
	for (int y = 0; y < m_nowStageNum.y; ++y)
	{
		num += m_stageSelectArray[y].size();
	}
	num += m_nowStageNum.x;

	return static_cast<int>(num);
}