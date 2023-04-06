#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/Camera.h"
#include"../../../src/engine/ForUser/Timer.h"
#include"../src/Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"
//#include"../../../src/engine/KuroEngine.h"

struct MovieCameraEaseData
{
	KuroEngine::EASE_CHANGE_TYPE easeChangeType;
	KuroEngine::EASING_TYPE easeType;
};

struct MovieCameraData
{
	KuroEngine::Transform transform;	//制御点の位置
	bool skipInterpolationFlag;			//座標と角度の補間をスキップするかどうか
	int interpolationTimer;				//次の制御点に向かうまでの秒数
	int stopTimer;						//カメラが定位置についてどれくらいの秒数止まるか
	MovieCameraEaseData easeData;		//カメラの補間の仕方
};

class MovieCamera
{
public:
	MovieCamera();
	void Update();
	void StartMovie(KuroEngine::Transform transform, std::vector<MovieCameraData>move_data);

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

		KuroEngine::Transform transform = m_directCameraTransform;
		transform.SetScale({ 5.0f,5.0f,5.0f });
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			transform);

		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			KuroEngine::Transform());

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


	KuroEngine::Vec4<float> ConvertXMVECTORtoVec4(XMVECTOR vec)
	{
		return { vec.m128_f32[0], vec.m128_f32[1] , vec.m128_f32[2] , vec.m128_f32[3] };
	}
	XMVECTOR ConvertVec4toXMVECTOR(const KuroEngine::Vec4<float> &vec)
	{
		return { vec.x, vec.y , vec.z , vec.w };
	}

	KuroEngine::Matrix Ease(const KuroEngine::Matrix &transformA, const KuroEngine::Matrix &transformB, float rate, const MovieCameraEaseData &ease_data)
	{
		std::array<KuroEngine::Vec4<float>, 4>matA;
		matA[0] = ConvertXMVECTORtoVec4(transformA.r[0]);
		matA[1] = ConvertXMVECTORtoVec4(transformA.r[1]);
		matA[2] = ConvertXMVECTORtoVec4(transformA.r[2]);
		matA[3] = ConvertXMVECTORtoVec4(transformA.r[3]);

		std::array<KuroEngine::Vec4<float>, 4>matB;
		matB[0] = ConvertXMVECTORtoVec4(transformB.r[0]);
		matB[1] = ConvertXMVECTORtoVec4(transformB.r[1]);
		matB[2] = ConvertXMVECTORtoVec4(transformB.r[2]);
		matB[3] = ConvertXMVECTORtoVec4(transformB.r[3]);

		std::array<XMVECTOR, 4>result;
		KuroEngine::Matrix mat;
		for (int i = 0; i < matA.size(); ++i)
		{
			result[i] =
				ConvertVec4toXMVECTOR(
					KuroEngine::Math::Ease(
						ease_data.easeChangeType,
						ease_data.easeType,
						rate,
						matA[i],
						matB[i]
					));

			mat.r[i] = result[i];
		}

		return mat;

	};
};

