#include "Title.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"
#include"../OperationConfig.h"
#include"../System/SaveDataManager.h"
#include"KuroEngineDevice.h"
#include"../GameScene.h"

void Title::MenuUpdate(bool arg_inputUp, bool arg_inputDown, bool arg_inputDone, GameScene* arg_gameScene)
{
	//項目の更新
	auto oldItem = m_nowItem;
	if (m_nowItem < TITLE_MENU_ITEM_NUM - 1 && arg_inputDown)	//下へ
	{
		m_nowItem = (TITLE_MENU_ITEM)(m_nowItem + 1);
	}
	else if (0 < m_nowItem && arg_inputUp)		//上へ
	{
		m_nowItem = (TITLE_MENU_ITEM)(m_nowItem - 1);
	}

	//セーブデータがないときは「つづきから」を選べない
	//if (m_nowItem == CONTINUE && !SaveDataManager::Instance()->IsExistSaveData())m_nowItem = oldItem;

	//選択項目が変わった
	if (m_nowItem != oldItem)
	{
		m_itemArray[m_nowItem].m_status = ITEM_STATUS::SELECT;
		m_itemArray[oldItem].m_status = ITEM_STATUS::DEFAULT;
	}

	//決定
	if (arg_inputDone)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
		switch (m_nowItem)
		{
			//つづきから
			case CONTINUE:
				arg_gameScene->ActivateFastTravel();
				break;
				//はじめから
			case NEW_GAME:
				//最終確認
				m_confirmNewGame.m_isNo = true;
				m_mode = MODE_CONFIRM_NEW_GAME;
				break;
				//設定
			case SETTING:
				break;
				//ゲームをやめる
			case QUIT:
				KuroEngine::KuroEngineDevice::Instance()->GameEnd();
				break;
			default:
				break;
		}
	}
}

void Title::MenuDraw()
{
	using namespace KuroEngine;

	//項目の描画中心座標
	static const Vec2<float>ITEM_DRAW_CENTER_POS = { 1044.0f,200.0f };
	//項目間の行間
	static const float ITEM_LINE_SPACE = 70.0f;

	//項目の描画
	float offsetY = 0.0f;
	for (int itemIdx = 0; itemIdx < TITLE_MENU_ITEM_NUM; ++itemIdx)
	{
		//描画位置計算
		auto drawPos = ITEM_DRAW_CENTER_POS + m_itemArray[itemIdx].m_offsetPos + Vec2<float>(0.0f, offsetY);

		//セーブデータがなければつづきからは選択不可
		float alpha = 1.0f;
		//if (itemIdx == CONTINUE && !SaveDataManager::Instance()->IsExistSaveData())alpha = 0.3f;

		//選択項目
		if (itemIdx == m_nowItem)
		{
			//影描画
			DrawFunc2D::DrawRotaGraph2D(drawPos, { 1.0f,1.0f }, 0.0f, m_selectShadowTex);
		}
		//項目描画
		DrawFunc2D::DrawRotaGraph2D(drawPos, { 1.0f,1.0f }, 0.0f, m_itemArray[itemIdx].GetTex(), alpha);

		//オフセットYずらし
		offsetY += m_itemArray[itemIdx].GetTex()->GetGraphSize().y + ITEM_LINE_SPACE;
	}
}

void Title::ConfirmNewGameUpdate(bool arg_inputLeft, bool arg_inputRight, bool arg_inputDone, GameScene* arg_gameScene)
{
	//「はい」「いいえ」選択
	if ((!m_confirmNewGame.m_isNo && arg_inputLeft) || (m_confirmNewGame.m_isNo && arg_inputRight))
	{
		m_confirmNewGame.m_isNo = !m_confirmNewGame.m_isNo;
	}

	//決定
	if (arg_inputDone)
	{
		//いいえ
		if (m_confirmNewGame.m_isNo)m_mode = MODE_MENU;
		//はい
		else arg_gameScene->StartGame(0, StageManager::Instance()->GetStartPointTransform());
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
}

void Title::ConfirmNewGameDraw()
{
	using namespace KuroEngine;

	const float CENTER_X = 1004.0f;

	//アイコンの座標
	const Vec2<float>ICON_CENTER_POS = { CENTER_X,256.0f };
	DrawFunc2D::DrawRotaGraph2D(ICON_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_iconTex);
	//質問テクスチャの座標
	const Vec2<float>STR_CENTER_POS = { CENTER_X,392.0f };
	DrawFunc2D::DrawRotaGraph2D(STR_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_strTex);

	//「はい」「いいえ」の中心座標からのオフセットX
	const float YES_NO_OFFSET_X = 94.0f;
	//「はい」テクスチャの座標
	const Vec2<float>YES_CENTER_POS = { CENTER_X + YES_NO_OFFSET_X,540.0f };

	//「いいえ」テクスチャの座標
	const Vec2<float>NO_CENTER_POS = { CENTER_X - YES_NO_OFFSET_X,540.0f };

	//影のテクスチャの座標
	auto shadowPos = m_confirmNewGame.m_isNo ? NO_CENTER_POS : YES_CENTER_POS;
	DrawFunc2D::DrawRotaGraph2D(shadowPos, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_shadowTex);

	//「はい」
	DrawFunc2D::DrawRotaGraph2D(YES_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.GetYesTex());
	//「いいえ」
	DrawFunc2D::DrawRotaGraph2D(NO_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.GetNoTex());
}

Title::Title()
	//:m_startGameFlag(false), m_isFinishFlag(false), m_startOPFlag(false), m_generateCameraMoveDataFlag(false),
	//m_delayTime(10)
{
	SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);

	using namespace KuroEngine;

	//テクスチャのディレクトリ
	static const std::string DIR = "resource/user/tex/title/";

	//タイトルロゴ
	m_titleLogoTex = D3D12App::Instance()->GenerateTextureBuffer(DIR + "logo_bright.png");

	//項目のテクスチャ
	const std::array<std::string, TITLE_MENU_ITEM_NUM>TEX_FILE_NAME =
	{
		"continue.png",
		"new_game.png",
		"setting.png",
		"quit.png"
	};
	for (int itemIdx = 0; itemIdx < TITLE_MENU_ITEM_NUM; ++itemIdx)
	{
		D3D12App::Instance()->GenerateTextureBuffer(m_itemArray[itemIdx].m_texArray.data(),
			DIR + TEX_FILE_NAME[itemIdx], ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	}

	m_selectShadowTex = D3D12App::Instance()->GenerateTextureBuffer(DIR + "shadow.png");

	//「はじめから」の最終確認のテクスチャ
	static const std::string CONFIRM_DIR = "resource/user/tex/title/confirm/";
	m_confirmNewGame.m_iconTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "icon.png");
	m_confirmNewGame.m_strTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "str.png");
	D3D12App::Instance()->GenerateTextureBuffer(m_confirmNewGame.m_yesTex.data(), CONFIRM_DIR + "yes.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	D3D12App::Instance()->GenerateTextureBuffer(m_confirmNewGame.m_noTex.data(), CONFIRM_DIR + "no.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	m_confirmNewGame.m_shadowTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "shadow.png");

	//セーブデータが存在に応じて選択項目の初期化
	m_nowItem = SaveDataManager::Instance()->IsExistSaveData() ? CONTINUE : NEW_GAME;
	m_itemArray[m_nowItem].m_status = SELECT;

}

void Title::Init()
{
	//m_startGameFlag = false;
	//m_isFinishFlag = false;
	//m_startOPFlag = false;
	//m_startPazzleFlag = false;
	//m_generateCameraMoveDataFlag = false;
	//m_delayInputFlag = false;
	//m_delayTime.Reset();
	//m_stageSelect.Init();

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

	//m_pazzleModeLogoPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(KuroEngine::WinApp::Instance()->GetExpandWinCenter().x / 2.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);
	//m_storyModeLogoPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(-KuroEngine::WinApp::Instance()->GetExpandWinCenter().x / 2.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);

	//m_startPazzleFlag = false;
	//switch (title_mode)
	//{
	//case TITLE_SELECT:
	//	break;
	//case TITLE_PAZZLE:
	//	m_startPazzleFlag = true;

	//	break;
	//default:
	//	break;
	//}
	//m_doneFlag = false;

	m_mode = MODE_MENU;
}

void Title::Update(KuroEngine::Transform* player_camera, std::shared_ptr<KuroEngine::Camera> arg_cam, GameScene* arg_gameScene)
{
	//入力
	bool inputUp = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool inputDown = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool inputLeft = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool inputRight = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool inputDone = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);

	//選択音
	if (inputUp || inputDown || inputLeft || inputRight)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
	
	switch (m_mode)
	{
		case MODE_MENU:	//通常のメニュー
			MenuUpdate(inputUp, inputDown, inputDone, arg_gameScene);
			break;
		case MODE_CONFIRM_NEW_GAME:	//「はじめから」の確認
			ConfirmNewGameUpdate(inputLeft, inputRight, inputDone, arg_gameScene);
			break;
	}


	//bool isInputSpace = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::A);
	//if (m_isPazzleModeFlag && isInputSpace && !m_startGameFlag && m_stageSelect.IsEnableToDone())
	//{
	//	m_startPazzleFlag = true;
	//	SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	//	m_stageSelect.Init();
	//}


	//急速接近が終わったらOP開始
	//if (m_startGameFlag && m_camera.IsFinish())
	//{
	//	m_startOPFlag = true;
	//}
	//if (m_startGameFlag)
	//{
	//	//m_alphaRate.UpdateTimer();
	//}

	//OPのカメラ挙動
	//if (m_startOPFlag && !m_generateCameraMoveDataFlag)
	//{
	//	SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	//	std::vector<MovieCameraData> lookDownDataArray;

	//	//プレイヤーを基準に座標を動かして気を見ている
	//	MovieCameraData data1;
	//	data1.transform.SetParent(player_camera);
	//	data1.transform.SetPos(KuroEngine::Vec3<float>(0.0f, 20.0f, -10.0f));
	//	data1.transform.SetRotate(KuroEngine::Angle(-60), KuroEngine::Angle(0), KuroEngine::Angle(0));
	//	data1.interpolationTimer = 1;
	//	data1.preStopTimer = 2;
	//	data1.easePosData.easeChangeType = KuroEngine::Out;
	//	data1.easePosData.easeType = KuroEngine::Circ;
	//	lookDownDataArray.emplace_back(data1);

	//	//プレイヤーに戻る
	//	MovieCameraData data2;
	//	data2.transform.SetParent(player_camera);
	//	data2.transform.SetPos(KuroEngine::Vec3<float>(0.0f, 0.0f, 0.0f));
	//	data2.transform.SetRotate(KuroEngine::Angle(0), KuroEngine::Angle(0), KuroEngine::Angle(0));
	//	data2.interpolationTimer = 1;
	//	data2.preStopTimer = 4;
	//	data2.easePosData.easeChangeType = KuroEngine::Out;
	//	data2.easePosData.easeType = KuroEngine::Circ;
	//	lookDownDataArray.emplace_back(data2);

	//	m_camera.StartMovie(lookDownDataArray, false);
	//	m_generateCameraMoveDataFlag = true;
	//}
	////OP終了
	//else if (m_generateCameraMoveDataFlag && !m_camera.IsStart() && m_camera.IsFinish())
	//{
	//	//m_isFinishFlag = true;
	//	OperationConfig::Instance()->SetAllInputActive(true);
	//}

	//if (m_startPazzleFlag)
	//{
	//	//m_stageSelect.Update(arg_cam);

	//	if (m_delayTime.IsTimeUp())
	//	{
	//		m_delayInputFlag = true;
	//	}
	//	m_delayTime.UpdateTimer();

	//	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_ESCAPE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::B))
	//	{
	//		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	//		m_startPazzleFlag = false;
	//		m_delayInputFlag = false;
	//		//m_stageSelect.Stop();
	//		m_delayTime.Reset();
	//	}
	//}
	m_camera.Update();

	//選択された項目のオフセットX
	const float SELECT_ITEM_OFFSET_X = -59.0f;
	for (int itemIdx = 0; itemIdx < TITLE_MENU_ITEM_NUM; ++itemIdx)
	{
		KuroEngine::Vec2<float>targetOffset = { 0.0f,0.0f };
		if (itemIdx == m_nowItem)targetOffset.x = SELECT_ITEM_OFFSET_X;
		m_itemArray[itemIdx].m_offsetPos = KuroEngine::Math::Lerp(m_itemArray[itemIdx].m_offsetPos, targetOffset, 0.08f);
	}
}

void Title::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	using namespace KuroEngine;

	//if (m_isFinishFlag)
	//{
	//	return;
	//}

	////タイトルロゴ描画
	//if (m_startPazzleFlag)
	//{
	//	//m_stageSelect.Draw(arg_cam);
	//	return;
	//}

	////ゲームが始まったら選択画面を表示しない
	//if (m_startGameFlag)
	//{
	//	return;
	//}

	//左端の四角描画
	DrawFunc2D::DrawBox2D({ 0,0 }, { 154.0f,WinApp::Instance()->GetExpandWinSize().y }, Color(0, 21, 13, 180), true);

	//タイトルロゴ
	const Vec2<float>TITLE_LOGO_CENTER_POS = { 412.0f,282.0f };
	DrawFunc2D::DrawRotaGraph2D(TITLE_LOGO_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_titleLogoTex);

	switch (m_mode)
	{
		case MODE_MENU:	//通常のメニュー
			MenuDraw();
			break;
		case MODE_CONFIRM_NEW_GAME:	//「はじめから」の確認
			ConfirmNewGameDraw();
			break;
	}
}
