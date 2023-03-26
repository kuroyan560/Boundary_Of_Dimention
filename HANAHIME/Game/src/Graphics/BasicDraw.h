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

	//トゥーンシェーダーの共有パラメータ
	struct ToonCommonParameter
	{
		//明るさのしきい値（範囲を持たせている）
		float m_brightThresholdLow = 0.66f;
		float m_brightThresholdRange = 0.03f;
		//リムライトの影響部分をそのままの色で出力する際のしきい値
		float m_limThreshold = 0.4f;
	};
	ToonCommonParameter m_toonCommonParam;

	//エッジの共有パラメータ
	struct EdgeCommonParameter
	{
		//エッジ描画の判断をする深度差のしきい値
		float m_depthThreshold = 0.19f;
		float m_pad[3];
		//深度値を比べるテクセルへのUVオフセット（近傍8）
		std::array<KuroEngine::Vec2<float>, 8>m_uvOffset;
	};
	EdgeCommonParameter m_edgeShaderParam;

	//モデル通常描画カウント
	int m_drawCount = 0;
	//モデルインスタンシング描画カウント
	int m_drawCountHuge = 0;
	//使用するトゥーン情報のカウント
	int m_individualParamCount = 0;

	//共通のトゥーンシェーダー用情報
	std::shared_ptr<KuroEngine::ConstantBuffer>m_toonCommonParamBuff;
	//描画毎のトゥーン描画情報
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_toonIndividualParamBuff;

	//モデル描画
	std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>m_drawPipeline;
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuff;

	//インスタンシグ描画で一度に描画できるインスタンス最大数
	static const int s_instanceMax = 1024;
	//インスタンシング描画でデプスステンシルに深度を描画するかのタブ
	enum INSTANCING_DRAW_MODE { WRITE_DEPTH, NOT_WRITE_DEPTH };
	//モデル描画（インスタンシング描画）
	std::array<std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>, 2>m_instancingDrawPipeline;
	std::vector<std::shared_ptr<KuroEngine::ConstantBuffer>>m_drawTransformBuffHuge;


	//エッジ出力＆描画
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
	/// 描画
	/// </summary>
	/// <param name="arg_cam">カメラ</param>
	/// <param name="arg_ligMgr">ライトマネージャ</param>
	/// <param name="arg_model">モデル</param>
	/// <param name="arg_transform">トランスフォーム</param>
	/// <param name="arg_toonParam">トゥーンのパラメータ</param>
	/// <param name="arg_boneBuff">ボーンバッファ</param>
	void Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform& arg_transform, 
		const IndividualDrawParameter& arg_toonParam, 
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//描画（デフォルトのトゥーンパラメータを使用）
	void Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr, 
		std::weak_ptr<KuroEngine::Model>arg_model, 
		KuroEngine::Transform& arg_transform, 
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//モデルオブジェクト描画
	void BasicDraw::Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const IndividualDrawParameter& arg_toonParam,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None);

	//モデルオブジェクト描画（デフォルトのトゥーンパラメータを使用）
	void BasicDraw::Draw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None);

	//インスタンシング描画
	void InstancingDraw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix>& arg_matArray,
		const IndividualDrawParameter& arg_toonParam,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None, 
		const float& arg_depth = 0.0f, 
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	//インスタンシング描画（デフォルトのトゥーンパラメータを使用）
	void InstancingDraw(KuroEngine::Camera& arg_cam,
		KuroEngine::LightManager& arg_ligMgr,
		std::weak_ptr<KuroEngine::Model>arg_model,
		std::vector<KuroEngine::Matrix>& arg_matArray,
		bool arg_depthWriteMask,
		const KuroEngine::AlphaBlendMode& arg_blendMode = KuroEngine::AlphaBlendMode_None,
		const float& arg_depth = 0.0f,
		std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff = nullptr);

	/// <summary>
	/// エッジ描画
	/// </summary>
	/// <param name="arg_depthMap">深度マップ</param>
	/// <param name="arg_normalMap">法線マップ</param>
	/// <param name="arg_edgeColorMap">エッジカラーマップ</param>
	void DrawEdge(std::shared_ptr<KuroEngine::TextureBuffer>arg_depthMap, std::shared_ptr<KuroEngine::TextureBuffer>arg_normalMap, std::shared_ptr<KuroEngine::TextureBuffer>arg_edgeColorMap);
};