#include"Goal.h"
#include"../OperationConfig.h"

Goal::Goal() :m_model(std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb")), m_initFlag(false), m_clearEaseTimer(30),
m_upEffectEase(30), m_downEffectEase(10)
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinSize() + KuroEngine::Vec2<float>(0.0f, 100.0f);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::WinApp::Instance()->GetExpandWinCenter() / KuroEngine::Vec2<float>(1.0f, 2.0f);

	m_goalCamera = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");

	//m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/");

	m_camera = std::make_shared<KuroEngine::Camera>("cameraName");
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
	if(!m_initFlag)
	{
		return;
	}

	if (m_isHitFlag)
	{
		float upEffectRate = m_upEffectEase.GetTimeRate();

		//ゴールモデルの演出
		KuroEngine::Transform transform;
		//回転
		KuroEngine::Vec3<float>rotaVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, upEffectRate, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,360.0f });
		transform.SetRotate(rotaVel);
		//上に上げる
		KuroEngine::Vec3<float>upVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, upEffectRate, m_goalModelBaseTransform.GetPos(), m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f));
		transform.SetPos(upVel);

		m_upEffectEase.UpdateTimer();
		if (m_upEffectEase.IsTimeUp())
		{
			//下に下げる
			KuroEngine::Vec3<float>downVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_downEffectEase.GetTimeRate(), m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), m_goalModelBaseTransform.GetPos());
			transform.SetPos(downVel);
			m_downEffectEase.UpdateTimer();
		}
		if (m_downEffectEase.IsTimeUp())
		{
			m_startCameraFlag = true;
		}

		m_goalModel->SetTransform(transform);
	}

	//①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
	KuroEngine::Vec3<float>eyePos = m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 10.0f, 40.0f);
	KuroEngine::Vec3<float>eyeDir = m_goalModelBaseTransform.GetPos() - eyePos;
	frontVec = eyeDir;
	eyeDir.Normalize();
	frontVec.Normalize();

	//②視点ベクトルと(0, 0, 1)ベクトルを外積して法線を求める。
	KuroEngine::Vec3<float>eyeNormal = eyeDir.Cross(KuroEngine::Vec3<float>(0.0f, 0.0f, 1.0f));
	eyeNormal.Normalize();

	//③視点ベクトルと(0, 0, 1)ベクトルを内積して回転量を求める。
	float eyeRota = eyeDir.Dot(KuroEngine::Vec3<float>(0.0f, 0.0f, 1.0f));
	eyeRota = acos(eyeRota);
	if (std::isnan(eyeRota))
	{
		eyeRota = 0.0f;
	}

	//④クォータニオンの関数で②を回転軸として③回転するクォータニオンを求める。
	KuroEngine::Quaternion quaternion = DirectX::XMQuaternionRotationNormal(eyeNormal, eyeRota);

	//⑤④で求めれたクォータニオンを(0, 1, 0)ベクトルにかける。
	KuroEngine::Vec3<float> result(0, 1, 0);
	DirectX::XMMATRIX mat = DirectX::XMMatrixRotationQuaternion(quaternion);
	DirectX::XMVECTOR rota(DirectX::XMVector3Transform(result, mat));
	result = { rota.m128_f32[0],rota.m128_f32[1],rota.m128_f32[2] };

	upVec = result;
	upVec.Normalize();

	//これで上ベクトルが求められます！
	//DirectX::XMQuaternionRotationRollPitchYawFromVector();

	m_goalCamera->m_transform.SetPos(eyePos);

	//m_goalCamera->m_transform.SetUp(result);
	//m_goalCamera->m_transform.SetLookAtRotate(m_goalModelBaseTransform.GetPos());
	//m_goalCamera->m_transform.SetRotate();
	

	KuroEngine::Vec3<float>rightDir(upVec.Cross(frontVec));

	DirectX::XMMATRIX matA = DirectX::XMMatrixIdentity();
	matA.r[0] = { rightDir.x,rightDir.y,rightDir.z,0.0f };
	matA.r[1] = { upVec.x,upVec.y,upVec.z,0.0f };
	matA.r[2] = { frontVec.x,frontVec.y,frontVec.z,0.0f };
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

	//前
	{
		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + frontVec * 5.0f);
		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 0, 255, 255), 1.0f);
	}
	//上
	{
		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + upVec * 5.0f);
		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 255, 0, 255), 1.0f);
	}
	//横
	{
		KuroEngine::Vec3<float>rightDir(upVec.Cross(frontVec));

		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + rightDir * 5.0f);
		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(255, 0, 0, 255), 1.0f);
	}
#endif // _DEBUG

	//KuroEngine::DrawFunc2D::DrawGraph(m_pos, m_clearTex);
	//KuroEngine::DrawFunc3D::DrawNonShadingPlane(m_ddsTex, transform, camera);
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

