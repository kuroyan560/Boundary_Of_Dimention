#pragma once
#include"Gate.h"

class StageSelect
{
public:
	StageSelect();
	void Update();
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

private:
	std::vector<std::unique_ptr<Gate>>m_gateArray;
	std::vector<GateData>m_gateDataArray;
};

