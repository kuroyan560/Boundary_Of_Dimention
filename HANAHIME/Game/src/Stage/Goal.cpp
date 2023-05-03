#include"Goal.h"
#include"../OperationConfig.h"

Goal::Goal(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle) :m_initFlag(false), m_clearEaseTimer(30),
m_upEffectEase(60), m_downEffectEase(10), m_gpuParticleBuffer(particle), m_shake({ 1.0f,1.0f,1.0f })
{
	m_startGoalEffectFlag = false;

	m_basePos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() + KuroEngine::Vec2<float>(0.0f, 100.0f);
	m_goalPos = KuroEngine::WinApp::Instance()->GetExpandWinCenter() - KuroEngine::Vec2<float>(0.0f, 200.0f);

	m_goalCamera = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");

	m_clearTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/gameClear.png");

	m_camera = std::make_shared<KuroEngine::Camera>("cameraName");


	m_splineArray[0] = std::make_unique<SplineParticle>(m_gpuParticleBuffer);
	m_splineArray[1] = std::make_unique<SplineParticle>(m_gpuParticleBuffer);
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

	m_glitterEmitt.Finalize();
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

	//�J�����z�u---------------------------------------

	InitCameraPosArray(m_goalBasePos);

	//�J�����z�u---------------------------------------
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

	if (m_isStartFlag != m_prevStartFlag)
	{
		SoundConfig::Instance()->Play(SoundConfig::JINGLE_STAGE_CLEAR);
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


	//�J�����؂�ւ�
	if (m_isStartFlag && !m_startGoalEffectFlag)
	{
		//�v���C���[�ƍł��߂����C�ɃJ������z�u
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

		//�J�������z�u�o���Ȃ�����
		if (index == -1)
		{
			static_assert(1);
		}
		m_goalCamera->m_transform.SetPos(cameraPosArray[index]);
		m_zoomCameraPos = cameraPosArray[index];

		m_shake.Shake(10000.0f, 0.0f, 0.0f, 0.5f);
		m_changeCameraFlag = true;

		//���o�J�n---------------------------------------
		if (m_playerGoalFlag)
		{
			InitLimitPos(transform->GetPos());
		}

		m_loucusParticle[0].Init(m_splineLimitPos[0]);
		m_loucusParticle[1].Init(m_splineLimitPos[1]);

		m_startGoalEffectFlag = true;
		//���o�J�n---------------------------------------
	}

	KuroEngine::Vec3<float>distance = m_goalBasePos - m_zoomCameraPos;
	if (m_isStartFlag)
	{
		//�i�X���ɋ߂Â�
		KuroEngine::Vec3<float>pos = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Quint, m_zoomInTimer.GetTimeRate(), m_zoomCameraPos, m_zoomCameraPos + distance * 0.7f);
		m_goalCamera->m_transform.SetPos(pos);

		if (m_zoomInTimer.IsTimeUp())
		{
			KuroEngine::Vec3<float>pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Quint, m_zoomOutTimer.GetTimeRate(), m_zoomCameraPos + distance * 0.5f, m_zoomCameraPos - distance * 1.0f);
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
			m_glitterEmitt.Init(m_goalBasePos);
			m_scaleOffset = { bigScale,bigScale,bigScale };
			m_initParticleFlag = true;
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
			//����h�炷
			m_goalModel->m_offset.SetPos(KuroEngine::Vec3<float>(m_shake.GetOffset().x, 0.0f, m_shake.GetOffset().z));
			m_goalModel->m_offset.SetScale({ 0.0f,0.0f,0.0f });
		}
	}

	m_glitterEmitt.Update();


	//�@���_�x�N�g��(�J��������I�u�W�F�N�g�܂ł̃x�N�g��)�����߂�B
	KuroEngine::Vec3<float>eyePos = m_goalCamera->m_transform.GetPos();
	KuroEngine::Vec3<float>eyeDir = m_goalBasePos - eyePos;
	eyeDir.Normalize();

	//�A��x�N�g�����Œ肵�A�E�x�N�g�������߂�B
	KuroEngine::Vec3<float> rightVec = eyeDir.Cross({ 0,-1,0 });
	rightVec.Normalize();

	//�B���_�x�N�g���ƉE�x�N�g�����琳������x�N�g�������߂�B
	KuroEngine::Vec3<float> upVec = eyeDir.Cross(rightVec);
	upVec.Normalize();

	KuroEngine::Vec3<float>frontVec = upVec.Cross(rightVec);
	frontVec.Normalize();

	//�C���߂�ꂽ�x�N�g������p�����o���B
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

	for (auto &obj : m_loucusParticle)
	{
		obj.Draw(camera);
	}


#ifdef _DEBUG
	//KuroEngine::DrawFunc3D::DrawNonShadingModel(m_goalCamera, camera);

	//if (!m_isStartFlg)
	//{
	//	//�O
	//	{
	//		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
	//		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + frontVec * 5.0f);
	//		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 0, 255, 255), 1.0f);
	//	}
	//	//��
	//	{
	//		KuroEngine::Vec3<float>startPos(m_goalCamera->m_transform.GetPos());
	//		KuroEngine::Vec3<float>endPos(m_goalCamera->m_transform.GetPos() + upVec * 5.0f);
	//		KuroEngine::DrawFunc3D::DrawLine(camera, startPos, endPos, KuroEngine::Color(0, 255, 0, 255), 1.0f);
	//	}
	//	//��
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
	m_glitterEmitt.Draw(camera);

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

PlayerCollision::MeshCollisionOutput Goal::MeshCollision(const KuroEngine::Vec3<float> &arg_rayPos, const KuroEngine::Vec3<float> &arg_rayDir, std::vector<TerrianHitPolygon> &arg_targetMesh)
{
	/*===== ���b�V���ƃ��C�̓����蔻�� =====*/


	/*-- �@ �|���S����@���������ƂɃJ�����O���� --*/

	//�@���ƃ��C�̕����̓��ς�0���傫�������ꍇ�A���̃|���S���͔w�ʂȂ̂ŃJ�����O����B
	for (auto &index : arg_targetMesh) {

		index.m_isActive = true;

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- �A �|���S���ƃ��C�̓����蔻����s���A�e�����L�^���� --*/

	// �L�^�p�f�[�^
	std::vector<std::pair<PlayerCollision::MeshCollisionOutput, TerrianHitPolygon>> hitDataContainer;

	for (auto &index : arg_targetMesh) {

		//�|���S��������������Ă����玟�̏�����
		if (!index.m_isActive) continue;

		//���C�̊J�n�n�_���畽�ʂɂ��낵�������̒��������߂�
		//KuroEngine::Vec3<float> planeNorm = -index.m_p0.normal;
		KuroEngine::Vec3<float> planeNorm = KuroEngine::Vec3<float>(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p2.pos).GetNormal()).Cross(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p1.pos).GetNormal());
		float rayToOriginLength = arg_rayPos.Dot(planeNorm);
		float planeToOriginLength = index.m_p0.pos.Dot(planeNorm);
		//���_���畽�ʂɂ��낵�������̒���
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		//�O�p�֐��𗘗p���Ď��_����Փ˓_�܂ł̋��������߂�
		float dist = planeNorm.Dot(arg_rayDir);
		float impDistance = perpendicularLine / -dist;

		if (std::isnan(impDistance))continue;

		//�Փ˒n�_
		KuroEngine::Vec3<float> impactPoint = arg_rayPos + arg_rayDir * impDistance;

		/*----- �Փ˓_���|���S���̓����ɂ��邩�𒲂ׂ� -----*/

		/* ��1�{�� */
		KuroEngine::Vec3<float> P1ToImpactPos = (impactPoint - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP2 = (index.m_p1.pos - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP3 = (index.m_p2.pos - index.m_p0.pos).GetNormal();

		//�Փ˓_�ƕ�1�̓���
		float impactDot = P1ToImpactPos.Dot(P1ToP2);
		//�_1�Ɠ_3�̓���
		float P1Dot = P1ToP2.Dot(P1ToP3);

		//�Փ˓_�ƕ�1�̓��ς��_1�Ɠ_3�̓��ς�菬����������A�E�g
		if (impactDot < P1Dot) {
			index.m_isActive = false;
			continue;
		}

		/* ��2�{�� */
		KuroEngine::Vec3<float> P2ToImpactPos = (impactPoint - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP3 = (index.m_p2.pos - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP1 = (index.m_p0.pos - index.m_p1.pos).GetNormal();

		//�Փ˓_�ƕ�2�̓���
		impactDot = P2ToImpactPos.Dot(P2ToP3);
		//�_2�Ɠ_1�̓���
		float P2Dot = P2ToP3.Dot(P2ToP1);

		//�Փ˓_�ƕ�2�̓��ς��_2�Ɠ_1�̓��ς�菬����������A�E�g
		if (impactDot < P2Dot) {
			index.m_isActive = false;
			continue;
		}

		/* ��3�{�� */
		KuroEngine::Vec3<float> P3ToImpactPos = (impactPoint - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP1 = (index.m_p0.pos - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP2 = (index.m_p1.pos - index.m_p2.pos).GetNormal();

		//�Փ˓_�ƕ�3�̓���
		impactDot = P3ToImpactPos.Dot(P3ToP1);
		//�_3�Ɠ_2�̓���
		float P3Dot = P3ToP1.Dot(P3ToP2);

		//�Փ˓_�ƕ�3�̓��ς��_3�Ɠ_2�̓��ς�菬����������A�E�g
		if (impactDot < P3Dot) {
			index.m_isActive = false;
			continue;
		}

		/* �����܂ŗ�����|���S���ɏՓ˂��Ă�I */
		PlayerCollision::MeshCollisionOutput data;
		data.m_isHit = true;
		data.m_pos = impactPoint;
		data.m_distance = impDistance;
		data.m_normal = index.m_p0.normal;
		hitDataContainer.emplace_back(std::pair(data, index));

	}


	/*-- �B �L�^������񂩂�ŏI�I�ȏՓ˓_�����߂� --*/

	//hitPorygon�̒l��1�ȏゾ�����狗�����ŏ��̗v�f������
	if (0 < hitDataContainer.size()) {

		//�������ŏ��̗v�f������
		int min = 0;
		float minDistance = std::numeric_limits<float>().max();
		for (auto &index : hitDataContainer) {
			if (fabs(index.first.m_distance) < fabs(minDistance)) {
				minDistance = index.first.m_distance;
				min = static_cast<int>(&index - &hitDataContainer[0]);
			}
		}

		//�d�S���W�����߂�B
		KuroEngine::Vec3<float> bary = CalBary(hitDataContainer[min].second.m_p0.pos, hitDataContainer[min].second.m_p1.pos, hitDataContainer[min].second.m_p2.pos, hitDataContainer[min].first.m_pos);

		KuroEngine::Vec3<float> baryBuff = bary;

		//UVW�̒l�������̂ŏC���B
		bary.x = baryBuff.y;
		bary.y = baryBuff.z;
		bary.z = baryBuff.x;

		KuroEngine::Vec2<float> uv = KuroEngine::Vec2<float>();

		//�d�S���W����UV�����߂�B
		uv.x += hitDataContainer[min].second.m_p0.uv.x * bary.x;
		uv.x += hitDataContainer[min].second.m_p1.uv.x * bary.y;
		uv.x += hitDataContainer[min].second.m_p2.uv.x * bary.z;

		uv.y += hitDataContainer[min].second.m_p0.uv.y * bary.x;
		uv.y += hitDataContainer[min].second.m_p1.uv.y * bary.y;
		uv.y += hitDataContainer[min].second.m_p2.uv.y * bary.z;

		hitDataContainer[min].first.m_uv = uv;

		return hitDataContainer[min].first;
	}
	else {

		return PlayerCollision::MeshCollisionOutput();

	}


}

KuroEngine::Vec3<float> Goal::CalBary(const KuroEngine::Vec3<float> &PosA, const KuroEngine::Vec3<float> &PosB, const KuroEngine::Vec3<float> &PosC, const KuroEngine::Vec3<float> &TargetPos)
{

	/*===== �d�S���W�����߂� =====*/

	KuroEngine::Vec3<float> uvw = KuroEngine::Vec3<float>();

	// �O�p�`�̖ʐς����߂�B
	float areaABC = (PosC - PosA).Cross(PosB - PosA).Length() / 2.0f;

	// �d�S���W�����߂�B
	uvw.x = ((PosA - TargetPos).Cross(PosB - TargetPos).Length() / 2.0f) / areaABC;
	uvw.y = ((PosB - TargetPos).Cross(PosC - TargetPos).Length() / 2.0f) / areaABC;
	uvw.z = ((PosC - TargetPos).Cross(PosA - TargetPos).Length() / 2.0f) / areaABC;

	return uvw;

}
