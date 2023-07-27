#include"Goal.h"
#include"../OperationConfig.h"
#include"../Player/Player.h"

Goal::Goal(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle) :m_initFlag(false), m_clearEaseTimer(30),
m_upEffectEase(60), m_downEffectEase(10), m_gpuParticleBuffer(particle), m_shake({ 1.0f,1.0f,1.0f }),
m_fireWork(m_gpuParticleBuffer)
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(0.0f, 100.0f);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::Vec2<float>(0.0f, 200.0f);

	m_goalCamera = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");

	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/gameClear.png");

	m_camera = std::make_shared<KuroEngine::Camera>("cameraName");

	m_splineArray[0] = std::make_unique<SplineParticle>(m_gpuParticleBuffer);
	m_splineArray[1] = std::make_unique<SplineParticle>(m_gpuParticleBuffer);

	m_fireWork.SetParticleColorTex(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Particle/goal_particle_gradation.png"));
}

void Goal::Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model)
{
	m_initFlag = true;
	m_startGoalEffectFlag = false;
	m_isStartFlag = false;
	m_prevStartFlag = false;

	m_movieCamera.Init();

	m_clearEaseTimer.Reset();
	m_upEffectEase.Reset();
	m_downEffectEase.Reset();

	m_startCameraFlag = false;

	KuroEngine::Vec3<float>pos = transform.GetPos();
	InitLimitPos(pos);



	m_changeCameraFlag = false;
	m_initParticleFlag = false;


	m_zoomInTimer.Reset(60 * 2);
	m_zoomOutTimer.Reset(60);
	m_sceneChangeTimer.Reset(60);

	//m_glitterEmitt.Finalize();
	m_shake.Init();

	m_goalTexSize = { 1.0f,1.0f };



	m_goalModel = goal_model;


	if (!m_goalModel)
	{
		m_goalBasePos = { 0.0f,0.0f,0.0f };
		m_playerGoalFlag = true;
		return;
	}
	m_playerGoalFlag = false;
	m_goalBasePos = m_goalModel->GetTransform().GetPos();

	//カメラ配置---------------------------------------

	InitCameraPosArray(m_goalBasePos);

	//カメラ配置---------------------------------------
}

void Goal::Finalize()
{
	m_initFlag = false;
}

void Goal::Update(KuroEngine::Transform* transform, Player& arg_player)
{
	if (!m_initFlag)
	{
		return;
	}

	if (m_isStartFlag != m_prevStartFlag)
	{
		if (m_playerGoalFlag)
		{
			m_goalBasePos = transform->GetPos();
			InitCameraPosArray(m_goalBasePos);
		}
		m_prevStartFlag = m_isStartFlag;
	}

	if (m_isStartFlag)
	{
		for (auto &obj : m_splineArray)
		{
			obj->Update();
		}
	}


	//カメラ切り替え
	if (m_isStartFlag && !m_startGoalEffectFlag)
	{
		//プレイヤーと最も近いレイにカメラを配置
		KuroEngine::Vec3<float>pos(transform->GetPos());
		float minDistance = 1000, preMinDistance = 0;
		int index = -1;

		for (int i = 0; i < cameraPosArray.size(); ++i)
		{
			minDistance = std::min(minDistance, pos.Distance(cameraPosArray[i]));
			if (minDistance != preMinDistance)
			{
				index = i;
			}
			preMinDistance = minDistance;
		}

		//カメラが配置出来なかった
		if (index == -1)
		{
			static_assert(1);
		}
		m_goalCamera->m_transform.SetPos(cameraPosArray[index]);
		m_zoomCameraPos = cameraPosArray[index];

		m_shake.Shake(10000.0f, 0.0f, 0.0f, 0.5f);
		m_changeCameraFlag = true;

		//演出開始---------------------------------------
		if (m_playerGoalFlag)
		{
			InitLimitPos(transform->GetPos());
		}

		m_loucusParticle[0].Init(m_splineLimitPos[0]);
		m_loucusParticle[1].Init(m_splineLimitPos[1]);

		m_startGoalEffectFlag = true;
		//演出開始---------------------------------------
	}

	KuroEngine::Vec3<float>distance = m_goalBasePos - m_zoomCameraPos;
	if (m_isStartFlag)
	{
		//段々旗に近づく
		KuroEngine::Vec3<float>pos = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Quint, m_zoomInTimer.GetTimeRate(), m_zoomCameraPos, m_zoomCameraPos + distance * 0.7f);
		m_goalCamera->m_transform.SetPos(pos);

		if (m_zoomInTimer.IsTimeUp())
		{
			KuroEngine::Vec3<float>pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Quint, m_zoomOutTimer.GetTimeRate(), m_zoomCameraPos + distance * 0.5f, m_zoomCameraPos - distance * 1.2f);
			m_goalCamera->m_transform.SetPos(pos);

			m_zoomOutTimer.UpdateTimer();
		}
		m_zoomInTimer.UpdateTimer();
	}


	if (m_isStartFlag && m_zoomInTimer.IsTimeUp())
	{
		float bigScale = 1.0f, smallScale = 0.0f;
		if (!m_initParticleFlag)
		{
			//m_glitterEmitt.Init(m_goalBasePos);
			m_fireWork.Init(m_goalBasePos);
			m_scaleOffset = { bigScale,bigScale,bigScale };
			m_initParticleFlag = true;

			SoundConfig::Instance()->Play(SoundConfig::JINGLE_STAGE_CLEAR);
			m_goalModel->GrowUpWood();
			arg_player.PlantLightExplosion();
		}
		m_scaleOffset = KuroEngine::Math::Lerp(m_scaleOffset, KuroEngine::Vec3<float>(smallScale, smallScale, smallScale), 0.1f);

		if (m_goalModel)
		{
			m_goalModel->m_offset.SetScale(m_scaleOffset);
			m_goalModel->m_offset.SetPos({ 0.0f,0.0f,0.0f });
		}

		m_clearEaseTimer.UpdateTimer();


		if (m_clearEaseTimer.IsTimeUp())
		{
			m_sceneChangeTimer.UpdateTimer();
		}
		if (m_sceneChangeTimer.IsTimeUp())
		{
			m_goalFlag = true;
		}
	}
	else
	{
		if (m_goalModel)
		{
			//旗を揺らす
			m_goalModel->m_offset.SetPos(KuroEngine::Vec3<float>(m_shake.GetOffset().x, 0.0f, m_shake.GetOffset().z));
			m_goalModel->m_offset.SetScale({ 0.0f,0.0f,0.0f });
		}
	}

	//m_glitterEmitt.Update();
	m_fireWork.Update();

	//①視点ベクトル(カメラからオブジェクトまでのベクトル)を求める。
	KuroEngine::Vec3<float>eyePos = m_goalCamera->m_transform.GetPos();
	KuroEngine::Vec3<float>eyeDir = m_goalBasePos - eyePos;
	eyeDir.Normalize();

	//②上ベクトルを固定し、右ベクトルを求める。
	KuroEngine::Vec3<float> rightVec = eyeDir.Cross({ 0,-1,0 });
	rightVec.Normalize();

	//③視点ベクトルと右ベクトルから正しい上ベクトルを求める。
	KuroEngine::Vec3<float> upVec = eyeDir.Cross(rightVec);
	upVec.Normalize();

	KuroEngine::Vec3<float>frontVec = upVec.Cross(rightVec);
	frontVec.Normalize();

	//④求められたベクトルから姿勢を出す。
	DirectX::XMMATRIX matA = DirectX::XMMatrixIdentity();
	matA.r[0] = { rightVec.x,rightVec.y,rightVec.z,0.0f };
	matA.r[1] = { upVec.x,upVec.y,upVec.z,0.0f };
	matA.r[2] = { eyeDir.x,eyeDir.y,eyeDir.z,0.0f };
	m_cameraTransform.SetPos(eyePos);
	m_cameraTransform.SetRotaMatrix(matA);
	m_cameraTransform.CalucuratePosRotaBasedOnWorldMatrix();

	auto &cameraTransform = m_camera->GetTransform();
	cameraTransform.SetParent(&m_cameraTransform);


	m_pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_clearEaseTimer.GetTimeRate(), m_basePos, m_goalPos);
	m_goalTexSize = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_clearEaseTimer.GetTimeRate(), { 0.2f,0.2f }, { 1.0f,1.0f });
	clearTexRadian = KuroEngine::Angle::ConvertToRadian(KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_clearEaseTimer.GetTimeRate(), 0.0f, 360.0f));


	if (!m_zoomInTimer.IsTimeUp())
	{
		m_shake.Update(1.0f);
	}


	for (auto &obj : m_loucusParticle)
	{
		obj.Update();
	}

}

void Goal::Draw(KuroEngine::Camera &camera)
{
	if (!m_initFlag)
	{
		return;
	}

	//for (auto &obj : m_loucusParticle)
	//{
	//	//obj.Draw(camera);
	//}


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

#endif // _DEBUG
	//m_glitterEmitt.Draw(camera);

	//KuroEngine::DrawFunc3D::DrawNonShadingPlane(m_ddsTex, transform, camera);
}

void Goal::Draw2D()
{
	if (m_zoomInTimer.IsTimeUp())
	{
		KuroEngine::DrawFunc2D::DrawRotaGraph2D(m_pos, m_goalTexSize, 0.0f, m_clearTex);
	}
}

bool Goal::IsEnd()
{
	if (m_isStartFlag && m_goalFlag && !m_goalTriggerFlag)
	{
		m_goalTriggerFlag = true;
		return true;
	};
	return false;
}
