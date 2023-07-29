#include"PazzleStageSelect.h"
#include"FrameWork/WinApp.h"
#include"../Stage/StageManager.h"
#include"../Stage/Stage.h"
#include <cmath>
#include <limits>
#include <iostream>
#include"../OperationConfig.h"
#include"../GameScene.h"

PazzleStageSelect::PazzleStageSelect() :m_beatTimer(90), m_appearTimer(60), m_hideTiemr(60)
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


	for (int i = 0; i < STAGE_MAX_NUM; ++i)
	{
		m_stageTex.emplace_back(
			//KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/stage_name_main_test.png"),
			KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage_select/stage_name_main_" + std::to_string(i + 1) + ".png"),
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

	m_appearTimer.ForciblyTimeUp();
	m_hideTiemr.ForciblyTimeUp();

	m_previewCamera = std::make_shared<KuroEngine::Camera>("PreviewCamera");
	m_cameraAngle = 0;
	m_cameraLength = DEF_CAMERA_LENGTH;

	for (int index = 0; index < 2; ++index) {
		m_arrowAlpha[index] = 0.0f;
		m_arrowSinTimer[index] = 0;
		m_arrowSinTimerAddNow[index] = ARROW_SINE_TIMER;
		m_arrowSinTimerAddBase[index] = ARROW_SINE_TIMER;
		m_arrowSineLengthNow[index] = ARROW_SINE_INIT_LENGTH;
		m_arrowSineLengthBase[index] = ARROW_SINE_INIT_LENGTH;
	}
}

void PazzleStageSelect::Init()
{
	m_stopFlag = false;
	m_previweFlag = false;
	m_cameraAngle = 0;
	m_cameraLength = DEF_CAMERA_LENGTH;

	for (int index = 0; index < 2; ++index) {
		m_arrowAlpha[index] = 0.0f;
		m_arrowSinTimer[index] = 0;
		m_arrowSinTimerAddNow[index] = ARROW_SINE_TIMER;
		m_arrowSinTimerAddBase[index] = ARROW_SINE_TIMER;
		m_arrowSineLengthNow[index] = ARROW_SINE_INIT_LENGTH;
		m_arrowSineLengthBase[index] = ARROW_SINE_INIT_LENGTH;
	}
	for (int i = 0; i < StageManager::Instance()->GetAllStageNum(); ++i)
	{
		StageManager::Instance()->SetStage(i);
	}
	StageManager::Instance()->SetStage(0);
	m_doneFlag = false;
}

void PazzleStageSelect::Update(std::shared_ptr<KuroEngine::Camera> arg_cam, GameScene* arg_gameScene)
{
	if (m_stopFlag)
	{
		return;
	}

	bool selectFlag = false;
	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT))
	{
		++m_nowStageNum.x;
		selectFlag = true;
		m_cameraLength = FAR_CAMERA_LENGTH;
		m_arrowSineLengthNow[RIGHT] = ARROW_SINE_INIT_LENGTH_ADD;
		m_arrowSinTimerAddNow[RIGHT] = ARROW_SINE_TIMER_ADD;
	}

	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT))
	{
		--m_nowStageNum.x;
		selectFlag = true;
		m_cameraLength = FAR_CAMERA_LENGTH;
		m_arrowSineLengthNow[LEFT] = ARROW_SINE_INIT_LENGTH_ADD;
		m_arrowSinTimerAddNow[LEFT] = ARROW_SINE_TIMER_ADD;
	}
	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP))
	{
		//--m_nowStageNum.y;
		//selectFlag = true;
		//m_cameraLength = FAR_CAMERA_LENGTH;
		//m_arrowSineLengthNow[RIGHT] = ARROW_SINE_INIT_LENGTH_ADD;
		//m_arrowSinTimerAddNow[RIGHT] = ARROW_SINE_TIMER_ADD;
	}
	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN))
	{
		//++m_nowStageNum.y;
		//selectFlag = true;
		//m_cameraLength = FAR_CAMERA_LENGTH;
		//m_arrowSineLengthNow[LEFT] = ARROW_SINE_INIT_LENGTH_ADD;
		//m_arrowSinTimerAddNow[LEFT] = ARROW_SINE_TIMER_ADD;
	}

	//タイトルへ戻る
	if (OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER)
		|| OperationConfig::Instance()->GetOperationInput(OperationConfig::PAUSE, OperationConfig::ON_TRIGGER))
	{
		arg_gameScene->GoBackTitle();
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}

	if (selectFlag)
	{
		m_previweFlag = false;
		SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
	}

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
		if (GetNumber() != m_preStageNum)
		{
			StageManager::Instance()->SetStage(GetNumber());
		}
	}
	m_preStageNum = GetNumber();

	if (m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].m_isClearFlag)
	{
		//クリアUIのビート表現
		if (m_beatTimer.IsTimeUp())
		{
			float vel = 0.2f;
			m_hexaSize[0] += vel;
			m_hexaSize[1] += vel * 0.7f;
			m_clearSize += vel * 0.8f;
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


	for (auto& obj : m_bandArray)
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


	KuroEngine::UsersInput::MouseMove mouseVel = KuroEngine::UsersInput::Instance()->GetMouseMove();
	KuroEngine::Vec3<float>inputVel = {
		static_cast<float>(mouseVel.m_inputX / 5.0f) + KuroEngine::UsersInput::Instance()->GetRightStickVec(0).x * 5.0f,
		static_cast<float>(mouseVel.m_inputY / 5.0f) + KuroEngine::UsersInput::Instance()->GetRightStickVec(0).y * 5.0f,
		static_cast<float>(mouseVel.m_inputZ)
	};

	bool moveFlag = inputVel.x != m_preMouseVel.x ||
		inputVel.y != m_preMouseVel.y ||
		inputVel.z != m_preMouseVel.z;
	float mouseDeadLine = 10.0f;
	bool velFlag = mouseDeadLine <= abs(inputVel.x) ||
		mouseDeadLine <= abs(inputVel.y) ||
		mouseDeadLine <= abs(inputVel.z);

	bool controllerFlag = KuroEngine::UsersInput::Instance()->GetRightStickVec(0).x != 0.0f || KuroEngine::UsersInput::Instance()->GetRightStickVec(0).y != 0.0f;

	if ((moveFlag && velFlag) || controllerFlag)
	{
		m_previweFlag = true;
	}

	if (OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER) && !m_doneFlag)
	{
		if (m_previweFlag)
		{
			stop1FlameFlag = true;
		}
		m_previweFlag = false;

		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}

	//プレビューモードのカメラ
	if (m_previweFlag)
	{
		//カメラの初期化
		if (m_previweFlag && !m_triggerPreviewFlag)
		{
			KuroEngine::Matrix mat = m_camera.GetCamera().lock()->GetTransform().GetMatWorld();
			m_cameraTransform.SetWorldMat(mat);
			m_cameraTransform.CalucuratePosRotaBasedOnWorldMatrix();

			m_cameraPos = m_cameraTransform.GetPosWorld();

			//角度入手----------------------------------------
			KuroEngine::Vec3<float>cameraVec(m_cameraPos);
			cameraVec.y = 0.0f;
			cameraVec.Normalize();
			float radian = atan2(cameraVec.z, cameraVec.x);
			m_angle.x = static_cast<float>(KuroEngine::Angle::ConvertToDegree(radian));
			m_angle.y = 0.0f;

			m_larpAngle = m_angle;

			//角度入手----------------------------------------

			m_hitBoxRadius = 100.0f;
		}
		m_larpAngle.x += inputVel.x;
		m_larpAngle.y += inputVel.y;

		const float ANGLE_MAX = 90;
		if (m_larpAngle.y <= -ANGLE_MAX)
		{
			m_larpAngle.y = -ANGLE_MAX;
		}
		if (10.0f <= m_larpAngle.y)
		{
			m_larpAngle.y = 10.0f;
		}
		m_angle = KuroEngine::Math::Lerp(m_angle, m_larpAngle, 0.1f);

		KuroEngine::Vec2<float>radian(KuroEngine::Angle::ConvertToRadian(m_angle.x), KuroEngine::Angle::ConvertToRadian(m_angle.y));
		KuroEngine::Vec2<float>velX = { cosf(radian.x) * m_hitBoxRadius,sinf(radian.x) * m_hitBoxRadius };
		KuroEngine::Vec2<float>velY = { cosf(radian.y) * m_hitBoxRadius,sinf(radian.y) * m_hitBoxRadius };
		m_cameraPos = {
			velX.x,
			50.0f + velY.y,
			velX.y
		};


		//①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
		KuroEngine::Vec3<float>eyePos = m_cameraPos;
		KuroEngine::Vec3<float>eyeDir = KuroEngine::Vec3<float>(0.0f, 0.0f, 0.0f) - eyePos;
		eyeDir.Normalize();

		//②上ベクトルを固定し、右ベクトルを求める。
		KuroEngine::Vec3<float> rightVec = eyeDir.Cross({ 0,-1,0 });
		rightVec.Normalize();

		//③視点ベクトルと右ベクトルから正しい上ベクトルを求める。
		KuroEngine::Vec3<float> upVec = eyeDir.Cross(rightVec);
		upVec.Normalize();

		//④求められたベクトルから姿勢を出す。
		DirectX::XMMATRIX matA = DirectX::XMMatrixIdentity();
		matA.r[0] = { rightVec.x,rightVec.y,rightVec.z,0.0f };
		matA.r[1] = { upVec.x,upVec.y,upVec.z,0.0f };
		matA.r[2] = { eyeDir.x,eyeDir.y,eyeDir.z,0.0f };
		m_cameraTransform.SetPos(m_cameraPos);
		m_cameraTransform.SetRotaMatrix(matA);
		m_cameraTransform.CalucuratePosRotaBasedOnWorldMatrix();

		auto& transform = m_previewCamera->GetTransform();
		transform.SetParent(&m_cameraTransform);
	}
	else
	{

		m_cameraAngle += CAMERA_ANGLE_ADD;

		//カメラの座標を求める。
		KuroEngine::Vec3<float> cameraDir = KuroEngine::Vec3<float>(cosf(m_cameraAngle), 0.3f, sinf(m_cameraAngle));
		cameraDir.Normalize();
		m_cameraPos = cameraDir * m_cameraLength;

		//①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
		KuroEngine::Vec3<float>eyePos = m_cameraPos;
		KuroEngine::Vec3<float>eyeDir = StageManager::Instance()->GetNowStage().lock()->GetStartPointTransform().GetPosWorld() - eyePos;
		eyeDir.Normalize();

		//②上ベクトルを固定し、右ベクトルを求める。
		KuroEngine::Vec3<float> rightVec = eyeDir.Cross({ 0,-1,0 });
		rightVec.Normalize();

		//③視点ベクトルと右ベクトルから正しい上ベクトルを求める。
		KuroEngine::Vec3<float> upVec = eyeDir.Cross(rightVec);
		upVec.Normalize();

		//④求められたベクトルから姿勢を出す。
		DirectX::XMMATRIX matA = DirectX::XMMatrixIdentity();
		matA.r[0] = { rightVec.x,rightVec.y,rightVec.z,0.0f };
		matA.r[1] = { upVec.x,upVec.y,upVec.z,0.0f };
		matA.r[2] = { eyeDir.x,eyeDir.y,eyeDir.z,0.0f };
		m_cameraTransform.SetPos(m_cameraPos);
		m_cameraTransform.SetRotaMatrix(matA);
		m_cameraTransform.CalucuratePosRotaBasedOnWorldMatrix();

		arg_cam->GetTransform().SetPos({});
		arg_cam->GetTransform().SetRotate(KuroEngine::Quaternion());
		auto& transform = arg_cam->GetTransform();
		transform.SetParent(&m_cameraTransform);

		//m_camera.Update();
	}
	m_triggerPreviewFlag = m_previweFlag;
	m_preMouseVel = inputVel;

	//カメラの距離を補間
	m_cameraLength += (DEF_CAMERA_LENGTH - m_cameraLength) / 10.0f;

	//座標を補完する。
	for (int y = 0; y < m_stageSelectArray.size(); ++y)
	{
		for (int x = 0; x < m_stageSelectArray[y].size(); ++x)
		{

			m_nowPos[y][x] += (m_basePos[y][x] - m_nowPos[y][x]) / 10.0f;
			m_nowAlpha[y][x] += (m_baseAlpha[y][x] - m_nowAlpha[y][x]) / 10.0f;

		}

	}

	//選択したらアルファを0にする。
	if (selectFlag) {
		//選択した瞬間に選択中の数字のアルファ値を減らす。
		m_nowAlpha[0][GetNumber()] = 0.0f;
	}

	for (int index = 0; index < 2; ++index) {

		//サイン波のタイマーを更新。
		m_arrowSinTimerAddBase[index] += (ARROW_SINE_TIMER - m_arrowSinTimerAddBase[index]) / 2.0f;
		m_arrowSinTimerAddNow[index] += (m_arrowSinTimerAddBase[index] - m_arrowSinTimerAddNow[index]) / 15.0f;
		m_arrowSinTimer[index] += m_arrowSinTimerAddNow[index];

		//サイン波のタイマーを更新。
		m_arrowSineLengthBase[index] += (ARROW_SINE_INIT_LENGTH - m_arrowSineLengthBase[index]) / 2.0f;
		m_arrowSineLengthNow[index] += (m_arrowSineLengthBase[index] - m_arrowSineLengthNow[index]) / 15.0f;
	}

	//最初矢印表示
	if (GetNumber() == 0)
	{
		m_arrowAlpha[LEFT] -= m_arrowAlpha[LEFT] / 5.0f;
		m_arrowAlpha[RIGHT] += (1.0f - m_arrowAlpha[RIGHT]) / 5.0f;
	}
	//最後矢印表示
	else if (GetNumber() == StageManager::Instance()->GetAllStageNum() - 1)
	{
		m_arrowAlpha[RIGHT] -= m_arrowAlpha[RIGHT] / 5.0f;
		m_arrowAlpha[LEFT] += (1.0f - m_arrowAlpha[LEFT]) / 5.0f;
	}
	//両方表示
	else {
		m_arrowAlpha[RIGHT] += (1.0f - m_arrowAlpha[RIGHT]) / 5.0f;
		m_arrowAlpha[LEFT] += (1.0f - m_arrowAlpha[LEFT]) / 5.0f;
	}

}

void PazzleStageSelect::Draw(KuroEngine::Camera& arg_cam)
{
	m_baseStageSelectPos = { 200.0f,64.0f };


	for (auto& obj : m_bandArray)
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

			m_basePos[y][x] = basePos;
			m_baseAlpha[y][x] = numberAlpha;

			//桁用意
			std::vector<int>timeArray = CountNumber(stageNumber + 1);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_nowPos[y][x] - m_hideVel, size, 0.0f, m_nowNumTexArray[timeArray[0]], m_nowAlpha[y][x]);
			KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_nowPos[y][x] + digitsBetween - m_hideVel, size, 0.0f, m_nowNumTexArray[timeArray[1]], m_nowAlpha[y][x]);

		}
		yNum += static_cast<int>(m_stageSelectArray[y].size());
	}

	std::array<KuroEngine::Vec2<float>, 2>posArray;
	KuroEngine::Vec2<float>offset(60.0f, 180.0f);
	posArray[0] = { offset.x,KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + offset.y };
	posArray[1] = { KuroEngine::WinApp::Instance()->GetExpandWinSize().x - offset.x,KuroEngine::WinApp::Instance()->GetExpandWinCenter().y + offset.y };

	//矢印の移動量を計算。
	float sineMoveRight = std::sinf(m_arrowSinTimer[RIGHT]) * m_arrowSineLengthNow[RIGHT];
	float sineMoveLeft = std::sinf(m_arrowSinTimer[LEFT]) * m_arrowSineLengthNow[LEFT];

	KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[LEFT] + m_hideVel + KuroEngine::Vec2<float>(0, sineMoveLeft), { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(0), m_dirTex[LEFT], m_arrowAlpha[LEFT]);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(posArray[RIGHT] + m_hideVel + KuroEngine::Vec2<float>(0, sineMoveRight), { 1.0f,1.0f }, KuroEngine::Angle::ConvertToRadian(180), m_dirTex[RIGHT], m_arrowAlpha[RIGHT]);

	//プレビュー時に隠れきれてないUIを隠す
	const float offsetVel = 2.0f;

	//ステージ名,ステージサブタイトル
	KuroEngine::Vec2<float>stageUIPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter();
	stageUIPos.x = 500.0f;
	stageUIPos.y += 170.0f;
	//for (auto& uiTex : m_stageTex)
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageUIPos + m_hideVel * offsetVel, { 1.0f,1.0f }, 0.0f, m_stageTex[m_nowStageNum.x].m_stageTex);
		//KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageUIPos + KuroEngine::Vec2<float>(80.0f, 120.0f) + m_hideVel * offsetVel, { 1.0f,1.0f }, 0.0f, uiTex.m_subStageTex, 80.0f / 255.0f);
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
	else
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(flamePos + m_hideVel * offsetVel, m_clearSize, 0.0f, m_clearTex, alpha);
	}
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
