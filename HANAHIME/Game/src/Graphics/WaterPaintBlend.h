#pragma once
#include<memory>
#include"Common/PerlinNoise.h"

namespace KuroEngine
{
	class TextureBuffer;
	class RenderTarget;
}

//���ʉ敗�ɂQ�̃e�N�X�`�����u�����h
class WaterPaintBlend
{
	//�p�[�����m�C�Y�̐ݒ�
	KuroEngine::NoiseInitializer m_noiseInitializer;
	//���E�ڂ����Ɏg���m�C�Y�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_noiseTex;
	//���ʂ̕`���
	std::shared_ptr<KuroEngine::RenderTarget>m_resultTex;

public:
	WaterPaintBlend();

	//�O���t�B�b�N�X�}�l�[�W���ɓo�^
	void Register(
		std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex,
		std::shared_ptr<KuroEngine::TextureBuffer>arg_blendTex,
		std::shared_ptr<KuroEngine::TextureBuffer>arg_maskTex);

	//���ʂ̃e�N�X�`���擾
};

