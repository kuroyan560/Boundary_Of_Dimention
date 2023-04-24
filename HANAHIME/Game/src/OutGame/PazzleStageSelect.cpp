#include"PazzleStageSelect.h"
#include"FrameWork/WinApp.h"
#include"../Stage/StageManager.h"


PazzleStageSelect::PazzleStageSelect() :m_beatTimer(30), m_appearTimer(60), m_hideTiemr(60)
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

	m_numMainTexArray.resize(10);
	m_numSubTexArray.resize(10);
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_numMainTexArray.data(), "resource/user/tex/stage_select/stage_num_main.png", 10, { 10,1 });
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_numSubTexArray.data(), "resource/user/tex/stage_select/stage_num_sub.png", 10, { 10,1 });

	m_selectTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/cursor.png");


	for (int i = 0; i < GetMaxNumber(); ++i)
	{
		m_stageTex.emplace_back(
			KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/stage_name_main_test.png"),
			KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/stage_name_sub_test.png")
		);
	}

	m_dirTex[0] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/select_arrow.png");
	m_dirTex[1] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/select_arrow.png");

	m_clearFlameTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/clear_hexagon_big.png");
	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/clear_str.png");

	float bandSize = 100.0f;

	m_bandArray[0] = std::make_unique<Band>(KuroEngine::Vec2<float>(0.0f, bandSize), KuroEngine::WinApp::Instance()->GetExpandWinSize() * KuroEngine::Vec2<float>(1.0f, -1.0f), KuroEngine::Vec2<float>(0.0f, -bandSize), 60.0f);
	m_bandArray[1] = std::make_unique<Band>(KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinSize().y - bandSize), KuroEngine::WinApp::Instance()->GetExpandWinSize(), KuroEngine::Vec2<float>(0.0f, bandSize), 60.0f);


	//ホームの周りを円状に回っていく処理----------------------------------------
	const int xAngle = 20;
	const float radius = 100.0f;
	const float height = 50.0f;

	std::vector<MovieCameraData> titleCameraMoveDataArray;
	MovieCameraData data;
	data.easePosData.easeType = KuroEngine::EASING_TYPE_NUM;
	data.easePosData.easeChangeType = KuroEngine::EASE_CHANGE_TYPE_NUM;
	data.easeRotaData.easeType = KuroEngine::EASING_TYPE_NUM;
	data.easeRotaData.easeChangeType = KuroEngine::EASE_CHANGE_TYPE_NUM;

	const int limitPosMaxNum = 20;
	for (int i = 0; i < limitPosMaxNum; ++i)
	{
		int angle = (360 / limitPosMaxNum) * i;
		float radian = KuroEngine::Angle(angle);

		data.transform.SetPos(KuroEngine::Vec3<float>(cosf(radian) * radius, height, sinf(radian) * radius));
		data.transform.SetRotate(KuroEngine::Angle(xAngle), KuroEngine::Angle(-90 - angle), KuroEngine::Angle(0));
		data.preStopTimer = 0;
		data.interpolationTimer = 2;
		titleCameraMoveDataArray.emplace_back(data);

		KuroEngine::Matrix mat = titleCameraMoveDataArray[i].transform.GetMatWorld();
		mat = titleCameraMoveDataArray[i].transform.GetMatWorld();
	}
	float radian = KuroEngine::Angle((360 / limitPosMaxNum) * 0);
	data.transform.SetPos(KuroEngine::Vec3<float>(cosf(radian) * radius, height, sinf(radian) * radius));
	data.transform.SetRotate(KuroEngine::Angle(xAngle), KuroEngine::Angle(-90), KuroEngine::Angle(0));
	data.preStopTimer = 0;
	data.interpolationTimer = 2;
	titleCameraMoveDataArray.emplace_back(data);
	//ホームの周りを円状に回っていく処理----------------------------------------

	m_camera.StartMovie(titleCameraMoveDataArray, true);
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
	bool selectFlag = false;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT) || isInputRightController)
	{
		++m_nowStageNum.x;
		selectFlag = true;
	}
	bool isInputLeftController = -DEADLINE < m_prevContollerLeftStick.x &&contollerLeftStickInput.x < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_LEFT) || isInputLeftController)
	{
		--m_nowStageNum.x;
		selectFlag = true;
	}
	bool isInputUpController = -DEADLINE < m_prevContollerLeftStick.y &&contollerLeftStickInput.y < -DEADLINE;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_UP) || isInputUpController)
	{
		--m_nowStageNum.y;
		selectFlag = true;
	}
	bool isInputDownController = m_prevContollerLeftStick.y < DEADLINE &&DEADLINE < contollerLeftStickInput.y;
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_DOWN) || isInputDownController)
	{
		++m_nowStageNum.y;
		selectFlag = true;
	}
	if (selectFlag)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
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


	int stageXMaxNum = static_cast<int>(StageManager::Instance()->GetAllStageNum());
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

	if (0 <= GetNumber() && GetNumber() < StageManager::Instance()->GetAllStageNum())
	{
		StageManager::Instance()->SetStage(GetNumber());
	}

	if (m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
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

		m_hexaSize[0] = KuroEngine::Math::Lerp(m_hexaSize[0], { 1.0f,1.0f }, 0.1f);
		m_hexaSize[1] = KuroEngine::Math::Lerp(m_hexaSize[1], { 0.75f,0.75f }, 0.1f);
		m_clearSize = KuroEngine::Math::Lerp(m_clearSize, { 1.0f,1.0f }, 0.1f);

		++m_flameAngle;
	}
	else
	{
		m_beatTimer.Reset();
		m_flameAngle = -10;
		m_hexaSize[0] = { 1.0f,1.0f };
		m_hexaSize[1] = { 0.75f,0.75f };
		m_clearSize = { 1.0f,1.0f };
	}


	if (KuroEngine::UsersInput::Instance()->KeyInput(DIK_L))
	{
		m_previweFlag = true;
	}
	else
	{
		m_previweFlag = false;
	}

	for (auto &obj : m_bandArray)
	{
		if (m_previweFlag)
		{
			obj->Appear();
		}
		obj->Update();
	}


	if (m_previweFlag)
	{
		m_hideTiemr.Reset();
		m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_appearTimer.GetTimeRate(), KuroEngine::Vec2<float>(0.0f, 0.0f), KuroEngine::Vec2<float>(0.0f, 250.0f));
		m_appearTimer.UpdateTimer();
	}
	else
	{
		m_appearTimer.Reset();
		m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_hideTiemr.GetTimeRate(), m_hideVel, KuroEngine::Vec2<float>(0.0f, 0.0f));
		m_hideTiemr.UpdateTimer();
	}

	m_camera.Update();
}

void PazzleStageSelect::Draw()
{
	m_baseStageSelectPos = { 200.0f,64.0f };


	for (auto &obj : m_bandArray)
	{
		obj->Draw();
	}

	//選択中の数字の背景
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_baseStageSelectPos + KuroEngine::Vec2<float>(10.0f, 25.0f) - m_hideVel, KuroEngine::Vec2<float>(1.0f, 1.0f), 0.0f, m_selectTex, 1.0f);

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
			//選択中の番号からみて遠ければ遠いほど透明にしていく
			if (GetNumber() <= stageNumber || stageNumber < GetNumber() + 10)
			{
				numberAlpha = 1.0f - abs((stageNumber - GetNumber()) / 10.0f);
			}
			if (!m_stageSelectArray[y][x].enableFlag)
			{
				numberAlpha = 0.0f;
			}

			KuroEngine::Vec2<float>basePos(pos * texSize + m_baseStageSelectPos);
			KuroEngine::Vec2<float>size(1.0f, 1.0f);
			//桁の間のスペース
			KuroEngine::Vec2<float>digitsBetween(30.0f, 0.0f);
			//選択中の数字は強調させる
			bool isSelectingFlag = GetNumber() == stageNumber;
			m_nowNumTexArray = m_numSubTexArray;
			if (isSelectingFlag)
			{
				m_nowNumTexArray = m_numMainTexArray;
				size = { 0.8f,0.8f };
				digitsBetween = { 60.0f,0.0f };
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
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos - m_hideVel, size, 0.0f, m_nowNumTexArray[timeArray[0]], numberAlpha);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(basePos + digitsBetween - m_hideVel, size, 0.0f, m_nowNumTexArray[timeArray[1]], numberAlpha);
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
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[1] + m_hideVel, { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(180), m_dirTex[1]);
	}
	//最後矢印表示
	else if (GetNumber() == StageManager::Instance()->GetAllStageNum() - 1)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[0] + m_hideVel, { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(0), m_dirTex[0]);
	}
	//両方矢印表示
	else
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[0] + m_hideVel, { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(0), m_dirTex[0]);
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[1] + m_hideVel, { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(180), m_dirTex[1]);
	}

	//プレビュー時に隠れきれてないUIを隠す
	const float offsetVel = 2.0f;

	//ステージ名,ステージサブタイトル
	KuroEngine::Vec2<float>stageUIPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter();
	stageUIPos.x = 470.0f;
	stageUIPos.y += 130.0f;
	for (auto &uiTex : m_stageTex)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageUIPos + m_hideVel * offsetVel, { 1.0f,1.0f }, 0.0f, uiTex.m_stageTex);
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageUIPos + KuroEngine::Vec2<float>(80.0f, 120.0f) + m_hideVel * offsetVel, { 1.0f,1.0f }, 0.0f, uiTex.m_subStageTex, 80.0f / 255.0f);
	}

	//クリア表示
	KuroEngine::Vec2<float>flamePos = KuroEngine::WinApp::Instance()->GetExpandWinSize();
	flamePos.x -= 250.0f;
	flamePos.y = KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + 150.0f;

	float alpha = 1.0f;
	if (!m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
		alpha *= 0.2f;
	}
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos + m_hideVel * offsetVel, m_clearSize, 0.0f, m_clearTex, alpha);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos + m_hideVel * offsetVel, m_hexaSize[0], KuroEngine::Angle::ConvertToRadian(m_flameAngle), m_clearFlameTex, alpha);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos + m_hideVel * offsetVel, m_hexaSize[1], KuroEngine::Angle::ConvertToRadian(-m_flameAngle), m_clearFlameTex, alpha);

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
