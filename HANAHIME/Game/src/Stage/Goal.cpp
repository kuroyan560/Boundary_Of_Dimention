#include"Goal.h"
#include"../OperationConfig.h"

Goal::Goal() :m_initFlag(false), m_clearEaseTimer(30),
m_upEffectEase(60), m_downEffectEase(10), m_splineTimer(120)
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinSize().y);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::Vec2<float>(0.0f, KuroEngine::WinApp::Instance()->GetExpandWinCenter().y / 2.0f);

	m_goalCamera = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");

	for (auto &obj : limitPosArray)
	{
		obj = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");
	}

	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/gameClear.png");

	m_camera = std::make_shared<KuroEngine::Camera>("cameraName");

	GenerateLoucus();

}

void Goal::Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model)
{
	m_initFlag = true;
	m_startGoalEffectFlag = false;
	m_isStartFlg = false;

	m_movieCamera.Init();

	m_clearEaseTimer.Reset();
	m_upEffectEase.Reset();
	m_downEffectEase.Reset();

	//m_goalModel = goal_model;
	//m_goalModelBaseTransform = goal_model->GetTransform();
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

	//if (m_isStartFlg && false)
	//{
	//	float upEffectRate = m_upEffectEase.GetTimeRate();

	//	//ゴールモデルの演出
	//	KuroEngine::Transform transform;
	//	//回転
	//	KuroEngine::Vec3<float>rotaVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, upEffectRate, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,360.0f });
	//	transform.SetRotate(rotaVel);
	//	//上に上げる
	//	KuroEngine::Vec3<float>upVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, upEffectRate, m_goalModelBaseTransform.GetPos(), m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f));
	//	transform.SetPos(upVel);

	//	m_upEffectEase.UpdateTimer();
	//	if (m_upEffectEase.IsTimeUp())
	//	{
	//		//下に下げる
	//		KuroEngine::Vec3<float>downVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_downEffectEase.GetTimeRate(), m_goalModelBaseTransform.GetPos() + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), m_goalModelBaseTransform.GetPos());
	//		transform.SetPos(downVel);
	//		m_downEffectEase.UpdateTimer();
	//	}
	//	if (m_downEffectEase.IsTimeUp())
	//	{
	//		m_startCameraFlag = true;
	//	}

	//	m_goalModel->SetTransform(transform);
	//}

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
	//
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


	std::array<DirectX::XMFLOAT3, LIMIT_POS_MAX>posArray;
	posArray[0] = { 0.0f,10.0f,0.0f };
	posArray[1] = { 10.0f,10.0f,0.0f };
	posArray[2] = { 10.0f,15.0f,0.0f };
	posArray[3] = { 15.0f,15.0f,0.0f };
	posArray[4] = { 15.0f,20.0f,0.0f };
	posArray[5] = { 15.0f,20.0f,10.0f };
	posArray[6] = { 15.0f,-20.0f,-10.0f };
	posArray[7] = { 10.0f,10.0f,0.0f };
	posArray[8] = { 0.0f,10.0f,0.0f };
	posArray[9] = { 0.0f,10.0f,0.0f };

	for (auto &obj : limitPosArray)
	{
		int index = static_cast<int>(&obj - &limitPosArray[0]);
		obj->m_transform.SetPos(
			{
				posArray[index].x,
				posArray[index].y,
				posArray[index].z
			}
		);
	}


	UINT num = static_cast<UINT>(posArray.size());
	m_limitIndexBuffer->Mapping(&num);
	m_limitIndexPosBuffer->Mapping(posArray.data());

	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) && false)
	{
		std::vector<KuroEngine::RegisterDescriptorData>descData =
		{
			{m_particleBuffer,KuroEngine::UAV},
			{m_limitIndexPosBuffer,KuroEngine::UAV},
			{m_limitIndexBuffer,KuroEngine::CBV},
		};
		KuroEngine::D3D12App::Instance()->DispathOneShot(m_initLoucusPipeline, { 1,1,1 }, descData);
	}
	cd.scaleRotate = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
	if (m_splineTimer.IsTimeUp())
	{
		++cd.startIndex;
		m_splineTimer.Reset();
	}
	if (posArray.size() <= cd.startIndex)
	{
		cd.startIndex = 0;
	}
	cd.rate = m_splineTimer.GetTimeRate();
	m_splineTimer.UpdateTimer();
	m_scaleRotaBuffer->Mapping(&cd);


	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_particleBuffer,KuroEngine::UAV},
		{m_gpuParticleBuffer,KuroEngine::UAV},
		{m_scaleRotaBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_updateLoucusPipeline, { 1,1,1 }, descData);



	//ゴールカメラモード
	if (m_isStartFlg && !m_startGoalEffectFlag)
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
	if (m_isStartFlg)
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
	//KuroEngine::DrawFunc3D::DrawNonShadingModel(m_goalCamera, camera);

	//if (!m_isStartFlg)
	//{
	//	//前
	//	{
	//		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
	//		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + frontVec * 5.0f);
	//		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 0, 255, 255), 1.0f);
	//	}
	//	//上
	//	{
	//		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
	//		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + upVec * 5.0f);
	//		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 255, 0, 255), 1.0f);
	//	}
	//	//横
	//	{
	//		KuroEngine::Vec3<float>rightDir(upVec.Cross(frontVec));

	//		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
	//		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + rightDir * 5.0f);
	//		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(255, 0, 0, 255), 1.0f);
	//	}
	//}
	//KuroEngine::Vec3<float> result(0, 1, 0);
	//DirectX::XMMATRIX mat = DirectX::XMMatrixRotationQuaternion(m_goalModel->GetTransform().GetRotate());
	//DirectX::XMVECTOR rota(DirectX::XMVector3Transform(result, mat));
	//result = { rota.m128_f32[0],rota.m128_f32[1],rota.m128_f32[2] };
	//result.Normalize();

	//KuroEngine::Vec3<float>startPos(m_goalModelBaseTransform.GetPos());
	//KuroEngine::Vec3<float>endPos(m_goalModelBaseTransform.GetPos() + result * 5.0f);
	//KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(255, 0, 0, 255), 1.0f);

	//for (auto &obj : limitPosArray)
	//{
	//	KuroEngine::DrawFunc3D::DrawNonShadingModel(obj, camera);
	//}

#endif // _DEBUG

	KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_pos, { 1.0f,1.0f }, clearTexRadian, m_clearTex);
	//KuroEngine::DrawFunc3D::DrawNonShadingPlane(m_ddsTex, transform, camera);
}

bool Goal::IsEnd()
{
	return m_isStartFlg && m_movieCamera.IsFinish();
}

