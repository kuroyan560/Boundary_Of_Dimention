#pragma once
#include"Common/Vec.h"
#include"Framework/UsersInput.h"
#include<vector>
#include<array>
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"../Movie/MovieCamera.h"
#include"StageInfomation.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Stage/StageParts.h"

//ステージに配置されているゴール
class Goal
{
public:
	Goal();
	void Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model);
	void Finalize();
	void Update(KuroEngine::Transform *transform);
	void Draw(KuroEngine::Camera &camera);

	//ゴールとの衝突判定
	bool IsHit(const KuroEngine::Vec3<float> &player_pos);
	//ゴール演出が終わったか
	bool IsEnd();

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_movieCamera.GetCamera();
	}

private:
	bool m_initFlag;
	bool m_isHitFlag, m_startGoalEffectFlag;
	bool m_startCameraFlag;
	MovieCamera m_movieCamera;					//ゴール時のカメラワーク
	
	std::shared_ptr<KuroEngine::ModelObject> m_model;
	KuroEngine::Transform m_cameraTransform;

	//ゴールの文字演出

	KuroEngine::Vec2<float>m_pos, m_basePos,m_goalPos;
	std::shared_ptr<KuroEngine::TextureBuffer>m_clearTex;
	float clearTexRadian;
	KuroEngine::Timer m_clearEaseTimer;

	//ゴールのモデル演出
	std::shared_ptr<GoalPoint>m_goalModel;
	KuroEngine::Timer m_upEffectEase;
	KuroEngine::Timer m_downEffectEase;

	KuroEngine::Transform m_goalModelBaseTransform;


	//デバック用
	//ゴールカメラ表示
	std::shared_ptr<KuroEngine::ModelObject> m_goalCamera;
	std::shared_ptr<KuroEngine::Camera> m_camera;

	KuroEngine::Vec3<float>upVec, frontVec;

};