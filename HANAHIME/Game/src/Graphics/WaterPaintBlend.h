#pragma once
#include<memory>
#include"Common/PerlinNoise.h"
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class TextureBuffer;
	class ComputePipeline;
	class Camera;
	class DepthStencil;
}

//���ʉ敗�ɂQ�̃e�N�X�`�����u�����h
class WaterPaintBlend : public KuroEngine::Debugger
{
	static const int THREAD_PER_NUM = 32;
	
	//�C���N�e�N�X�`������
	static const int INK_TEX_NUM = 3;
	//�C���N�e�N�X�`��
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INK_TEX_NUM>m_inkTexArray;

	//�}�X�N���C���[�ɂ��ڂ��C���N
	struct MaskInk
	{
		KuroEngine::Vec3<float>m_pos;
		int m_texIdx;
	};
	std::vector<MaskInk>m_maskInkArray;

	//�p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_pipeline;
	void GeneratePipeline();

	//�p�[�����m�C�Y�̐ݒ�
	KuroEngine::NoiseInitializer m_noiseInitializer;
	//���E�ڂ����Ɏg���m�C�Y�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_noiseTex;
	//���ʂ̕`���
	std::shared_ptr<KuroEngine::TextureBuffer>m_resultTex;

	//�}�X�N���C���[�����_�[�^�[�Q�b�g
	std::shared_ptr<KuroEngine::RenderTarget>m_maskLayer;

	bool m_isDirty = false;

	void OnImguiItems()override;

public:
	WaterPaintBlend();

	//����������
	void Init();

	//�}�X�N���C���[�ɃC���N���炷
	void DropMaskInk(KuroEngine::Vec3<float>arg_pos);

	//�O���t�B�b�N�X�}�l�[�W���ɓo�^
	void Register(std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex, KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_depthStencil);

	//���ʂ̃e�N�X�`���擾
	std::shared_ptr<KuroEngine::TextureBuffer>& GetResultTex() { return m_resultTex; }
};