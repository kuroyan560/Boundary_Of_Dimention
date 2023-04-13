#include "Title.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"
#include"../OperationConfig.h"

Title::Title()
	:m_startGameFlag(false), m_isFinishFlag(false), m_startOPFlag(false), m_generateCameraMoveDataFlag(false), 
	m_titleTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/title/logo.png")),
	m_alphaRate(30.0f), m_startPazzleFlag(false), m_isPazzleModeFlag(false), m_delayInputFlag(false),
	m_pazzleModeTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/title/Pazzle.png")),
	m_storyModeTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/title/Story.png")),
	m_delayTime(10)
{
}

void Title::Init(TitleMode title_mode)
{
	m_startGameFlag = false;
	m_isFinishFlag = false;
	m_startOPFlag = false;
	m_startPazzleFlag = false;
	m_generateCameraMoveDataFlag = false;
	m_isPazzleModeFlag = false;
	m_delayInputFlag = false;
	m_delayTime.Reset();
	m_alphaRate.Reset();
	m_stageSelect.Init();
	OperationConfig::Instance()->SetActive(false);


	std::vector<MovieCameraData> titleCameraMoveDataArray;

	const int xAngle = 20;
	const float radius = 500.0f;
	const float height = 250.0f;

	//ホームの周りを円状に回っていく処理----------------------------------------
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

	//中心から多少上にずらす
	m_titlePos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(0.0f, -KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f + 100);
	m_titleLogoSize = { 1,1 };

	m_pazzleModeLogoPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(KuroEngine::WinApp::Instance()->GetExpandWinCenter().x / 2.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);
	m_storyModeLogoPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(-KuroEngine::WinApp::Instance()->GetExpandWinCenter().x / 2.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);

	switch (title_mode)
	{
	case TITLE_SELECT:
		break;
	case TITLE_PAZZLE:
		m_startPazzleFlag = true;
		break;
	default:
		break;
	}
}

void Title::Update(KuroEngine::Transform *player_camera)
{
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT))
	{
		m_isPazzleModeFlag = true;
	}
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_LEFT))
	{
		m_isPazzleModeFlag = false;
	}


	//タイトル画面からツリー注目モードに切り替える
	if (!m_isPazzleModeFlag && KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) && !m_startGameFlag && !m_startPazzleFlag)
	{
		m_startGameFlag = true;

		std::vector<MovieCameraData> moveToTreeDataArray;
		MovieCameraData data1;
		data1.transform = m_camera.GetCamera().lock()->GetTransform();
		data1.interpolationTimer = 1;
		data1.easePosData.easeChangeType = KuroEngine::Out;
		data1.easePosData.easeType = KuroEngine::Circ;
		moveToTreeDataArray.emplace_back(data1);


		KuroEngine::Vec3<float>cameraPos = m_camera.GetCamera().lock()->GetTransform().GetPosWorld();
		MovieCameraData data2;
		data2.transform.SetPos(cameraPos.GetCenter(KuroEngine::Transform().GetPos()));
		data2.transform.SetRotaMatrix(data1.transform.GetMatWorld());
		data2.interpolationTimer = 0;
		data2.easePosData.easeChangeType = KuroEngine::Out;
		data2.easePosData.easeType = KuroEngine::Circ;
		moveToTreeDataArray.emplace_back(data2);

		m_camera.StartMovie(moveToTreeDataArray, false);
	}

	if (m_isPazzleModeFlag && KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) && !m_startGameFlag)
	{
		m_startPazzleFlag = true;
	}


	//急速接近が終わったらOP開始
	if (m_startGameFlag && m_camera.IsFinish())
	{
		m_startOPFlag = true;
	}
	if (m_startGameFlag)
	{
		m_alphaRate.UpdateTimer();
	}

	//OPのカメラ挙動
	if (m_startOPFlag && !m_generateCameraMoveDataFlag)
	{
		std::vector<MovieCameraData> lookDownDataArray;

		//プレイヤーを基準に座標を動かして気を見ている
		MovieCameraData data1;
		data1.transform.SetParent(player_camera);
		data1.transform.SetPos(KuroEngine::Vec3<float>(0.0f, 20.0f, -10.0f));
		data1.transform.SetRotate(KuroEngine::Angle(-60), KuroEngine::Angle(0), KuroEngine::Angle(0));
		data1.interpolationTimer = 1;
		data1.preStopTimer = 2;
		data1.easePosData.easeChangeType = KuroEngine::Out;
		data1.easePosData.easeType = KuroEngine::Circ;
		lookDownDataArray.emplace_back(data1);

		//プレイヤーに戻る
		MovieCameraData data2;
		data2.transform.SetParent(player_camera);
		data2.transform.SetPos(KuroEngine::Vec3<float>(0.0f, 0.0f, 0.0f));
		data2.transform.SetRotate(KuroEngine::Angle(0), KuroEngine::Angle(0), KuroEngine::Angle(0));
		data2.interpolationTimer = 1;
		data2.preStopTimer = 4;
		data2.easePosData.easeChangeType = KuroEngine::Out;
		data2.easePosData.easeType = KuroEngine::Circ;
		lookDownDataArray.emplace_back(data2);

		m_camera.StartMovie(lookDownDataArray, false);
		m_generateCameraMoveDataFlag = true;
	}
	//OP終了
	else if (m_generateCameraMoveDataFlag && !m_camera.IsStart() && m_camera.IsFinish())
	{
		m_isFinishFlag = true;
		OperationConfig::Instance()->SetActive(true);
	}

	if (m_startPazzleFlag)
	{
		m_stageSelect.Update();

		if (m_delayTime.IsTimeUp())
		{
			m_delayInputFlag = true;
		}
		m_delayTime.UpdateTimer();

		if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_ESCAPE))
		{
			m_startPazzleFlag = false;
			m_delayInputFlag = false;
			m_delayTime.Reset();
		}
	}
	m_camera.Update();
}

void Title::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_isFinishFlag)
	{
		return;
	}

	//タイトルロゴ描画
	if (m_startPazzleFlag)
	{
		m_stageSelect.Draw();
		return;
	}
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_titlePos, m_titleLogoSize.Float(), 0.0f, m_titleTexBuff, 1.0f - m_alphaRate.GetTimeRate());


	//ゲームが始まったら選択画面を表示しない
	if (m_startGameFlag)
	{
		return;
	}
	float storyLogoAlpha = 1.0f;
	float pazzleLogoAlpha = 1.0f;
	if (m_isPazzleModeFlag)
	{
		storyLogoAlpha = 0.5f;
	}
	else
	{
		pazzleLogoAlpha = 0.5f;
	}

	//パズルモード選択中
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_pazzleModeLogoPos, { 1.0f,1.0f }, 0.0f, m_pazzleModeTexBuff, pazzleLogoAlpha);
	//ストーリーモード選択中
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_storyModeLogoPos, { 1.0f,1.0f }, 0.0f, m_storyModeTexBuff, storyLogoAlpha);

}
