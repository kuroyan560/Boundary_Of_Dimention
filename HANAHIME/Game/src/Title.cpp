#include "Title.h"
#include"../../../src/engine/FrameWork/UsersInput.h"
#include"../../../src/engine/FrameWork/WinApp.h"

Title::Title() :m_startGameFlag(false), m_isFinishFlag(false), m_startOPFlag(false), m_generateCameraMoveDataFlag(false), m_blackTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(KuroEngine::Color(0, 0, 0, 255)))
{
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

	//下のが良いが	C4172	ローカル変数またはテンポラリのアドレスを返しますが出る為要相談
	//m_titlePos = { KuroEngine::WinApp::Instance()->GetWinCenter()};
	m_titlePos = { 600,300 };
	m_titleLogoSize = { 400,100 };

	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
}

void Title::Update(KuroEngine::Transform *player_camera)
{
	//タイトル画面からツリー注目モードに切り替える
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) && !m_startGameFlag)
	{
		m_startGameFlag = true;

		MovieCameraData data1;
		data1.transform = m_camera.GetCamera().lock()->GetTransform();
		data1.interpolationTimer = 1;
		data1.easePosData.easeChangeType = KuroEngine::Out;
		data1.easePosData.easeType = KuroEngine::Circ;
		moveToTreeDataArray.emplace_back(data1);

		KuroEngine::Vec3<float>cameraPos = m_camera.GetCamera().lock()->GetTransform().GetPosWorld();


		KuroEngine::Vec3<float>normal = cameraPos - KuroEngine::Transform().GetPos();
		KuroEngine::Vec3<float>pos = cameraPos.GetCenter(KuroEngine::Transform().GetPos());
		normal.Normalize();
		MovieCameraData data2;
		data2.transform.SetPos(pos);
		data2.transform.SetRotaMatrix(data1.transform.GetMatWorld());
		KuroEngine::Vec3<KuroEngine::Angle> angle = m_camera.GetCamera().lock()->GetTransform().GetRotateAsEuler();
		data2.interpolationTimer = 0;
		data2.easePosData.easeChangeType = KuroEngine::Out;
		data2.easePosData.easeType = KuroEngine::Circ;
		moveToTreeDataArray.emplace_back(data2);


		m_camera.StartMovie(moveToTreeDataArray, false);

	}


	//急速接近が終わったらOP開始
	if (m_startGameFlag && m_camera.IsFinish())
	{
		m_startOPFlag = true;
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
		data2.transform.SetPos(KuroEngine::Vec3<float>(0.0f,0.0f,0.0f));
		data2.transform.SetRotate(KuroEngine::Angle(0), KuroEngine::Angle(0), KuroEngine::Angle(0));
		data2.interpolationTimer = 1;
		data2.preStopTimer = 4;
		data2.easePosData.easeChangeType = KuroEngine::Out;
		data2.easePosData.easeType = KuroEngine::Circ;
		lookDownDataArray.emplace_back(data2);

		m_camera.StartMovie(lookDownDataArray, false);
		m_generateCameraMoveDataFlag = true;
		m_startOPFlag = false;
	}
	//OP終了
	else if (m_generateCameraMoveDataFlag && !m_camera.IsStart() && m_camera.IsFinish())
	{
		m_isFinishFlag = true;
	}


	m_camera.Update();
}

void Title::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (!m_startGameFlag)
	{
		//タイトルロゴ描画
		//KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_titlePos.Float(), m_titleLogoSize.Float(), 0.0f, m_blackTexBuff);
	}
}
