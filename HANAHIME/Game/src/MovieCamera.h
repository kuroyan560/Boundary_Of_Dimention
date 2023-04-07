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

	MovieCameraEaseData() :easeChangeType(KuroEngine::In), easeType(KuroEngine::Sine)
	{};
};

struct MovieCameraData
{
	KuroEngine::Transform transform;	//制御点の位置
	bool skipInterpolationFlag;			//座標と角度の補間をスキップするかどうか
	int interpolationTimer;				//次の制御点に向かうまでの秒数
	int stopTimer;						//カメラが定位置についてどれくらいの秒数止まるか
	MovieCameraEaseData easePosData;	//カメラ座標の補間の仕方
	MovieCameraEaseData easeRotaData;	//カメラ角度の補間の仕方
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


	std::vector<KuroEngine::Vec3<float>>m_splinePosArray;

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
	XMVECTOR ConvertVec3toXMVECTOR(const KuroEngine::Vec3<float> &vec)
	{
		return { vec.x, vec.y , vec.z , 0.0f };
	}
	KuroEngine::Vec3<float> ConvertXMVECTORtoVec3(const XMVECTOR &vec)
	{
		return { vec.m128_f32[0], vec.m128_f32[1] , vec.m128_f32[2] };
	}
	KuroEngine::Matrix Ease(const KuroEngine::Matrix &transformA, const KuroEngine::Matrix &transformB, float rate, const MovieCameraEaseData &ease_pos_data, const MovieCameraEaseData &ease_rota_data)
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
		//角度のみ補間し、座標は補間しない
		for (int i = 0; i < matA.size() - 1; ++i)
		{
			//通常の補間
			if (ease_rota_data.easeChangeType == KuroEngine::EASE_CHANGE_TYPE_NUM || ease_rota_data.easeType == KuroEngine::EASING_TYPE_NUM)
			{
				result[i] = 
					Interpolation(
						matA[i],
						matB[i],
						rate
					);
			}
			//イージング
			else
			{
				result[i] =
					ConvertVec4toXMVECTOR(
						KuroEngine::Math::Ease(
							ease_rota_data.easeChangeType,
							ease_rota_data.easeType,
							rate,
							matA[i],
							matB[i]
						));
			}

			mat.r[i] = result[i];
		}

		float easeRate = 0.0f;
		//通常の補間
		if (ease_pos_data.easeChangeType == KuroEngine::EASE_CHANGE_TYPE_NUM || ease_pos_data.easeType == KuroEngine::EASING_TYPE_NUM)
		{
			easeRate = rate;
		}
		//イージング
		else
		{
			easeRate = KuroEngine::Math::Ease(ease_pos_data.easeChangeType, ease_pos_data.easeType, rate, 0.0f, 1.0f);
		}

		//親子関係を考慮した座標を入手
		KuroEngine::Vec3<float> pos = SplinePosition(
			m_splinePosArray,
			m_moveDataIndex,
			easeRate,
			false
		);

		mat.r[3] = { pos.x,pos.y,pos.z,1.0f };

		return mat;
	};


	KuroEngine::XMVECTOR Interpolation(const KuroEngine::Vec4<float> &pos_a, KuroEngine::Vec4<float>pos_b,float rate)
	{
		KuroEngine::Vec4<float> posA = pos_a;
		return ConvertVec4toXMVECTOR(posA + (pos_b - pos_a) * rate);
	}

	KuroEngine::Vec3<float> SplinePosition(const std::vector<KuroEngine::Vec3<float>> &points, size_t startIndex, float t, bool Loop)
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

};

