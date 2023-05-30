#pragma once
#include"Common/Vec.h"
#include"Framework/UsersInput.h"
#include<vector>
#include<array>
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"ForUser/ImpactShake.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"../Movie/MovieCamera.h"
#include"StageInfomation.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Stage/StageParts.h"
#include"../GPUParticle/SplineParticle.h"
#include"../GPUParticle/FireWorkParitlce.h"
#include"../SoundConfig.h"
#include"../Player/PlayerCollision.h"
#include"StageManager.h"
#include"StageInfomation.h"
#include"Stage.h"
#include"../CPUParticle/GlitterEmitter.h"
#include"../CPUParticle/CPULoucusParticle.h"
#include"../Player/CollisionDetectionOfRayAndMesh.h"

//ステージに配置されているゴール
class Goal
{
public:
	Goal(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle);
	void Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model);
	void Finalize();
	void Update(KuroEngine::Transform *transform);
	void Draw(KuroEngine::Camera &camera);
	void Draw2D();

	//ゴール演出スタート
	void Start()
	{
		m_isStartFlag = true;
	}

	//ゴール演出が終わったか
	bool IsEnd();

	bool ChangeCamera()
	{
		return m_changeCameraFlag;
	}

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_camera;
	}


	void InitLimitPos(const KuroEngine::Vec3<float> &pos)
	{
		m_splineLimitPos[0].clear();
		m_splineLimitPos[0].shrink_to_fit();
		m_splineLimitPos[1].clear();
		m_splineLimitPos[1].shrink_to_fit();

		for (int i = 0; i < 9; ++i)
		{
			float radius = 7.0f;
			float angle = static_cast<float>(i * 90);
			float height = (-1.3f + static_cast<float>(i) * 0.4f) * GetFlagUpVec().y;
			m_splineLimitPos[0].emplace_back(pos + KuroEngine::Vec3<float>(cosf(KuroEngine::Angle::ConvertToRadian(angle)), height, sinf(KuroEngine::Angle::ConvertToRadian(angle))) * radius);
			m_splineLimitPos[1].emplace_back(pos + KuroEngine::Vec3<float>(cosf(KuroEngine::Angle::ConvertToRadian(angle + 180.0f)), height, sinf(KuroEngine::Angle::ConvertToRadian(angle + 180.0f))) * radius);
		}

		m_goalFlag = false;
		m_goalTriggerFlag = false;

		m_fireWorkEmittPos[0] = m_splineLimitPos[0][m_splineLimitPos[0].size() - 2];
		m_fireWorkEmittPos[1] = m_splineLimitPos[1][m_splineLimitPos[1].size() - 2];
	};

private:
	bool m_initFlag;
	bool m_isStartFlag, m_prevStartFlag, m_startGoalEffectFlag;
	bool m_startCameraFlag;
	MovieCamera m_movieCamera;					//ゴール時のカメラワーク
	bool m_goalFlag, m_goalTriggerFlag;
	KuroEngine::Transform m_cameraTransform;

	//ゴールの文字演出

	KuroEngine::Vec2<float>m_pos, m_basePos, m_goalPos;
	KuroEngine::Vec2<float>m_goalTexSize;
	std::shared_ptr<KuroEngine::TextureBuffer>m_clearTex;
	float clearTexRadian;
	KuroEngine::Timer m_clearEaseTimer;

	//ゴールのモデル演出
	std::shared_ptr<GoalPoint>m_goalModel;
	KuroEngine::Vec3<float>m_goalModelPos;
	KuroEngine::Vec3<float>m_scaleOffset;
	KuroEngine::Timer m_upEffectEase;
	KuroEngine::Timer m_downEffectEase;

	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_gpuParticleBuffer;
	//KuroEngine::Transform m_goalModelBaseTransform;


	//デバック用
	//ゴールカメラ表示
	std::shared_ptr<KuroEngine::ModelObject> m_goalCamera;
	std::shared_ptr<KuroEngine::Camera> m_camera;
	KuroEngine::Vec3<float>upVec, frontVec;
	std::array<KuroEngine::Vec3<float>, 4>m_cameraHitDirArray;

	std::vector<KuroEngine::Vec3<float>> cameraPosArray;


	//軌跡----------------------------------------
	bool m_playerGoalFlag;
	std::array<std::vector<KuroEngine::Vec3<float>>, 2>m_splineLimitPos;
	std::array<KuroEngine::Vec3<float>, 2>m_fireWorkEmittPos;
	std::array<std::unique_ptr<SplineParticle>, 2>m_splineArray;
	//軌跡----------------------------------------


	bool m_changeCameraFlag;

	KuroEngine::Vec3<float>GetFlagUpVec()
	{
		if (!m_goalModel)
		{
			return KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f);
		}
		DirectX::XMVECTOR dir;
		float radian = 0.0f;
		DirectX::XMQuaternionToAxisAngle(&dir, &radian, m_goalModel->GetTransform().GetRotate());

		radian += KuroEngine::Angle::ConvertToRadian(90);
		KuroEngine::Vec3<float>basePos = m_goalModel->GetTransform().GetPos();
		KuroEngine::Vec3<float>circleDir(basePos + KuroEngine::Vec3<float>(cosf(radian), sinf(radian), 0.0f));

		KuroEngine::Vec3<float>resultDir = circleDir - m_goalModel->GetTransform().GetPos();
		resultDir.Normalize();

		return resultDir;
	}



	//段々近づく
	KuroEngine::Vec3<float>m_zoomCameraPos;

	KuroEngine::Timer m_zoomInTimer, m_zoomOutTimer, m_sceneChangeTimer;

	bool m_initParticleFlag;
	//GlitterEmitter m_glitterEmitt;

	KuroEngine::ImpactShake m_shake;
	KuroEngine::Vec3<float>m_goalBasePos;



	std::array<CPULoucusEmitter, 2> m_loucusParticle;

	
	FireWork m_fireWork;

	void InitCameraPosArray(const KuroEngine::Vec3<float> &pos)
	{
		auto stage = StageManager::Instance()->GetNowStage().lock()->GetTerrianArray();
		//前
		m_cameraHitDirArray[0] = KuroEngine::Vec3<float>(0.0f, 0.0f, 1.0f);
		//後
		m_cameraHitDirArray[1] = KuroEngine::Vec3<float>(0.0f, 0.0f, -1.0f);
		//左
		m_cameraHitDirArray[2] = KuroEngine::Vec3<float>(-1.0f, 0.0f, 0.0f);
		//右
		m_cameraHitDirArray[3] = KuroEngine::Vec3<float>(1.0f, 0.0f, 0.0f);

		const float distance = 30.0f;
		std::array<bool, 4>hitIndex = { false,false,false,false };
		for (auto &dir : m_cameraHitDirArray)
		{
			//地形配列走査
			for (auto &terrian : stage)
			{
				//モデル情報取得
				auto model = terrian.GetModel().lock();

				//メッシュを走査
				for (auto &modelMesh : model->m_meshes)
				{
					//判定↓============================================

					//当たり判定を行うメッシュ。
					std::vector<TerrianHitPolygon> mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];
					CollisionDetectionOfRayAndMesh::MeshCollisionOutput result = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(pos, dir, mesh);
					if (result.m_isHit && result.m_distance <= distance && 0.0f <= result.m_distance)
					{
						hitIndex[&dir - &m_cameraHitDirArray[0]] = true;
					}
					//=================================================
				}
			}
		}

		cameraPosArray.clear();
		for (auto &obj : hitIndex)
		{
			if (!obj)
			{
				cameraPosArray.emplace_back(pos + m_cameraHitDirArray[&obj - &hitIndex[0]] * distance);
			}
		}

	}

};




