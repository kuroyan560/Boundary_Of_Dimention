#include "EnemyStatus.h"

EnemyKnockBack::EnemyKnockBack()
{
}

void EnemyKnockBack::Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &dir, int time)
{
	m_maxSpeed = 1.0f;
	m_vel = dir * m_maxSpeed;
	m_timer.Reset(time);

	m_timer.UpdateTimer(static_cast<float>(time / 2));
}

KuroEngine::Vec3<float> EnemyKnockBack::Update()
{
	m_timer.UpdateTimer();
	KuroEngine::Vec3<float> vel = m_vel * KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Exp, m_timer.GetTimeRate(), 0.0f, m_maxSpeed);
	return vel;
}

EnemyHeadAttack::EnemyHeadAttack()
{
}

void EnemyHeadAttack::Init(const KuroEngine::Transform &transform, const KuroEngine::Vec3<float> &dir)
{
	m_transform = transform;
	m_dir = dir;
	m_speed = { 2.0f,2.0f,2.0f };
	m_angle = 0.0f;
	m_rotationVec = { 1.0f,0.0f,0.0f };


	//ジャンプ
	m_vel = m_dir * m_speed;
	m_vel += m_transform.GetUp();	//上ベクトルに合わせて移動量を見る
	m_gravity = {};
	m_basePos = m_transform.GetPos();

	m_initTransform = m_transform;
	m_isHitGroundFlag = false;


	float timer = 50.0f;
	float length = 10.0f;
	float height = 10.0f;


	KuroEngine::Vec3<float>startPos(transform.GetPos()), endPos(transform.GetPos() + dir * length);


	m_limitPosArray.clear();
	m_limitPosArray.emplace_back(startPos);
	m_limitPosArray.emplace_back(startPos);

	KuroEngine::Vec3<float>distance(endPos - startPos);
	KuroEngine::Vec3<float>yPos(startPos + distance / 2.0f + KuroEngine::Vec3<float>(0.0f, height, 0.0f));
	m_limitPosArray.emplace_back(yPos);

	m_limitPosArray.emplace_back(endPos);
	m_limitPosArray.emplace_back(endPos);

	m_splineIndex = 1;


	m_initFlag = false;



	m_timer = timer;
	m_splineTimer.Reset(m_timer);
	m_blowTimer.Reset(static_cast<float>((m_limitPosArray.size() - 2) * m_timer));


	m_nowPos = SplinePosition(m_limitPosArray, m_splineIndex, m_splineTimer.GetTimeRate(), false);
	m_prevPos = m_nowPos;
}

HeadAttackData EnemyHeadAttack::Update(const KuroEngine::Vec3<float> &pos)
{
	//ヒットエフェクト


	//重量ありのジャンプ
	if (m_splineTimer.UpdateTimer())
	{
		++m_splineIndex;
		m_splineTimer.Reset(m_timer);
	}
	if (m_limitPosArray.size() - 2 <= m_splineIndex)
	{
		m_initFlag = true;
	}
	else
	{
		m_nowPos = SplinePosition(m_limitPosArray, m_splineIndex, m_splineTimer.GetTimeRate(), false);
	}

	//m_gravity += m_transform.GetUp() * 0.1f;
	//m_vel += -m_gravity;

	//float getBasePos = 0.0f;
	//if (1.0f <= abs(m_initTransform.GetUp().x))
	//{
	//	getBasePos = m_initTransform.GetPos().x;
	//}
	//else if (1.0f <= abs(m_initTransform.GetUp().y))
	//{
	//	getBasePos = m_initTransform.GetPos().y;
	//}
	//else if (1.0f <= abs(m_initTransform.GetUp().z))
	//{
	//	getBasePos = m_initTransform.GetPos().z;
	//}
	////地面に着地したらエフェクト
	//if (pos.y <= getBasePos)
	//{
	//	m_isHitGroundFlag = true;
	//}
	m_angle += 15.0f;
	HeadAttackData headAttackData;
	headAttackData.m_dir = m_nowPos - m_prevPos;
	headAttackData.m_rotation = DirectX::XMQuaternionRotationAxis(m_rotationVec, KuroEngine::Angle::ConvertToRadian(m_angle));

	m_prevPos = m_nowPos;
	return headAttackData;
}

void EnemyHeadAttack::ParticleUpdate()
{
	//着地パーティクル
	if (m_initFlag && !m_initTriggerFlag)
	{
		m_hitGroundEffecct.Init(m_nowPos);
	}
	m_initTriggerFlag = m_initFlag;
	m_hitGroundEffecct.Update();
	//着地パーティクル
}

void EnemyHeadAttack::ParticleDraw(KuroEngine::Camera &camera)
{
	//着地パーティクル
	m_hitGroundEffecct.Draw(camera);
	//着地パーティクル
}

KuroEngine::Vec3<float> EnemyHeadAttack::SplinePosition(const std::vector<KuroEngine::Vec3<float>> &points, size_t startIndex, float t, bool Loop)
{
	if (startIndex < 1)
	{
		return points[1];
	}
	DirectX::XMVECTOR p0 = ConvertVec3toXMVECTOR(points[startIndex - 1]);
	DirectX::XMVECTOR p1 = ConvertVec3toXMVECTOR(points[startIndex]);
	DirectX::XMVECTOR p2;
	DirectX::XMVECTOR p3;

	size_t subIndex = 3;
	if (Loop == true)
	{
		if (startIndex > points.size() - subIndex)
		{
			p2 = ConvertVec3toXMVECTOR(points[1]);
			p3 = ConvertVec3toXMVECTOR(points[2]);
		}
		else
		{
			p2 = ConvertVec3toXMVECTOR(points[startIndex + 1]);
			p3 = ConvertVec3toXMVECTOR(points[startIndex + 2]);
		}
	}
	else
	{
		int size = static_cast<int>(points.size());
		if (startIndex > size - 3)return points[size - 3];
		p2 = ConvertVec3toXMVECTOR(points[startIndex + 1]);
		p3 = ConvertVec3toXMVECTOR(points[startIndex + 2]);
	}
	using namespace DirectX;
	DirectX::XMVECTOR anser2 =
		0.5 * ((2 * p1 + (-p0 + p2) * t) +
			(2 * p0 - 5 * p1 + 4 * p2 - p3) * (t * t) +
			(-p0 + 3 * p1 - 3 * p2 + p3) * (t * t * t));


	KuroEngine::Vec3<float>result = { anser2.m128_f32[0],anser2.m128_f32[1],anser2.m128_f32[2] };
	return result;
};