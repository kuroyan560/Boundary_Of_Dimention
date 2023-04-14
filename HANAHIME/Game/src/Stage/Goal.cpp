#include"Goal.h"
#include"../OperationConfig.h"

Goal::Goal() :m_model(std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb")), m_initFlag(false),m_clearEaseTimer(30)
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinSize() + KuroEngine::Vec2<float>(0.0f, 100.0f);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::WinApp::Instance()->GetExpandWinCenter() / KuroEngine::Vec2<float>(1.0f, 2.0f);


	//m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/");
}

void Goal::Init(const KuroEngine::Transform &transform)
{
	m_initFlag = true;
	m_model->m_transform = transform;
	m_model->m_transform.SetScale(3.0f);
	m_startGoalEffectFlag = false;
	m_isHitFlag = false;

	m_movieCamera.Init();

	m_clearEaseTimer.Reset();
}

void Goal::Finalize()
{
	m_initFlag = false;
}

void Goal::Update(KuroEngine::Transform *transform)
{
	if(!m_initFlag)
	{
		return;
	}

	//ゴールカメラモード
	if (m_isHitFlag && !m_startGoalEffectFlag)
	{
		std::vector<MovieCameraData>cameraDataArray;

		{
			MovieCameraData data;
			data.transform = m_model->m_transform;
			data.transform.SetPos(m_model->m_transform.GetPos() + KuroEngine::Vec3<float>(0.0f, 0.0f, -12.0f));

			data.interpolationTimer = 0;
			data.afterStopTimer = 3;
			cameraDataArray.emplace_back(data);
		}

		{
			MovieCameraData data;
			data.transform = m_model->m_transform;
			data.transform.SetPos(m_model->m_transform.GetPos() + KuroEngine::Vec3<float>(0.0f, 0.0f, -12.0f));
			//data.afterStopTimer = 2;

			cameraDataArray.emplace_back(data);
		}

		m_movieCamera.StartMovie(cameraDataArray, false);
		m_startGoalEffectFlag = true;
	}
	m_movieCamera.Update();


	m_pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_clearEaseTimer.GetTimeRate(), m_basePos, m_goalPos);

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
	//KuroEngine::DrawFunc3D::DrawNonShadingModel(m_model, camera);
#endif // _DEBUG

	//KuroEngine::DrawFunc2D::DrawGraph(m_pos, m_clearTex);
}

bool Goal::IsHit(const KuroEngine::Vec3<float> &player_pos)
{
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

