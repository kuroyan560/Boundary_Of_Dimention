#pragma once
#include"Common/Transform.h"
#include"ForUser/JsonData.h"
#include"Render/RenderObject/Light.h"
#include"../Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"

/// <summary>
/// ���̏��������
/// </summary>
struct GateData
{
	GateData(KuroEngine::Transform transform = {}, int stage_num = -1) :
		m_transform(transform), m_stageNum(stage_num)
	{};

	KuroEngine::Transform m_transform;
	int m_stageNum;
};

/// <summary>
/// �ʂ̃X�e�[�W�ړ�����p�̔�
/// </summary>
class Gate
{
public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="transform">���W�A�傫���A�p�x</param>
	/// <param name="stage_num">�ړ�����X�e�[�W�ԍ�</param>
	Gate(const KuroEngine::Transform &transform, int stage_num);

	void Update();

	/// <summary>
	/// ���Ƃ̔���
	/// </summary>
	/// <param name="player_pos">�v���C���[�̍��W</param>
	/// <returns>true...���ƐڐG,false...�ڐG���Ă��Ȃ�</returns>
	bool IsHit(const KuroEngine::Vec3<float> &player_pos);

	int GetStageNum() { return m_stageNum; };

	KuroEngine::Vec3<float>ForceVel();

	/// <summary>
	/// �����蔻�����
	/// </summary>
	void DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

private:
	std::shared_ptr<KuroEngine::Model>m_model;
	KuroEngine::Transform m_transform;
	int m_stageNum;
};

