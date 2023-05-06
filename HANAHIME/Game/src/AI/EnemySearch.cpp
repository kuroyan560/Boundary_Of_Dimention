#include"EnemySearch.h"

SightSearch::SightSearch()
{
	m_rayArray.emplace_back();
	KuroEngine::Vec3<float>basePos(0.0f, 0.0f, 0.0f);
	float nearScale = 1.0f;
	float farScale = 6.0f;
	float length = 15.0f;
	float nearHeight = 1.0f;
	float farHeight = 5.0f;

	int index = 0;
	//右上
	{
		m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, nearHeight, 0.0f);
		m_rayArray[index].m_length = length;
		KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, farHeight, length));
		KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
		distance.Normalize();
		m_rayArray[index].m_dir = distance;
	}

	m_rayArray.emplace_back();
	++index;
	//左上
	{
		m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, nearHeight, 0.0f);
		m_rayArray[index].m_length = length;
		KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, farHeight, length));
		KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
		distance.Normalize();
		m_rayArray[index].m_dir = distance;
	}

	m_rayArray.emplace_back();
	++index;
	//右下
	{
		m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, -nearHeight, 0.0f);
		m_rayArray[index].m_length = length;
		KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, -farHeight, length));
		KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
		distance.Normalize();
		m_rayArray[index].m_dir = distance;
	}

	m_rayArray.emplace_back();
	++index;
	//左下
	{
		m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, -nearHeight, 0.0f);
		m_rayArray[index].m_length = length;
		KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, -farHeight, length));
		KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
		distance.Normalize();
		m_rayArray[index].m_dir = distance;
	}

	for (auto &obj : m_rayArray)
	{
		m_sightRay.emplace_back();
		m_sightRay.back().ray = obj;
	}
}

void SightSearch::Init(
	float nearScale,
	float farScale,
	float length,
	float nearHeight,
	float farHeight
)
{
	InitSight({ 0.0f, 0.0f, 0.0f }, length, nearScale, nearHeight, farScale, farHeight);
}

bool SightSearch::IsFind(const KuroEngine::Vec3<float> &pos, KuroEngine::Transform *rotation)
{

	bool findFlag = true;
	for (int i = 0; i < m_rayArray.size(); ++i)
	{
		//親子関係にもとづいて判定を動かす。

		KuroEngine::Transform hitStartPos;
		KuroEngine::Transform hitEndPos;
		hitStartPos.SetParent(rotation);
		hitEndPos.SetParent(rotation);
		hitStartPos.SetPos(m_rayArray[i].m_startPos);
		hitEndPos.SetPos(m_rayArray[i].GetEndPos());

		m_sightRay[i].ray.m_startPos = hitStartPos.GetPosWorld();
		m_sightRay[i].ray.m_dir = KuroEngine::Vec3<float>(hitEndPos.GetPosWorld() - hitStartPos.GetPosWorld()).GetNormal();

		//当たり判定
		float cross = collisionDetection.CaluCross(hitStartPos.GetPosWorld(), hitEndPos.GetPosWorld(), pos);

		//点が線の中に入っている事の確認
		if (cross <= 0.0f)
		{
			m_sightRay[i].hitFlag = true;
		}
		else
		{
			m_sightRay[i].hitFlag = false;
			findFlag = false;
		}
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
	float distance = sphere.m_centerPos->Distance(*m_hitBox.m_centerPos);
	float sumRadist = *m_hitBox.m_radius + *sphere.m_radius;
	return (distance <= sumRadist);
}

void CircleSearch::DebugDraw(KuroEngine::Camera &camera)
{
	KuroEngine::Transform transform;
	transform.SetPos(*m_hitBox.m_centerPos);
	transform.SetScale(*m_hitBox.m_radius);
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_hitBoxModel,
		transform.GetMatWorld(),
		camera,
		0.5f
	);
}
