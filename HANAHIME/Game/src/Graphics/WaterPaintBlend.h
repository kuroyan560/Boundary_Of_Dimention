#pragma once
#include<memory>
#include"ForUser/Debugger.h"
#include<vector>
#include"Common/Vec.h"
#include"ForUser/Timer.h"

namespace KuroEngine
{
	class VertexBuffer;
	class TextureBuffer;
	class ConstantBuffer;
	class StructuredBuffer;
	class RWStructuredBuffer;

	class GraphicsPipeline;
	class ComputePipeline;

	class DepthStencil;
	class Camera;
}

//���ʉ敗�ɂQ�̃e�N�X�`�����u�����h
class WaterPaintBlend : public KuroEngine::Debugger
{
	static const int THREAD_PER_NUM = 32;
	
	//�C���N�e�N�X�`������
	static const int INK_TEX_NUM = 3;
	//�C���N�e�N�X�`��
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INK_TEX_NUM>m_inkTexArray;

	//�萔�o�b�t�@
	struct ConstData
	{
		//�}�X�N�C���N�̃X�P�[��
		float m_initScale = 2.0f;
		//���W�Y���ő�
		float m_posOffsetMax = 0.3f;
		//�C���N�e�N�X�`����
		int m_texMax = INK_TEX_NUM;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//��x�ɐ����ł���ő吔
	static const int GENERATE_MAX_ONCE = 20;
	//�����\��̃C���N�̍��W�z��
	std::vector<KuroEngine::Vec3<float>>m_appearInkPosArray;
	//�����\��̃C���N���X�^�b�N���Ă����o�b�t�@
	std::shared_ptr<KuroEngine::StructuredBuffer>m_stackInkBuffer;

	//�}�X�N���C���[�ɂ��ڂ����C���N
	struct MaskInk
	{
		KuroEngine::Vec3<float>m_pos;
		float m_scale;
		KuroEngine::Vec3<float>m_posOffset;
		int m_texIdx;
	};
	//���������C���N�̃o�b�t�@�[
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_aliveInkBuffer;
	//���������C���N�̃J�E���^�[�o�b�t�@�[
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_aliveInkCounterBuffer;

	//�����ł���}�X�N�C���N�̍ő吔
	int m_aliveInkMax = 10000;

	//�}�X�N�C���N�̍X�V����X�p��
	int m_updateSpan = 3;
	KuroEngine::Timer m_updateTimer;

	//�}�X�N�C���N�`��p�̔|�����_�o�b�t�@
	static std::shared_ptr<KuroEngine::VertexBuffer>s_maskInkPolygon;

	//�}�X�N�C���N������������p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_initInkPipeline;
	//�}�X�N�C���N�𐶐�����p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_appearInkPipeline;
	//�}�X�N�C���N���X�V����p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_updateInkPipeline;
	//�}�X�N�C���N��`�悷��p�C�v���C��
	static std::shared_ptr<KuroEngine::GraphicsPipeline>s_drawInkPipeline;

	//���ʉ敗�ɂ���p�C�v���C��
	static std::shared_ptr<KuroEngine::ComputePipeline>s_waterPaintPipeline;
	void GeneratePipeline();

	//���ʂ̕`���
	std::shared_ptr<KuroEngine::TextureBuffer>m_resultTex;

	//�}�X�N���C���[�����_�[�^�[�Q�b�g
	std::shared_ptr<KuroEngine::RenderTarget>m_maskLayer;

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