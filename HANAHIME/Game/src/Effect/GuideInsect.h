#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include<vector>
#include"../GPUParticle/SignSpotFireFly.h"

class GuideInsect
{
public:
	struct CheckPointData
	{
		KuroEngine::Vec3<float>m_pos;
		bool m_isHitFlag;
	};


	GuideInsect(std::shared_ptr<KuroEngine::RWStructuredBuffer>particle_buffer);

	void Init();
	void Update();
	void Draw(KuroEngine::Camera &camera, KuroEngine::LightManager &light);

	std::shared_ptr<CheckPointData> Stack();

	void SetStageNum(int num)
	{
		m_stageIndex = num;
	}


	void GoToCheckPoint(const KuroEngine::Vec3<float> &pos);

private:
	int m_index;
	int m_stageIndex;
	std::vector<std::shared_ptr<CheckPointData>>m_checkPointArray;


	SignSpotFireFly m_guideInsect;
	bool m_goToCheckPointFlag;
	KuroEngine::Timer m_tiemr;
};