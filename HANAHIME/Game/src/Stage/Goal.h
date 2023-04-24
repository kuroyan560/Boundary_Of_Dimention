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
#include"../SoundConfig.h"


//�X�e�[�W�ɔz�u����Ă���S�[��
class Goal
{
public:
	Goal();
	void Init(const KuroEngine::Transform &transform, std::shared_ptr<GoalPoint>goal_model);
	void Finalize();
	void Update(KuroEngine::Transform *transform);
	void Draw(KuroEngine::Camera &camera);

	//�S�[�����o�X�^�[�g
	void Start()
	{
		m_isStartFlag = true;
	}

	//�S�[�����o���I�������
	bool IsEnd();

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_movieCamera.GetCamera();
	}


	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_gpuParticleBuffer;

private:
	bool m_initFlag;
	bool m_isStartFlag, m_prevStartFlag, m_startGoalEffectFlag;
	bool m_startCameraFlag;
	MovieCamera m_movieCamera;					//�S�[�����̃J�������[�N
	
	KuroEngine::Transform m_cameraTransform;

	//�S�[���̕������o

	KuroEngine::Vec2<float>m_pos, m_basePos,m_goalPos;
	std::shared_ptr<KuroEngine::TextureBuffer>m_clearTex;
	float clearTexRadian;
	KuroEngine::Timer m_clearEaseTimer;

	//�S�[���̃��f�����o
	//std::shared_ptr<GoalPoint>m_goalModel;
	KuroEngine::Timer m_upEffectEase;
	KuroEngine::Timer m_downEffectEase;

	//KuroEngine::Transform m_goalModelBaseTransform;


	//�f�o�b�N�p
	//�S�[���J�����\��
	std::shared_ptr<KuroEngine::ModelObject> m_goalCamera;
	std::shared_ptr<KuroEngine::Camera> m_camera;

	KuroEngine::Vec3<float>upVec, frontVec;


	//�O��----------------------------------------

	struct SplineData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
		DirectX::XMFLOAT4 color;
		int startIndex;
		float rate;
	};

	struct ConstData
	{
		DirectX::XMMATRIX scaleRotate;
		UINT startIndex;
		float rate;

		ConstData()
		{
			startIndex = 0;
			rate = 0.0f;
		};
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_particleBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_limitIndexPosBuffer;

	std::shared_ptr<KuroEngine::ConstantBuffer> m_scaleRotaBuffer;
	std::shared_ptr<KuroEngine::ConstantBuffer> m_limitIndexBuffer;

	std::shared_ptr<KuroEngine::ComputePipeline> m_initLoucusPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline> m_updateLoucusPipeline;

	ConstData cd;
	KuroEngine::Timer m_splineTimer;

	static const int LIMIT_POS_MAX = 10;
	std::array<std::shared_ptr<KuroEngine::ModelObject>, LIMIT_POS_MAX>limitPosArray;

	void GenerateLoucus()
	{
		m_particleBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(SplineData), 1024);
		m_limitIndexPosBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(DirectX::XMFLOAT3), LIMIT_POS_MAX);

		m_scaleRotaBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1);
		m_limitIndexBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(UINT), 1);


		std::vector<KuroEngine::RootParam>rootParam =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�X�v���C���p�[�e�B�N���̏��(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"����_�̍��W(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"����_�̍ő�l(RWStructuredBuffer)")
		};
		auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SplineParticle.hlsl", "SplineInitMain", "cs_6_4");
		m_initLoucusPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

		auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SplineParticle.hlsl", "SplineUpdateMain", "cs_6_4");
		m_updateLoucusPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { KuroEngine::WrappedSampler(true,true) });

	};

	//�O��----------------------------------------



};




