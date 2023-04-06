#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/Camera.h"
#include"../../../src/engine/ForUser/Timer.h"
#include"../src/Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"

struct MovieCameraData
{
	KuroEngine::Vec3<float>pos;			//���̏ꏊ�Ɍ��������W
	KuroEngine::Vec3<float>rotation;
	bool skipInterpolationFlag;			//���W�Ɗp�x�̕�Ԃ��X�L�b�v���邩�ǂ���
	int interpolationTimer;				//(�b��)
	int stopTimer;						//�J��������ʒu�ɂ��Ăǂꂭ�炢�~�܂邩(�b��)
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
	/// �����蔻�����
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


	//�J������Ԋ֘A�̏��
	int m_moveDataIndex;
	std::vector<MovieCameraData>m_moveDataArray;
	KuroEngine::Transform m_nowTransform;

	std::vector<KuroEngine::Timer> m_timerArray;

	std::shared_ptr<KuroEngine::Model>m_model;
};

