#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/Camera.h"
#include"../../../src/engine/ForUser/Timer.h"
#include"../src/Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"

struct MovieCameraData
{
	KuroEngine::Vec3<float>pos;			//次の場所に向かう座標
	KuroEngine::Vec3<float>rotation;
	bool skipInterpolationFlag;			//座標と角度の補間をスキップするかどうか
	int interpolationTimer;				//(秒数)
	int stopTimer;						//カメラが定位置についてどれくらい止まるか(秒数)
};

class MovieCamera
{
public:
	MovieCamera();
	void Update();
	void StartMovie(KuroEngine::Vec3<float>camera_pos, KuroEngine::Vec3<float>front_vec, std::vector<MovieCameraData>move_data);

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_cam;
	}

	bool IsStart();
	bool IsFinish();

	/// <summary>
	/// 当たり判定可視化
	/// </summary>
	void DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
	{
#ifdef _DEBUG
		if (!m_startFlag)
		{
			return;
		}
		KuroEngine::Transform transform;
		transform.SetPos(m_moveDataArray[m_moveDataIndex].pos);

		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			transform);

		transform.SetPos(m_moveDataArray[m_moveDataIndex + 1].pos);
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			transform);

		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			m_nowTransform);
#endif // _DEBUG
	};
	KuroEngine::Transform m_directCameraTransform;
private:
	bool m_startFlag, m_finishFlag;
	bool m_stopFlag;
	int m_stopTimer;
	std::shared_ptr<KuroEngine::Camera>m_cam;


	//カメラ補間関連の情報
	int m_moveDataIndex;
	std::vector<MovieCameraData>m_moveDataArray;
	KuroEngine::Transform m_nowTransform;

	std::vector<KuroEngine::Timer> m_timerArray;

	std::shared_ptr<KuroEngine::Model>m_model;
};

