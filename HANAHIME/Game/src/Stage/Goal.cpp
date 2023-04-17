#include"Goal.h"
#include"../OperationConfig.h"

Goal::Goal() :m_model(std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb")),
m_upVecObj(std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb")),
m_initFlag(false), m_clearEaseTimer(30),
m_upEffectEase(60), m_downEffectEase(10),
m_heightTimer(120)
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinSize().y);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);

	m_goalCamera = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");
	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/KusodasaClear.png");

	m_camera = std::make_shared<KuroEngine::Camera>("cameraName");

	for (int i = 0; i < m_emittObject.size(); ++i)
	{
		m_emittObject[i] = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");
	}

	m_emitter[0].m_baseAngle = 0.0f;
	m_emitter[1].m_baseAngle = 180.0f;
}

void Goal::Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model)
{
	m_initFlag = true;
	m_model->m_transform = transform;
	m_model->m_transform.SetScale(3.0f);
	m_startGoalEffectFlag = false;
	m_isHitFlag = false;

	m_movieCamera.Init();

	m_clearEaseTimer.Reset();
	m_upEffectEase.Reset();
	m_downEffectEase.Reset();

	m_goalModel = goal_model;
	m_goalModelBaseTransform = goal_model->GetTransform();
	m_startCameraFlag = false;

}

void Goal::Finalize()
{
	m_initFlag = false;
}

void Goal::Update(KuroEngine::Transform *transform)
{
	if (!m_initFlag)
	{
		return;
	}

	//①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
	KuroEngine::Vec3<float>eyePos = m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(-40.0f, -5.0f, 20.0f);
	m_goalCamera->m_transform.SetPos(eyePos);
	KuroEngine::Vec3<float>eyeDir = m_goalModelBaseTransform.GetPos() - eyePos;
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
	m_goalCamera->m_transform.SetRotaMatrix(matA);


	auto &c = m_camera->GetTransform();
	c.SetParent(&m_goalCamera->m_transform);



	//ゴールカメラモード
	if (m_isHitFlag && !m_startGoalEffectFlag)
	{
		std::vector<MovieCameraData>cameraDataArray;

		{
			MovieCameraData data;
			data.transform = m_goalCamera->m_transform;

			data.interpolationTimer = 0;
			data.afterStopTimer = 3;
			cameraDataArray.emplace_back(data);
		}

		{
			MovieCameraData data;
			data.transform = m_goalCamera->m_transform;
			//data.afterStopTimer = 2;

			cameraDataArray.emplace_back(data);
		}

		m_movieCamera.StartMovie(cameraDataArray, false);
		m_startGoalEffectFlag = true;
	}
	m_movieCamera.Update();


	m_pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_clearEaseTimer.GetTimeRate(), m_basePos, m_goalPos);
	clearTexRadian = KuroEngine::Angle::ConvertToRadian(KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_clearEaseTimer.GetTimeRate(), 0.0f, 360.0f));
	if (m_isHitFlag)
	{
		m_clearEaseTimer.UpdateTimer();
	}
}

void Goal::Draw(KuroEngine::Camera &camera)
{
	if (!m_initFlag)
	{
		return;
	}
#ifdef _DEBUG
	KuroEngine::DrawFunc3D::DrawNonShadingModel(m_model, camera);
	KuroEngine::DrawFunc3D::DrawNonShadingModel(m_goalCamera, camera);

	if (!m_isHitFlag)
	{
		//前
		{
			KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
			KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + frontVec * 5.0f);
			KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 0, 255, 100), 1.0f);
		}
		//上
		{
			KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
			KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + upVec * 5.0f);
			KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 255, 0, 100), 1.0f);
		}
		//横
		{
			KuroEngine::Vec3<float>rightDir(upVec.Cross(frontVec));

			KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
			KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + rightDir * 5.0f);
			KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(255, 0, 0, 100), 1.0f);
		}
	}

	//上ベクトル可視化----------------------------------------

	KuroEngine::Vec3<float> result = GetFlagUpVec();
	KuroEngine::Vec3<float>startPos(m_goalModelBaseTransform.GetPos());
	KuroEngine::Vec3<float>endPos(m_goalModelBaseTransform.GetPos() + result * 5.0f);
	KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(255, 0, 0, 255), 1.0f);

	m_upVecObj->m_transform.SetPos(endPos);
	KuroEngine::DrawFunc3D::DrawNonShadingModel(m_upVecObj, camera);

	//上ベクトル可視化----------------------------------------

	//回転可視化
	for (auto obj : m_emittObject)
	{
		KuroEngine::DrawFunc3D::DrawNonShadingModel(obj, camera);
	}


#endif // _DEBUG

	for (auto &obj : m_particleArray)
	{
		if (obj.IsAlive())
		{
			obj.Draw();
		}
	}

	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_pos, { 1.0f,1.0f }, clearTexRadian, m_clearTex);
	//KuroEngine::DrawFunc3D::DrawNonShadingPlane(m_ddsTex, transform, camera);
}

bool Goal::IsHit(const KuroEngine::Vec3<float> &player_pos)
{
	if (!m_initFlag)
	{
		return false;
	}
	std::array<KuroEngine::Vec3<float>, 2> size = { m_model->m_transform.GetScale(),m_model->m_transform.GetScale() };
	KuroEngine::Vec3<float>distance = m_model->m_transform.GetPos() - player_pos;
	const int square1 = 0;
	const int square2 = 1;
	bool isHitFlag =
		fabs(distance.x) <= size[square1].x + size[square2].x &&
		fabs(distance.y) <= size[square1].y + size[square2].y &&
		fabs(distance.z) <= size[square1].z + size[square2].z;
	m_isHitFlag = isHitFlag;
	return isHitFlag;
}

bool Goal::IsEnd()
{
	return m_isHitFlag && m_movieCamera.IsFinish();
}

Goal::Particle::Particle()
{

}

void Goal::Particle::Init(const KuroEngine::Vec3<float> &pos)
{
	m_initFlag = true;
	m_timer = 0;
}

void Goal::Particle::Update()
{
	if (120 <= m_timer)
	{
		m_initFlag = false;
	}
	++m_timer;
}

void Goal::Particle::Draw()
{
	//KuroEngine::DrawFunc3D::DrawNonShadingModel(, );
}