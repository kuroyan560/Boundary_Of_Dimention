#pragma once
#include"Common/Color.h"
#include"DirectX12/D3D12Data.h"
#include<vector>
#include"Common/Transform.h"
#include<memory>
#include"Render/RenderObject/Camera.h"
#include"ForUser/Object/Object.h"
#include<map>

namespace KuroEngine
{
	class Model;
	class LightManager;
	class CubeMap;
	class ModelAnimator;

	class DrawFunc3D
	{
		//DrawLine
		static int s_drawLineCount;
		static std::map<DXGI_FORMAT, std::array<std::shared_ptr<GraphicsPipeline>, AlphaBlendModeNum>>s_drawLinePipeline;

		//DrawNonShadingModel
		static int s_drawNonShadingCount;
		static int s_drawNonShadingCountHuge;
		//DrawADSShadingModel
		static int s_drawAdsShadingCount;
		//DrawPBRShadingModel
		static int s_drawPbrShadingCount;
		//DrawToonModel
		static int s_drawToonCount;
		//DrawShadowFallModel
		static int s_drawShadowFallCount;

		//DrawNonShadingPlane
		static int s_drawNonShadingPlaneCount;

	public:
		//インスタンシング描画で一度に描画できるインスタンス最大数
		static const int INSTANCE_MAX = 1024;

		//呼び出しカウントリセット
		static void CountReset()
		{
			s_drawLineCount = 0;
			s_drawNonShadingCount = 0;
			s_drawNonShadingCountHuge = 0;
			s_drawAdsShadingCount = 0;
			s_drawPbrShadingCount = 0;
			s_drawToonCount = 0;
			s_drawShadowFallCount = 0;
			s_drawNonShadingPlaneCount = 0;
		}

		//線描画パイプライン事前生成
		static void GenerateDrawLinePipeline(DXGI_FORMAT arg_format, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans);
		//線描画
		static void DrawLine(Camera& arg_cam, const Vec3<float>& arg_from, const Vec3<float>& arg_to, const Color& arg_lineColor, const float& arg_thickness, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, const int& arg_layer = 0);
		//通常描画
		static void DrawNonShadingModel(const std::weak_ptr<Model>arg_model, const Matrix& arg_worldMat, Camera& arg_camera, const float& arg_alpha = 1.0f, std::shared_ptr<ModelAnimator> arg_animator = nullptr, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0);
		static void DrawNonShadingModel(const std::weak_ptr<Model>arg_model, Transform& arg_transform, Camera& arg_camera, const float& arg_alpha = 1.0f, std::shared_ptr<ModelAnimator> arg_animator = nullptr, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0)
		{
			DrawNonShadingModel(arg_model, arg_transform.GetMatWorld(), arg_camera, arg_alpha, arg_animator, arg_blendMode, arg_layer);
		}
		static void DrawNonShadingModel(const std::weak_ptr<ModelObject>arg_modelObject, Camera& arg_camera, const float& arg_alpha = 1.0f, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans)
		{
			auto obj = arg_modelObject.lock();
			DrawNonShadingModel(obj->m_model, obj->m_transform, arg_camera, arg_alpha, obj->m_animator, arg_blendMode);
		}
		static void DrawNonShadingModel(const std::weak_ptr<Model>arg_model, const std::vector<Matrix>& arg_matArray, Camera& arg_Camera, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, const int& arg_layer = 0, std::shared_ptr<ModelAnimator> arg_animator = nullptr);
		//影つき描画
		static void DrawADSShadingModel(LightManager& arg_ligManager, const std::weak_ptr<Model>arg_model, Transform& arg_transform, Camera& arg_cam, std::shared_ptr<ModelAnimator> arg_animator = nullptr, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0);
		static void DrawADSShadingModel(LightManager& arg_ligManager, const std::weak_ptr<ModelObject>arg_modelObject, Camera& arg_cam, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0)
		{
			auto obj = arg_modelObject.lock();
			DrawADSShadingModel(arg_ligManager, obj->m_model, obj->m_transform, arg_cam, obj->m_animator, arg_blendMode, arg_layer);
		}
		//影つき描画(PBR)
		static void DrawPBRShadingModel(LightManager& arg_ligManager, const std::weak_ptr<Model>arg_model, Transform& arg_transform, Camera& arg_cam, std::shared_ptr<ModelAnimator> arg_animator = nullptr, std::shared_ptr<CubeMap>arg_attachCubeMap = nullptr, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0);
		static void DrawPBRShadingModel(LightManager& arg_ligManager, const std::weak_ptr<ModelObject>arg_modelObject, Camera& arg_cam, std::shared_ptr<CubeMap>arg_attachCubeMap = nullptr, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0)
		{
			auto obj = arg_modelObject.lock();
			DrawPBRShadingModel(arg_ligManager, obj->m_model, obj->m_transform, arg_cam, obj->m_animator, arg_attachCubeMap, arg_blendMode, arg_layer);
		}
		//トゥーンシェーディング
		static void DrawToonModel(const std::weak_ptr<TextureBuffer>arg_toonTex, LightManager& arg_ligManager, const std::weak_ptr<Model>arg_model, Transform& arg_transform, Camera& arg_cam, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0);
		static void DrawToonModel(const std::weak_ptr<TextureBuffer>arg_toonTex, LightManager& arg_ligManager, const std::weak_ptr<ModelObject>arg_modelObject, Camera& arg_cam, const AlphaBlendMode& arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0)
		{
			auto obj = arg_modelObject.lock();
			DrawToonModel(arg_toonTex, arg_ligManager, obj->m_model, obj->m_transform, arg_cam, arg_blendMode, arg_layer);
		}

		//テクスチャを設定して板ポリ通常描画(サイズ：1x1）
		static void DrawNonShadingPlane(const std::weak_ptr<TextureBuffer>arg_tex, Transform& arg_transform, Camera& arg_camera, const float& arg_alpha = 1.0f, const Vec2<bool>& arg_mirror = { false,false }, AlphaBlendMode arg_blendMode = AlphaBlendMode_Trans, int arg_layer = 0);

	};
}