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

	if (m_isHitFlag && false)
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
	KuroEngine::Transform transformA = m_goalModelBaseTransform;
	transformA.SetRotate(m_goalModelBaseTransform.GetRotate());

	m_goalModel->SetTransform(transformA);


	if (KuroEngine::UsersInput::Instance()->KeyInput(DIK_H))
	{
		m_radius = 5.0f;
		m_height = -5.0f;
		float height = m_height + m_heightTimer.GetTimeRate() * 13.0f;
		KuroEngine::Vec3<float>heightVec3(height, height, height);
		heightVec3 *= GetFlagUpVec();

		for (auto &emitter : m_emitter)
		{
			emitter.m_angle = emitter.m_baseAngle + 360 * emitter.m_timer.GetTimeRate();


			KuroEngine::Vec3<float>vel(1.0f, 1.0f, 1.0f);
			vel *= GetFlagUpVec();
			if (vel.x <= 0.0f)
			{
				vel.x = 1.0f;
			}
			if (vel.y <= 0.0f)
			{
				vel.y = 1.0f;
			}
			if (vel.z <= 0.0f)
			{
				vel.z = 1.0f;
			}

			KuroEngine::Vec3<float>circleVel(
				heightVec3.x + cosf(KuroEngine::Angle::ConvertToRadian(emitter.m_angle)) * m_radius,
				heightVec3.y,
				heightVec3.z + sinf(KuroEngine::Angle::ConvertToRadian(emitter.m_angle)) * m_radius);

			emitter.m_emittPos = m_goalModelBaseTransform.GetPos() + circleVel * vel;

			emitter.m_timer.UpdateTimer();
		}

		for (int i = 0; i < m_emitter.size(); ++i)
		{
			m_emittObject[i]->m_transform.SetPos(m_emitter[i].m_emittPos);
		}

		m_heightTimer.UpdateTimer();
	}
	else
	{
		m_heightTimer.Reset();
		for (auto &emitter : m_emitter)
		{
			emitter.m_timer.Reset();
		}
	}

	for (auto &obj : m_particleArray)
	{
		obj.Update();
	}


	////①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
	//KuroEngine::Vec3<float>eyePos = m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 0.0f, 30.0f);
	//KuroEngine::Vec3<float>eyeDir = m_goalModelBaseTransform.GetPos() - eyePos;
	//frontVec = eyeDir;
	//eyeDir.Normalize();
	//frontVec.Normalize();

	////②視点ベクトルと(0, 0, 1)ベクトルを外積して法線を求める。
	//KuroEngine::Vec3<float>eyeNormal = eyeDir.Cross(KuroEngine::Vec3<float>(0.0f, 0.0f, 1.0f));
	//eyeNormal.Normalize();
	////③視点ベクトルと(0, 0, 1)ベクトルを内積して回転量を求める。
	//float eyeRota = eyeDir.Dot(KuroEngine::Vec3<float>(0.0f, 0.0f, 1.0f));
	//eyeRota = acos(eyeRota);
	//if (std::isnan(eyeRota))
	//{
	//	eyeRota = 0.0f;
	//}

	////④クォータニオンの関数で②を回転軸として③回転するクォータニオンを求める。
	//KuroEngine::Quaternion quaternion = DirectX::XMQuaternionRotationNormal(eyeNormal, eyeRota);

	////⑤④で求めれたクォータニオンを(0, 1, 0)ベクトルにかける。
	//KuroEngine::Vec3<float> result(0, 1, 0);
	//DirectX::XMMATRIX mat = DirectX::XMMatrixRotationQuaternion(quaternion);
	//DirectX::XMVECTOR rota(DirectX::XMVector3Transform(result, mat));
	//result = { rota.m128_f32[0],rota.m128_f32[1],rota.m128_f32[2] };

	//upVec = result;
	//upVec.Normalize();

	////これで上ベクトルが求められます！
	////DirectX::XMQuaternionRotationRollPitchYawFromVector();

	//m_goalCamera->m_transform.SetPos(eyePos);

	////m_goalCamera->m_transform.SetUp(result);
	////m_goalCamera->m_transform.SetLookAtRotate(m_goalModelBaseTransform.GetPos());
	////m_goalCamera->m_transform.SetRotate();
	//

	//KuroEngine::Vec3<float>rightDir(upVec.Cross(frontVec));

	////DirectX::XMMATRIX matA = DirectX::XMMatrixIdentity();
	////matA.r[0] = { rightDir.x,rightDir.y,rightDir.z,0.0f };
	////matA.r[1] = { upVec.x,upVec.y,upVec.z,0.0f };
	////matA.r[2] = { frontVec.x,frontVec.y,frontVec.z,0.0f };
	////m_goalCamera->m_transform.SetRotaMatrix(matA);


	//auto &c = m_camera->GetTransform();
	//c.SetParent(&m_goalCamera->m_transform);



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
	KuroEngine::DrawFunc3D::DrawNonShadingModel(, );
}