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

//�X�e�[�W�ɔz�u����Ă���S�[��
class Goal
{
public:
	Goal();
	void Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model);
	void Finalize();
	void Update(KuroEngine::Transform *transform);
	void Draw(KuroEngine::Camera &camera);

	//�S�[���Ƃ̏Փ˔���
	bool IsHit(const KuroEngine::Vec3<float> &player_pos);
	//�S�[�����o���I�������
	bool IsEnd();

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_movieCamera.GetCamera();
	}

private:
	bool m_initFlag;
	bool m_isHitFlag, m_startGoalEffectFlag;
	bool m_startCameraFlag;
	MovieCamera m_movieCamera;					//�S�[�����̃J�������[�N

	std::shared_ptr<KuroEngine::ModelObject> m_model;
	std::shared_ptr<KuroEngine::ModelObject> m_upVecObj;
	KuroEngine::Transform m_cameraTransform;

	//�S�[���̕������o

	KuroEngine::Vec2<float>m_pos, m_basePos, m_goalPos;
	std::shared_ptr<KuroEngine::TextureBuffer>m_clearTex;
	float clearTexRadian;
	KuroEngine::Timer m_clearEaseTimer;

	//�S�[���̃��f�����o
	std::shared_ptr<GoalPoint>m_goalModel;
	KuroEngine::Timer m_upEffectEase;
	KuroEngine::Timer m_downEffectEase;

	KuroEngine::Transform m_goalModelBaseTransform;


	//�f�o�b�N�p
	//�S�[���J�����\��
	std::shared_ptr<KuroEngine::ModelObject> m_goalCamera;
	std::shared_ptr<KuroEngine::Camera> m_camera;

	KuroEngine::Vec3<float>upVec, frontVec;

	KuroEngine::Vec3<float>GetFlagUpVec()
	{
		DirectX::XMVECTOR dir;
		float radian = 0.0f;
		DirectX::XMQuaternionToAxisAngle(&dir, &radian, m_goalModelBaseTransform.GetRotate());

		radian += KuroEngine::Angle::ConvertToRadian(90);
		KuroEngine::Vec3<float>basePos = m_goalModelBaseTransform.GetPos();
		KuroEngine::Vec3<float>circleDir(basePos + KuroEngine::Vec3<float>(cosf(radian), sinf(radian), 0.0f));

		KuroEngine::Vec3<float>resultDir = circleDir - m_goalModelBaseTransform.GetPos();
		resultDir.Normalize();

		return resultDir;
	}

	//��]�̏���----------------------------------------

	struct ClearParticleData
	{
		KuroEngine::Vec3<float> m_emittPos;
		KuroEngine::Timer m_timer;
		float m_angle;
		float m_baseAngle;
		ClearParticleData() :m_timer(120), m_angle(0.0f), m_baseAngle(0.0f)
		{};
	};
	std::array<ClearParticleData, 2>m_emitter;
	float m_radius;
	float m_height;
	KuroEngine::Timer m_heightTimer;

	class ParticleData
	{
		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Timer m_timer;
	};

	std::array<std::shared_ptr<KuroEngine::ModelObject>,2> m_emittObject;

	class Particle
	{
	public:
		Particle();
		void Init(const KuroEngine::Vec3<float> &pos);
		void Update();
		void Draw();
		bool IsAlive()
		{
			return m_initFlag;
		}
	private:
		bool m_initFlag;
		int m_timer;
	};
	std::array<Particle, 50>m_particleArray;
	//��]�̏���----------------------------------------

};