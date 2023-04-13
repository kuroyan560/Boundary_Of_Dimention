#pragma once
#include"Gate.h"
#include"../Movie/StageChange.h"
#include"../Movie/MovieCamera.h"

/// <summary>
/// �z�[�����
/// </summary>
class HomeStageSelect
{
public:
	HomeStageSelect();
	void Init();
	void Update();
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	/// <summary>
	/// ���Ɣ�������A�ԍ����󂯎��
	/// </summary>
	/// <param name="player_pos">�v���C���[�̍��W</param>
	/// <returns>�X�e�[�W�ԍ�</returns>
	int GetStageNumber(const KuroEngine::Vec3<float> &player_pos);
private:
	std::vector<std::unique_ptr<Gate>>m_gateArray;
	std::vector<GateData>m_gateDataArray;

	MovieCameraData titleCameraData;
};

