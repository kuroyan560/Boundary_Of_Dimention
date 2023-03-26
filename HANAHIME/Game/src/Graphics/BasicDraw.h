#pragma once
#include<memory>
#include"Render/LightBloomDevice.h"
#include"Common/Transform.h"
#include"Common/Color.h"
#include"BasicDrawParameters.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"Render/RenderObject/SpriteMesh.h"

namespace KuroEngine
{
	class LightManager;
	class ModelObject;
	class Model;
	class Camera;
	class CubeMap;
	class GraphicsPipeline;
	class ConstantBuffer;
};

class BasicDraw : public KuroEngine::DesignPattern::Singleton<BasicDraw>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<BasicDraw>;
	BasicDraw();

	//�g�D�[���V�F�[�_�[�̋��L�p�����[�^
	struct ToonCommonParameter
	{
		//���邳�̂������l�i�͈͂��������Ă���j
		float m_brightThresholdLow = 0.66f;
		float m_brightThresholdRange = 0.03f;
		//�������C�g�̉e�����������̂܂܂̐F�ŏo�͂���ۂ̂������l
		float m_limThreshold = 0.4f;
	};
	ToonCommonParameter m_toonCommonParam;

	//�G�b�W�̋��L�p�����[�^
	struct EdgeCommonParameter
	{
		//�G�b�W�`��̔��f������[�x���̂������l
		float m_depthThreshold = 0.19f;
		float m_pad[3];
		//�[�x�l���ׂ�e�N�Z���ւ�UV�I�t�Z�b�g�i�ߖT8�j
		std::array<KuroEngine::Vec2<float>, 8>m_uvOffset;
	};
	EdgeCommonParameter m_edgeShaderParam;

	//���f���ʏ�`��J�E���g
	int m_drawCount = 0;
	//���f���C���X�^���V���O�`��J�E���g
	int m_drawCountHuge = 0;
	//�g�p����g�D�[�����̃J�E���g
	int m_individualParamCount = 0;

	//���ʂ̃g�D�[���V�F�[�_�[�p���
	std::shared_ptr<KuroEngine::ConstantBuffer>m_toonCommonParamBuff;
	//�`�斈�̃g�D�[���`����
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_toonIndividualParamBuff;

	//���f���`��
	std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>m_drawPipeline;
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuff;

	//�C���X�^���V�O�`��ň�x�ɕ`��ł���C���X�^���X�ő吔
	static const int s_instanceMax = 1024;
	//�C���X�^���V���O�`��Ńf�v�X�X�e���V���ɐ[�x��`�悷�邩�̃^�u
	enum INSTANCING_DRAW_MODE { WRITE_DEPTH, NOT_WRITE_DEPTH };
	//���f���`��i�C���X�^���V���O�`��j
	std::array<std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>, 2>m_instancingDrawPipeline;
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuffHuge;


	//�G�b�W�o�́��`��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_edgePipeline;
	std::unique_ptr<KuroEngine::SpriteMesh>m_spriteMesh;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_edgeShaderParamBuff;

	void OnImguiItems()override;

public:
	void Awake(KuroEngine::Vec2<float>arg_screenSize, int arg_prepareBuffNum = 100);
	void CountReset() 
	{
		m_drawCount = 0;
		m_drawCountHuge = 0;
		m_individualParamCount = 0;
	}

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="arg_cam">�J����</param>
	/// <param name="arg_ligMgr">���C�g�}�l�[�W��</param>
	/// <param name="arg_model">���f��</param>
	/// <param name="arg_transform">�g�����X�t�H�[��</param>
	/// <param name="arg_toonParam">�g�D�[���̃p�����[�^</param>
	/// <param name="arg_boneBuff">�{�[���o�b�t�@</param>
	void Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform& arg_transform, 
		const IndividualDrawParameter& arg_toonParam, 
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr, 
		std::weak_ptr<KuroEngine::Model>arg_model, 
		KuroEngine::Transform& arg_transform, 
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//���f���I�u�W�F�N�g�`��
	void BasicDraw::Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const IndividualDrawParameter& arg_toonParam,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None);

	//���f���I�u�W�F�N�g�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void BasicDraw::Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None);

	//�C���X�^���V���O�`��
	void InstancingDraw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix>& arg_matArray,
		const IndividualDrawParameter& arg_toonParam,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None, 
		const float& arg_depth = 0.0f, 
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�C���X�^���V���O�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void InstancingDraw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix>& arg_matArray,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		const float& arg_depth = 0.0f,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	/// <summary>
	/// �G�b�W�`��
	/// </summary>
	/// <param name="arg_depthMap">�[�x�}�b�v</param>
	/// <param name="arg_normalMap">�@���}�b�v</param>
	/// <param name="arg_edgeColorMap">�G�b�W�J���[�}�b�v</param>
	void DrawEdge(std::shared_ptr<KuroEngine::TextureBuffer>arg_depthMap, std::shared_ptr<KuroEngine::TextureBuffer>arg_normalMap, std::shared_ptr<KuroEngine::TextureBuffer>arg_edgeColorMap);
};