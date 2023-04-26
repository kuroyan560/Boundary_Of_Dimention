#pragma once
#include"Render/RenderObject/Camera.h"
#include<memory>
#include<vector>
#include"DirectX12/D3D12Data.h"

namespace KuroEngine
{
	class GaussianBlur;
	class ModelObject;
	class ShadowMapDevice
	{
		std::shared_ptr<RenderTarget>m_shadowMap;
		std::shared_ptr<DepthStencil>m_shadowMapDepth;
		Camera m_lightCamera;
		std::shared_ptr<GaussianBlur> m_gaussianBlur;

	public:
		ShadowMapDevice();
		void DrawShadowMap(const std::vector<std::weak_ptr<ModelObject>>& Models);
		void DrawShadowReceiver(const std::vector<std::weak_ptr<ModelObject>>& Models, Camera& GameCamera, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, int Layer = 0);

		void SetPos(const Vec3<float>Pos = { 0.0f,10.0f,0.0f });
		void SetTarget(const Vec3<float>Target = { 0,0,0 });
		void SetBlurPower(const float& BlurPower = 8.0f);
	};
}