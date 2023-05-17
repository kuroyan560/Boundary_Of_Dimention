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
	class RenderTarget;
};

class BasicDraw : public KuroEngine::DesignPattern::Singleton<BasicDraw>, public KuroEngine::Debugger
{
public:
	//�����_�[�^�[�Q�b�g
	enum RENDER_TARGET_TYPE { MAIN, EMISSIVE, DEPTH, EDGE_COLOR, BRIGHT, NORMAL, NORMAL_GRASS, WORLD_POS, NUM };	//NORMAL_GRASS�͑��p�B���𐶂₵�����Ȃ��I�u�W�F�N�g�̖@���͏������܂Ȃ��B

private:

	friend class KuroEngine::DesignPattern::Singleton<BasicDraw>;
	BasicDraw();

	//�g�D�[���V�F�[�_�[�̋��L�p�����[�^
	struct ToonCommonParameter
	{
		//���邳�̂������l�i�͈͂��������Ă���j
		float m_brightThresholdLow = 0.66f;
		float m_brightThresholdRange = 0.03f;
		float m_monochromeRate = 0.3f;
	};
	ToonCommonParameter m_toonCommonParam;

	//�G�b�W�̋��L�p�����[�^
	struct EdgeCommonParameter
	{
		DirectX::XMMATRIX m_view;
		DirectX::XMMATRIX m_proj;
		//�G�b�W�`��̔��f������[�x���̂������l
		float m_depthThreshold = 0.19f;
	};
	EdgeCommonParameter m_edgeShaderParam;

	//�v���C���[�Ɋւ�����̒萔�o�b�t�@
	struct PlayerInfo
	{
		KuroEngine::Vec3<float>m_worldPos;
		KuroEngine::Vec2<float>m_screenPos;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_playerInfoBuffer;

	//���f���ʏ�`��J�E���g
	int m_drawCount = 0;
	//���f���C���X�^���V���O�`��J�E���g
	int m_drawCountHuge = 0;
	//�g�p����g�D�[�����̃J�E���g
	int m_individualParamCount = 0;
	//�r���{�[�h�̕`��
	int m_drawBillboardCount = 0;

	int m_drawRectBillboardCount = 0;

	//���ʂ̃g�D�[���V�F�[�_�[�p���
	std::shared_ptr<KuroEngine::ConstantBuffer>m_toonCommonParamBuff;
	//�`�斈�̃g�D�[���`����
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_toonIndividualParamBuff;

	//���f���`��
	std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>m_drawPipeline;
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_drawPipeline_player;
	std::array < std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>m_drawPipeline_stage;
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuff;

	//�C���X�^���V�O�`��ň�x�ɕ`��ł���C���X�^���X�ő吔
	static const int s_instanceMax = 1024;
	//�C���X�^���V���O�`��Ńf�v�X�X�e���V���ɐ[�x��`�悷�邩�̃^�u
	enum INSTANCING_DRAW_MODE { WRITE_DEPTH, NOT_WRITE_DEPTH };
	//���f���`��i�C���X�^���V���O�`��j
	std::array<std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>, 2>m_instancingDrawPipeline;
	std::array<std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>, 2>m_instancingDrawPipeline_nooutline;	//�A�E�g���C����`�悵�Ȃ��B
	std::array<std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>, 2>m_instancingDrawPipeline_smokeNoise;	//�����m�C�Y�ŕ`�悷��B
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuffHuge;

	//�G�b�W�o�́��`��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_edgePipeline;
	std::unique_ptr<KuroEngine::SpriteMesh>m_spriteMesh;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_edgeShaderParamBuff;
	//�f�v�X�}�b�v�̎Q�Ɨp�N���[��
	std::shared_ptr<KuroEngine::TextureBuffer>m_depthMapClone;

	std::array<std::shared_ptr<KuroEngine::RenderTarget>, RENDER_TARGET_TYPE::NUM>m_renderTargetArray;

	//�v���C���[����O�̃I�u�W�F�N�g�𓧉߂�����ۂ̃e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_playerMaskTex;

	//�A����ɐB�����郉�C�g�̐��̒萔�o�b�t�@
	static const int GROW_PLANT_LIGHT_MAX_NUM = 20;
	struct GrowPlantLightNum
	{
		unsigned int m_ptLig = 0;
		unsigned int m_spotLig = 0;
		unsigned int pad[2];
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_growPlantLigNumBuffer;
	//�A����ɐB������|�C���g���C�g�̍\���o�b�t�@
	std::shared_ptr<KuroEngine::StructuredBuffer>m_growPlantPtLigBuffer;
	//�A����ɐB������X�|�b�g���C�g�̍\���o�b�t�@
	std::shared_ptr<KuroEngine::StructuredBuffer>m_growPlantSpotLigBuffer;


	//�r���{�[�h---------------------------------------
	class Vertex
	{
	public:
		KuroEngine::Vec3<float>m_pos;
		KuroEngine::Vec2<float>m_size;
		KuroEngine::Color m_col;
		Vertex(const KuroEngine::Vec3<float> &Pos, const KuroEngine::Vec2<float> &Size, const KuroEngine::Color &Color)
			:m_pos(Pos), m_size(Size), m_col(Color) {}
	};
	std::vector<std::shared_ptr<KuroEngine::VertexBuffer>>s_graphVertBuff;

	std::shared_ptr<KuroEngine::GraphicsPipeline>m_drawBillBoardPipeline;

	class RectVertex
	{
	public:
		KuroEngine::Vec3<float>m_pos;
		KuroEngine::Vec2<float>m_upSize;
		KuroEngine::Vec2<float>m_downSize;
		KuroEngine::Color m_col;
		RectVertex(const KuroEngine::Vec3<float> &Pos, const KuroEngine::Vec2<float> &UpSize, const KuroEngine::Vec2<float> &DownSize,const KuroEngine::Color &Color)
			:m_pos(Pos), m_upSize(UpSize),m_downSize(DownSize), m_col(Color) {}
	};
	std::vector<std::shared_ptr<KuroEngine::VertexBuffer>>s_billBoardRectVertBuff;
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_drawRectBillBoardPipeline;


	void OnImguiItems()override;

public:
	void Awake(KuroEngine::Vec2<float>arg_screenSize, int arg_prepareBuffNum = 100);
	void CountReset()
	{
		m_drawCount = 0;
		m_drawCountHuge = 0;
		m_individualParamCount = 0;
		m_drawBillboardCount = 0;
		m_drawRectBillboardCount = 0;
	}

	void Update(KuroEngine::Vec3<float>arg_playerPos, KuroEngine::Camera &arg_cam);

	void RenderTargetsClearAndSet(std::weak_ptr<KuroEngine::DepthStencil>arg_ds);

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="arg_cam">�J����</param>
	/// <param name="arg_ligMgr">���C�g�}�l�[�W��</param>
	/// <param name="arg_model">���f��</param>
	/// <param name="arg_transform">�g�����X�t�H�[��</param>
	/// <param name="arg_toonParam">�g�D�[���̃p�����[�^</param>
	/// <param name="arg_layer">�`�惌�C���[</param>
	/// <param name="arg_boneBuff">�{�[���o�b�t�@</param>
	void Draw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform &arg_transform,
		const IndividualDrawParameter &arg_toonParam,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr,
		int arg_layer = 0);

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="arg_cam">�J����</param>
	/// <param name="arg_ligMgr">���C�g�}�l�[�W��</param>
	/// <param name="arg_model">���f��</param>
	/// <param name="arg_transform">�g�����X�t�H�[��</param>
	/// <param name="arg_toonParam">�g�D�[���̃p�����[�^</param>
	/// <param name="arg_boneBuff">�{�[���o�b�t�@</param>
	/// <param name="arg_layer">�`�惌�C���[</param>
	void Draw_Stage(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform &arg_transform,
		const IndividualDrawParameter &arg_toonParam,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr,
		int arg_layer = 0);

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="arg_cam">�J����</param>
	/// <param name="arg_ligMgr">���C�g�}�l�[�W��</param>
	/// <param name="arg_model">���f��</param>
	/// <param name="arg_transform">�g�����X�t�H�[��</param>
	/// <param name="arg_toonParam">�g�D�[���̃p�����[�^</param>
	/// <param name="arg_boneBuff">�{�[���o�b�t�@</param>
	/// <param name="arg_layer">�`�惌�C���[</param>
	void Draw_Player(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform &arg_transform,
		const IndividualDrawParameter &arg_toonParam,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr,
		int arg_layer = 0);

	//�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void Draw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform &arg_transform,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr, int arg_layer = 0);

	//���f���I�u�W�F�N�g�`��
	void BasicDraw::Draw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const IndividualDrawParameter &arg_toonParam,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0);

	//���f���I�u�W�F�N�g�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void BasicDraw::Draw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0);

	//�C���X�^���V���O�`��
	void InstancingDraw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix> &arg_matArray,
		const IndividualDrawParameter &arg_toonParam,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�C���X�^���V���O�`�� �p�[�e�B�N���p �A�E�g���C���Ȃ�
	void InstancingDraw_NoOutline(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix> &arg_matArray,
		const IndividualDrawParameter &arg_toonParam,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�C���X�^���V���O�`�� �p�[�e�B�N���p �A�E�g���C���Ȃ� �m�C�Y�ŉ���`�悷��B�v���C���[������������p�B
	void InstancingDraw_NoiseSmoke(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix> &arg_matArray,
		const IndividualDrawParameter &arg_toonParam,
		bool arg_depthWriteMask,
		std::shared_ptr < KuroEngine::ConstantBuffer> arg_smokeNoiseTimerBuffer,
		std::shared_ptr < KuroEngine::StructuredBuffer> arg_smokeNoiseAlphaBuffer,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�C���X�^���V���O�`��i�f�t�H���g�̃g�D�[���p�����[�^���g�p�j
	void InstancingDraw(KuroEngine::Camera &arg_cam,
		KuroEngine::LightManager &arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix> &arg_matArray,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None,
		int arg_layer = 0,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//�G�b�W�`��
	void DrawEdge(DirectX::XMMATRIX arg_camView, DirectX::XMMATRIX arg_camProj, std::weak_ptr<KuroEngine::DepthStencil>arg_ds);

	//���C���̃����_�[�^�[�Q�b�g�Q�b�^
	std::shared_ptr<KuroEngine::RenderTarget> &GetRenderTarget(RENDER_TARGET_TYPE arg_type) { return m_renderTargetArray[arg_type]; }


	void DrawBillBoard(KuroEngine::Camera &arg_cam, KuroEngine::Transform &arg_transform, std::shared_ptr<KuroEngine::TextureBuffer>Tex, float alpha = 1.0f, const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None);
	void DrawBillBoard(KuroEngine::Camera &arg_cam, const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec2<float> &upSize, const KuroEngine::Vec2<float> &downSize, std::shared_ptr<KuroEngine::TextureBuffer>Tex, float alpha = 1.0f, const KuroEngine::AlphaBlendMode &arg_blendMode = KuroEngine::AlphaBlendMode_None);
};