#include"EnemySearch.h"
#include"../Graphics/BasicDraw.h"

SightSearch::SightSearch()
{
	KuroEngine::Vec3<float>basePos(0.0f, 0.0f, 0.0f);
	float nearScale = 1.0f;
	float farScale = 6.0f;
	float length = 25.0f;
	float nearHeight = 1.0f;
	float farHeight = 5.0f;

	int index = 0;
	m_rayArray.emplace_back();
	//敵の正面ベクトル
	{
		m_rayArray[index].m_startPos = basePos;
		m_rayArray[index].m_length = length;
		KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(0.0f, 0.0f, length));
		KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
		distance.Normalize();
		m_rayArray[index].m_dir = distance;
	}

	for (auto &obj : m_rayArray)
	{
		m_sightRay.emplace_back();
		m_sightRay.back().ray = obj;
	}

	m_sightRay.emplace_back();
}

void SightSearch::Init(
	KuroEngine::Transform *transform
)
{
	m_transformPtr = transform;
	//InitSight({ 0.0f, 0.0f, 0.0f }, length, nearScale, nearHeight, farScale, farHeight);
}

bool SightSearch::IsFind(const KuroEngine::Vec3<float> &pos, float viewAngle)
{
	bool findFlag = true;
	for (int i = 0; i < m_rayArray.size(); ++i)
	{
		//親子関係にもとづいて判定を動かす。

		KuroEngine::Transform hitStartPos;
		KuroEngine::Transform hitEndPos;
		hitStartPos.SetParent(m_transformPtr);
		hitEndPos.SetParent(m_transformPtr);
		hitStartPos.SetPos(m_rayArray[i].m_startPos);
		hitEndPos.SetPos(m_rayArray[i].GetEndPos());

		//正面ベクトル
		m_sightRay[i].ray.m_startPos = hitStartPos.GetPosWorld();
		m_sightRay[i].ray.m_dir = KuroEngine::Vec3<float>(hitEndPos.GetPosWorld() - hitStartPos.GetPosWorld()).GetNormal();

		//プレイヤーとのベクトル
		KuroEngine::Vec3<float>playerVec(pos - hitStartPos.GetPosWorld());
		const float distance = pos.Distance(hitStartPos.GetPosWorld());
		playerVec.Normalize();

		//内積の衝突判定
		float dot = m_sightRay[i].ray.m_dir.Dot(playerVec);


		float rate = 1.0f - (viewAngle / 180.0f);
		if (-dot <= -rate && distance <= m_sightRay[i].ray.m_length * m_transformPtr[i].GetScale().Length())
		{
			m_sightRay[i].hitFlag = true;
		}
		else
		{
			m_sightRay[i].hitFlag = false;
			findFlag = false;
		}

		m_sightRay.back().ray.m_startPos = hitStartPos.GetPosWorld();
		m_sightRay.back().ray.m_dir = playerVec;
		m_sightRay.back().ray.m_length = m_sightRay[0].ray.m_length;

	}
	return findFlag;
}

void SightSearch::DebugDraw(KuroEngine::Camera &camera)
{
	for (auto &obj : m_sightRay)
	{
		if (obj.hitFlag)
		{
			KuroEngine::DrawFunc3D::DrawLine(camera, obj.ray.m_startPos, obj.ray.GetEndPos(), KuroEngine::Color(255, 0, 0, 255), 1.0f);
		}
		else
		{
			KuroEngine::DrawFunc3D::DrawLine(camera, obj.ray.m_startPos, obj.ray.GetEndPos(), KuroEngine::Color(255, 255, 255, 255), 1.0f);
		}
	}
}


CircleSearch::CircleSearch()
{
	m_hitBoxModel =
		KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Shpere.glb");
}

void CircleSearch::Init(const Sphere &sphere)
{
	m_hitBox = sphere;
}

bool CircleSearch::IsFind(const Sphere &sphere)
{
	return Collision::Instance()->CheckCircleAndCircle(sphere, m_hitBox);
}

void CircleSearch::DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	KuroEngine::Transform transform;
	transform.SetPos(*m_hitBox.m_centerPos);
	transform.SetScale(*m_hitBox.m_radius);

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);
	BasicDraw::Instance()->Draw_NoGrass(
		arg_cam,
		arg_ligMgr,
		m_hitBoxModel,
		transform,
		edgeColor,
		KuroEngine::Color(1.0f, 0.0f, 0.0f, 0.5f)
	);
}
