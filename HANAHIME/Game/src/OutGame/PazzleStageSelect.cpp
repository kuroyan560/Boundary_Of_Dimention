#include"PazzleStageSelect.h"
#include"FrameWork/WinApp.h"
#include"../Stage/StageManager.h"


PazzleStageSelect::PazzleStageSelect() :m_beatTimer(30)
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


	m_dirTex[0] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Dir.png");
	m_dirTex[1] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Dir.png");

	m_clearFlameTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Hexagon.png");
	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/title/Pazzle.png");

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

	//コントローラーの入力を保存。
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

	for (auto &obj : m_bandArray)
	{
		if (KuroEngine::UsersInput::Instance()->KeyInput(DIK_L))
		{
			obj->Appear();
		}
		obj->Update();
	}


	//クリアUIのビート表現
	if (m_beatTimer.IsTimeUp())
	{
		float vel = 0.4f;
		m_hexaSize[0] += vel;
		m_hexaSize[1] += vel;
		m_clearSize += vel;
		m_beatTimer.Reset();
	}
	m_beatTimer.UpdateTimer();

	m_hexaSize[0] = KuroEngine::Math::Lerp(m_hexaSize[0], { 2.0f,2.0f }, 0.1f);
	m_hexaSize[1] = KuroEngine::Math::Lerp(m_hexaSize[1], { 1.5f,1.5f }, 0.1f);
	m_clearSize = KuroEngine::Math::Lerp(m_clearSize, { 1.0f,1.0f }, 0.1f);

	++m_flameAngle;
}

void PazzleStageSelect::Draw()
{
	m_baseStageSelectPos = { 200.0f,64.0f };


	for (auto &obj : m_bandArray)
	{
		obj->Draw();
	}


	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_baseStageSelectPos + KuroEngine::Vec2<float>(10.0f, 25.0f), KuroEngine::Vec2<float>(128.0f, 128.0f), 0.0f, m_selectTex, 1.0f);

	//Y軸を考慮した数字のカウントに必要
	int yNum = 0;

	KuroEngine::Vec2<float>texSize(64.0f * 2.0f, 64.0f);
	//数字の描写
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

			//選択中の番号からみて遠ければ遠いほど透明にしていく
			if (GetNumber() <= stageNumber || stageNumber < GetNumber() + 10)
			{
				numberAlpha = 1.0f - abs((stageNumber - GetNumber()) / 10.0f);
			}

			KuroEngine::Vec2<float>basePos(pos * texSize + m_baseStageSelectPos);
			KuroEngine::Vec2<float>size(1.0f, 1.0f);
			//桁の間のスペース
			KuroEngine::Vec2<float>digitsBetween(30.0f, 0.0f);
			//選択中の数字は強調させる
			bool isSelectingFlag = GetNumber() == stageNumber;
			if (isSelectingFlag)
			{
				size = { 2.0f,2.0f };
				digitsBetween = { 50.0f,0.0f };
				basePos.y += 30.0f;
			}
			//桁の間を真ん中に持っていく処理
			basePos -= { 15.0f, 0.0f };
			//選択中の数字の裏に描画する為の座標を保存する
			KuroEngine::Vec2<float>selectingTexPos = basePos;

			//選択中の数字を基準に全ての数字をずらす。
			basePos.x -= GetNumber() * 128.0f;

			//桁用意
			std::vector<int>timeArray = CountNumber(stageNumber + 1);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos, size, 0.0f, m_numTexArray[timeArray[0]], numberAlpha);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos + digitsBetween, size, 0.0f, m_numTexArray[timeArray[1]], numberAlpha);
		}
		yNum += static_cast<int>(m_stageSelectArray[y].size());
	}


	std::array<KuroEngine::Vec2<float>, 2>posArray;
	KuroEngine::Vec2<float>offset(60.0f, 180.0f);
	posArray[0] = { offset.x,KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + offset.y };
	posArray[1] = { KuroEngine::WinApp::Instance()->GetExpandWinSize().x - offset.x,KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + offset.y };

	//最初矢印表示
	if (GetNumber() == 0)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[0], { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(180), m_dirTex[0]);
	}
	//最後矢印表示
	else if (GetNumber() == GetMaxNumber() - 1)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[1], { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(0), m_dirTex[1]);
	}
	//両方矢印表示
	else
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[0], { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(180), m_dirTex[0]);
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[1], { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(0), m_dirTex[1]);
	}


	//ステージ名
	//ステージサブタイトル

	//クリア表示

	KuroEngine::Vec2<float>flamePos = KuroEngine::WinApp::Instance()->GetExpandWinSize();
	flamePos.x -= 250.0f;
	flamePos.y = KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + 150.0f;
	if (m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos, m_clearSize, 0.0f, m_clearTex);
	}
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos, m_hexaSize[0], KuroEngine::Angle::ConvertToRadian(m_flameAngle), m_clearFlameTex);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos, m_hexaSize[1], KuroEngine::Angle::ConvertToRadian(-m_flameAngle), m_clearFlameTex);
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

int PazzleStageSelect::GetMaxNumber()
{
	size_t num = 0;
	for (int y = 0; y < m_stageSelectArray.size(); ++y)
	{
		num += m_stageSelectArray[y].size();
	}
	return static_cast<int>(num);
}
