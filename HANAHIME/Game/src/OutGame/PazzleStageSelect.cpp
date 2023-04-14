#include"PazzleStageSelect.h"
#include"FrameWork/WinApp.h"
#include"../Stage/StageManager.h"


PazzleStageSelect::PazzleStageSelect()
{
	m_baseStageSelectPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() / 2.0f;

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

	//コントローラーの入力を保存。
	KuroEngine::Vec2<float> contollerLeftStickInput = KuroEngine::UsersInput::Instance()->GetLeftStickVecFuna(0);

	const float DEADLINE = 0.8f;
	bool isInputRightController = m_prevContollerLeftStick.x < DEADLINE && DEADLINE < contollerLeftStickInput.x;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT) || isInputRightController)
	{
		++m_nowStageNum.x;
	}
	bool isInputLeftController = -DEADLINE < m_prevContollerLeftStick.x && contollerLeftStickInput.x < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_LEFT) || isInputLeftController)
	{
		--m_nowStageNum.x;
	}
	bool isInputUpController = -DEADLINE < m_prevContollerLeftStick.y && contollerLeftStickInput.y < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_UP) || isInputUpController)
	{
		--m_nowStageNum.y;
	}
	bool isInputDownController = m_prevContollerLeftStick.y < DEADLINE && DEADLINE < contollerLeftStickInput.y;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_DOWN) || isInputDownController)
	{
		++m_nowStageNum.y;
	}

	//コントローラーの入力を保存。
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
	//左上端から左に行こうとしたら右下端に向かう
	if (m_nowStageNum.x < 0 && m_nowStageNum.y == 0)
	{
		m_nowStageNum.y = stageYMaxNum - 1;
		m_nowStageNum.x = stageXMaxNum - 1;
	}
	//右下端から右に行こうとしたら左上端に向かう
	if (stageXMaxNum <= m_nowStageNum.x && m_nowStageNum.y == stageYMaxNum - 1)
	{
		m_nowStageNum.y = 0;
		m_nowStageNum.x = 0;
	}
	//左の最大値行こうとしたら上に行く
	if (m_nowStageNum.x < 0)
	{
		--m_nowStageNum.y;
		m_nowStageNum.x = stageXMaxNum - 1;
	}
	//右の最大値行こうとしたら下に行く
	if (stageXMaxNum <= m_nowStageNum.x)
	{
		++m_nowStageNum.y;
		m_nowStageNum.x = 0;
	}

	//クリアしたかどうか
	if (m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
		bool debug = false;
	}
}

void PazzleStageSelect::Draw()
{
	//Y軸を考慮した数字のカウントに必要
	int yNum = 0;

	KuroEngine::Vec2<float>texSize(64.0f, 64.0f);
	//数字の描写
	for (int y = 0; y < m_stageSelectArray.size(); ++y)
	{
		for (int x = 0; x < m_stageSelectArray[y].size(); ++x)
		{
			KuroEngine::Vec2<float>pos(static_cast<float>(x), static_cast<float>(y));
			int stageNumber = 1 + yNum + x;
			//桁用意
			std::vector<int>timeArray = CountNumber(stageNumber);

			float numberAlpha = 1.0f;
			if (!m_stageSelectArray[y][x].enableFlag)
			{
				numberAlpha = 0.5f;
			}

			//一桁
			if (stageNumber < 10)
			{
				KuroEngine::DrawFunc2D::DrawGraph(m_baseStageSelectPos + pos * texSize, m_numTexArray[timeArray[1]], numberAlpha);
			}
			//二桁
			else
			{
				KuroEngine::Vec2<float>basePos(pos * texSize + m_baseStageSelectPos);
				//桁の間を真ん中に持っていく処理
				basePos -= KuroEngine::Vec2<float>(15.0f, 0.0f);
				KuroEngine::DrawFunc2D::DrawGraph(basePos, m_numTexArray[timeArray[0]], numberAlpha);
				KuroEngine::DrawFunc2D::DrawGraph(basePos + KuroEngine::Vec2<float>(30.0f, 0.0f), m_numTexArray[timeArray[1]], numberAlpha);
			}
		}
		yNum += static_cast<int>(m_stageSelectArray[y].size());
	}
	//現在選択中
	KuroEngine::DrawFunc2D::DrawBox2D(m_baseStageSelectPos + m_nowStageNum.Float() * texSize, m_baseStageSelectPos + m_nowStageNum.Float() * texSize + texSize, KuroEngine::Color(255, 255, 0, 255), false);
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