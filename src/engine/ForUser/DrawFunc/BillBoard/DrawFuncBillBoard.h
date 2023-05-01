#pragma once
#include<map>
#include"KuroEngine.h"
#include"DirectX12/D3D12Data.h"

namespace KuroEngine
{
	class Camera;
	class DrawFuncBillBoard
	{
		//DrawBox
		static int s_drawBoxCount;
		//DrawGraph
		static int s_drawGraphCount;

		//専用頂点
		class Vertex
		{
		public:
			Vec3<float>m_pos;
			Vec2<float>m_size;
			Color m_col;
			Vertex(const Vec3<float>& Pos, const Vec2<float>& Size, const Color& Color)
				:m_pos(Pos), m_size(Size), m_col(Color) {}
		};

		//パイプライン
		static std::map<DXGI_FORMAT, std::array<std::shared_ptr<GraphicsPipeline>, AlphaBlendModeNum>>s_pipeline;
		static std::vector<std::shared_ptr<VertexBuffer>>s_graphVertBuff;

	public:
		static void GeneratePipeline(DXGI_FORMAT Format, const AlphaBlendMode& BlendMode);
		//事前に頂点バッファを生成しておく
		static void PrepareGraphVertBuff(int arg_num);

		//呼び出しカウントリセット
		static void CountReset()
		{
			s_drawBoxCount = 0;
			s_drawGraphCount = 0;
		}

		//四角描画
		static void Box(Camera& Cam, const Vec3<float>& Pos, const Vec2<float>& Size, const Color& BoxColor, const AlphaBlendMode& BlendMode = AlphaBlendMode_None, const int& arg_layer = 0);
		static void Graph(Camera& Cam, const Vec3<float>& Pos, const Vec2<float>& Size, std::shared_ptr<TextureBuffer>Tex, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const int& arg_layer = 0);
		static void Graph(Camera &Cam, const Vec3<float> &Pos, const Vec2<float> &Size, std::shared_ptr<TextureBuffer>Tex, float alpha, const AlphaBlendMode &BlendMode = AlphaBlendMode_Trans);
	};
}