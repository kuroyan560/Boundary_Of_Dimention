#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"Common/Transform.h"
#include<vector>
#include<memory>
#include"DirectX12/D3D12Data.h"
#include"DirectX12/D3D12App.h"

class EnemyDataReferenceForCircleShadow : public KuroEngine::DesignPattern::Singleton<EnemyDataReferenceForCircleShadow>
{

public:

	//�G�f�[�^�Q�Ɨp�\����
	struct RefData {

		const KuroEngine::Transform* m_refTransform;
		const float* m_refShadowRadius;
		const bool* m_refIsDead;

	};

	//GPU�ɓ]������f�[�^
	struct GPUData {

		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Vec3<float> m_up;
		float m_shadowRadius;

	};

	//�ő�Q�ƕۑ����B
	const int MAX_REFCOUNT = 512;

private:
	
	std::vector<RefData> m_refData;		//�ۑ�����Ă���G�f�[�^
	std::vector<GPUData> m_gpuData;		//GPU�ɓ]������f�[�^
	std::shared_ptr<KuroEngine::StructuredBuffer> m_gpuResourceBuffer;		//GPU�ɓ]������f�[�^������\�����o�b�t�@
	std::shared_ptr<KuroEngine::ConstantBuffer> m_gpuResourceCountBuffer;	//GPU�ɓ]������f�[�^�̃T�C�Y������萔�o�b�t�@
	int m_gpuResourceCount;

public:

	//�\�����o�b�t�@��p��
	void Setting();

	//������ �Q�Ƃ�S�ă��Z�b�g
	void Init();

	//�Q�Ƃ��Z�b�g�B
	void SetData(const KuroEngine::Transform* arg_refTransform, const float* arg_refShadowRadius, const bool* arg_refIsDead);

	//GPU�ɓ]������f�[�^���X�V����B
	void UpdateGPUData();

	//GPU�ɓ]������f�[�^�̃T�C�Y���擾�B
	std::shared_ptr<KuroEngine::ConstantBuffer> GetGPUResourceCountBuffer() { return m_gpuResourceCountBuffer; }

	//GPU�ɓ]������f�[�^���擾�B
	std::shared_ptr<KuroEngine::StructuredBuffer> GetGPUResourceBuffer() { return m_gpuResourceBuffer; }


};