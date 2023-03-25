#pragma once
#include<memory>
#include"Common/PerlinNoise.h"
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class TextureBuffer;
	class ComputePipeline;
}

//���ʉ敗�ɂQ�̃e�N�X�`�����u�����h
class WaterPaintBlend : public KuroEngine::Debugger
{
	static const int THREAD_PER_NUM = 32;

	//�p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_pipeline;
	void GeneratePipeline();

	//�p�[�����m�C�Y�̐ݒ�
	KuroEngine::NoiseInitializer m_noiseInitializer;
	//���E�ڂ����Ɏg���m�C�Y�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_noiseTex;
	//���ʂ̕`���
	std::shared_ptr<KuroEngine::TextureBuffer>m_resultTex;

	bool m_isDirty = false;

	void OnImguiItems()override;

public:
	WaterPaintBlend();

	//�O���t�B�b�N�X�}�l�[�W���ɓo�^
	void Register(
		std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex,
		std::shared_ptr<KuroEngine::TextureBuffer>arg_maskTex);

	//���ʂ̃e�N�X�`���擾
	std::shared_ptr<KuroEngine::TextureBuffer>& GetResultTex() { return m_resultTex; }
};