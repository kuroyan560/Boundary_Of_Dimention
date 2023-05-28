#include "Title.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"
#include"../OperationConfig.h"
#include"../System/SaveDataManager.h"
#include"KuroEngineDevice.h"
#include"../GameScene.h"

void Title::MenuUpdate(bool arg_inputUp, bool arg_inputDown, bool arg_inputDone, GameScene* arg_gameScene)
{
	//���ڂ̍X�V
	auto oldItem = m_nowItem;
	if (m_nowItem < TITLE_MENU_ITEM_NUM - 1 && arg_inputDown)	//����
	{
		m_nowItem = (TITLE_MENU_ITEM)(m_nowItem + 1);
	}
	else if (0 < m_nowItem && arg_inputUp)		//���
	{
		m_nowItem = (TITLE_MENU_ITEM)(m_nowItem - 1);
	}

	//�Z�[�u�f�[�^���Ȃ��Ƃ��́u�Â�����v��I�ׂȂ�
	//if (m_nowItem == CONTINUE && !SaveDataManager::Instance()->IsExistSaveData())m_nowItem = oldItem;

	//�I�����ڂ��ς����
	if (m_nowItem != oldItem)
	{
		m_itemArray[m_nowItem].m_status = ITEM_STATUS::SELECT;
		m_itemArray[oldItem].m_status = ITEM_STATUS::DEFAULT;
	}

	//����
	if (arg_inputDone)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
		switch (m_nowItem)
		{
			//�Â�����
			case CONTINUE:
				arg_gameScene->ActivateFastTravel();
				break;
				//�͂��߂���
			case NEW_GAME:
				//�ŏI�m�F
				m_confirmNewGame.m_isNo = true;
				m_mode = MODE_CONFIRM_NEW_GAME;
				break;
				//�ݒ�
			case SETTING:
				break;
				//�Q�[������߂�
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

	//���ڂ̕`�撆�S���W
	static const Vec2<float>ITEM_DRAW_CENTER_POS = { 1044.0f,200.0f };
	//���ڊԂ̍s��
	static const float ITEM_LINE_SPACE = 70.0f;

	//���ڂ̕`��
	float offsetY = 0.0f;
	for (int itemIdx = 0; itemIdx < TITLE_MENU_ITEM_NUM; ++itemIdx)
	{
		//�`��ʒu�v�Z
		auto drawPos = ITEM_DRAW_CENTER_POS + m_itemArray[itemIdx].m_offsetPos + Vec2<float>(0.0f, offsetY);

		//�Z�[�u�f�[�^���Ȃ���΂Â�����͑I��s��
		float alpha = 1.0f;
		//if (itemIdx == CONTINUE && !SaveDataManager::Instance()->IsExistSaveData())alpha = 0.3f;

		//�I������
		if (itemIdx == m_nowItem)
		{
			//�e�`��
			DrawFunc2D::DrawRotaGraph2D(drawPos, { 1.0f,1.0f }, 0.0f, m_selectShadowTex);
		}
		//���ڕ`��
		DrawFunc2D::DrawRotaGraph2D(drawPos, { 1.0f,1.0f }, 0.0f, m_itemArray[itemIdx].GetTex(), alpha);

		//�I�t�Z�b�gY���炵
		offsetY += m_itemArray[itemIdx].GetTex()->GetGraphSize().y + ITEM_LINE_SPACE;
	}
}

void Title::ConfirmNewGameUpdate(bool arg_inputLeft, bool arg_inputRight, bool arg_inputDone, GameScene* arg_gameScene)
{
	//�u�͂��v�u�������v�I��
	if ((!m_confirmNewGame.m_isNo && arg_inputLeft) || (m_confirmNewGame.m_isNo && arg_inputRight))
	{
		m_confirmNewGame.m_isNo = !m_confirmNewGame.m_isNo;
	}

	//����
	if (arg_inputDone)
	{
		//������
		if (m_confirmNewGame.m_isNo)m_mode = MODE_MENU;
		//�͂�
		else arg_gameScene->StartGame(0, StageManager::Instance()->GetStartPointTransform());
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
}

void Title::ConfirmNewGameDraw()
{
	using namespace KuroEngine;

	const float CENTER_X = 1004.0f;

	//�A�C�R���̍��W
	const Vec2<float>ICON_CENTER_POS = { CENTER_X,256.0f };
	DrawFunc2D::DrawRotaGraph2D(ICON_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_iconTex);
	//����e�N�X�`���̍��W
	const Vec2<float>STR_CENTER_POS = { CENTER_X,392.0f };
	DrawFunc2D::DrawRotaGraph2D(STR_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_strTex);

	//�u�͂��v�u�������v�̒��S���W����̃I�t�Z�b�gX
	const float YES_NO_OFFSET_X = 94.0f;
	//�u�͂��v�e�N�X�`���̍��W
	const Vec2<float>YES_CENTER_POS = { CENTER_X + YES_NO_OFFSET_X,540.0f };

	//�u�������v�e�N�X�`���̍��W
	const Vec2<float>NO_CENTER_POS = { CENTER_X - YES_NO_OFFSET_X,540.0f };

	//�e�̃e�N�X�`���̍��W
	auto shadowPos = m_confirmNewGame.m_isNo ? NO_CENTER_POS : YES_CENTER_POS;
	DrawFunc2D::DrawRotaGraph2D(shadowPos, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.m_shadowTex);

	//�u�͂��v
	DrawFunc2D::DrawRotaGraph2D(YES_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.GetYesTex());
	//�u�������v
	DrawFunc2D::DrawRotaGraph2D(NO_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmNewGame.GetNoTex());
}

Title::Title()
	//:m_startGameFlag(false), m_isFinishFlag(false), m_startOPFlag(false), m_generateCameraMoveDataFlag(false),
	//m_delayTime(10)
{
	SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);

	using namespace KuroEngine;

	//�e�N�X�`���̃f�B���N�g��
	static const std::string DIR = "resource/user/tex/title/";

	//�^�C�g�����S
	m_titleLogoTex = D3D12App::Instance()->GenerateTextureBuffer(DIR + "logo_bright.png");

	//���ڂ̃e�N�X�`��
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

	//�u�͂��߂���v�̍ŏI�m�F�̃e�N�X�`��
	static const std::string CONFIRM_DIR = "resource/user/tex/title/confirm/";
	m_confirmNewGame.m_iconTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "icon.png");
	m_confirmNewGame.m_strTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "str.png");
	D3D12App::Instance()->GenerateTextureBuffer(m_confirmNewGame.m_yesTex.data(), CONFIRM_DIR + "yes.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	D3D12App::Instance()->GenerateTextureBuffer(m_confirmNewGame.m_noTex.data(), CONFIRM_DIR + "no.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	m_confirmNewGame.m_shadowTex = D3D12App::Instance()->GenerateTextureBuffer(CONFIRM_DIR + "shadow.png");

	//�Z�[�u�f�[�^�����݂ɉ����đI�����ڂ̏�����
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

	//�z�[���̎�����~��ɉ���Ă�������----------------------------------------
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
	//�z�[���̎�����~��ɉ���Ă�������----------------------------------------


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
	//����
	bool inputUp = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool inputDown = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool inputLeft = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool inputRight = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool inputDone = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);

	//�I����
	if (inputUp || inputDown || inputLeft || inputRight)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
	
	switch (m_mode)
	{
		case MODE_MENU:	//�ʏ�̃��j���[
			MenuUpdate(inputUp, inputDown, inputDone, arg_gameScene);
			break;
		case MODE_CONFIRM_NEW_GAME:	//�u�͂��߂���v�̊m�F
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


	//�}���ڋ߂��I�������OP�J�n
	//if (m_startGameFlag && m_camera.IsFinish())
	//{
	//	m_startOPFlag = true;
	//}
	//if (m_startGameFlag)
	//{
	//	//m_alphaRate.UpdateTimer();
	//}

	//OP�̃J��������
	//if (m_startOPFlag && !m_generateCameraMoveDataFlag)
	//{
	//	SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	//	std::vector<MovieCameraData> lookDownDataArray;

	//	//�v���C���[����ɍ��W�𓮂����ċC�����Ă���
	//	MovieCameraData data1;
	//	data1.transform.SetParent(player_camera);
	//	data1.transform.SetPos(KuroEngine::Vec3<float>(0.0f, 20.0f, -10.0f));
	//	data1.transform.SetRotate(KuroEngine::Angle(-60), KuroEngine::Angle(0), KuroEngine::Angle(0));
	//	data1.interpolationTimer = 1;
	//	data1.preStopTimer = 2;
	//	data1.easePosData.easeChangeType = KuroEngine::Out;
	//	data1.easePosData.easeType = KuroEngine::Circ;
	//	lookDownDataArray.emplace_back(data1);

	//	//�v���C���[�ɖ߂�
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
	////OP�I��
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

	//�I�����ꂽ���ڂ̃I�t�Z�b�gX
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

	////�^�C�g�����S�`��
	//if (m_startPazzleFlag)
	//{
	//	//m_stageSelect.Draw(arg_cam);
	//	return;
	//}

	////�Q�[�����n�܂�����I����ʂ�\�����Ȃ�
	//if (m_startGameFlag)
	//{
	//	return;
	//}

	//���[�̎l�p�`��
	DrawFunc2D::DrawBox2D({ 0,0 }, { 154.0f,WinApp::Instance()->GetExpandWinSize().y }, Color(0, 21, 13, 180), true);

	//�^�C�g�����S
	const Vec2<float>TITLE_LOGO_CENTER_POS = { 412.0f,282.0f };
	DrawFunc2D::DrawRotaGraph2D(TITLE_LOGO_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_titleLogoTex);

	switch (m_mode)
	{
		case MODE_MENU:	//�ʏ�̃��j���[
			MenuDraw();
			break;
		case MODE_CONFIRM_NEW_GAME:	//�u�͂��߂���v�̊m�F
			ConfirmNewGameDraw();
			break;
	}
}
