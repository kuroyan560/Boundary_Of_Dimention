#pragma once
#include"Common/Vec.h"
#include<memory>
#include<string>
#include"DirectX12/D3D12Data.h"
#include<array>

namespace KuroEngine
{
	template<typename VertexType>
	class Mesh;

	class SpriteMesh
	{
	public:
		//頂点インデックス
		enum VERT_IDX { LB, LT, RB, RT, IDX_NUM };
		//スプライト専用頂点クラス
		class Vertex
		{
		public:
			Vec2<float>pos;
			Vec2<float>uv;

			static std::vector<InputLayoutParam>GetInputLayout()
			{
				static std::vector<InputLayoutParam>INPUT_LAYOUT =
				{
					InputLayoutParam("POSITION",DXGI_FORMAT_R32G32_FLOAT),
					InputLayoutParam("TEXCOORD",DXGI_FORMAT_R32G32_FLOAT)
				};
				return INPUT_LAYOUT;
			}
		};
	private:
		//メッシュ情報
		std::shared_ptr<Mesh<KuroEngine::SpriteMesh::Vertex>>mesh;

		//トリミングインデックス
		enum { TOP, BOTTOM, LEFT, RIGHT, TRIM_IDX_NUM };
		//各辺のトリミング率
		float trim[TRIM_IDX_NUM];

		//基準点
		Vec2<float>anchorPoint = { 0,0 };
		//サイズ
		Vec2<float>size = { 64,64 };
		//反転フラグ
		Vec2<bool>flip = { false,false };
		//頂点ごとの位置オフセット
		std::array<Vec2<float>, IDX_NUM>offset = { Vec2<float>(0,0) };
		//内部パラメータZバッファ
		int drawLayer = 0;

		//透過するか
		bool trans = false;

		bool dirty = true;

	public:
		SpriteMesh(const char* Name = nullptr);

		//セッタ
		void SetUv(const float& Top = 0.0f, const float& Buttom = 1.0f, const float& Left = 0.0f, const float& Right = 1.0f);
		void SetAnchorPoint(const Vec2<float>& AnchorPoint) {
			if (anchorPoint == AnchorPoint)return;
			anchorPoint = AnchorPoint;
			dirty = true;
		}
		void SetSize(const Vec2<float>& Size) {
			if (size == Size)return;
			size = Size;
			dirty = true;
		}
		void SetFlip(const Vec2<bool>& Flip) {
			if (flip == Flip)return;
			flip = Flip;
			dirty = true;
		}
		void SetZLayer(const int& Layer) {
			drawLayer = Layer;
		}
		void SetTransFlg(const bool& TransFlg) {
			trans = TransFlg;
		}

		//ゲッタ
		const Vec2<float>& GetSize()const { return size; }

		//画像上の座標比率（0.0f ~ 1.0f）を指定してトリミング
		void Trimming(const float& Top = 0.0f, const float& Bottom = 1.0f, const float& Left = 0.0f, const float& Right = 1.0f) {
			if (Top < 0 || 1.0f < Top)assert(0);
			if (Bottom < 0 || 1.0f < Bottom)assert(0);
			if (Left < 0 || 1.0f < Left)assert(0);
			if (Right < 0 || 1.0f < Right)assert(0);

			if (Bottom < Top)assert(0);
			if (Right < Left)assert(0);

			trim[TOP] = Top;
			trim[BOTTOM] = Bottom;
			trim[LEFT] = Left;
			trim[RIGHT] = Right;

			dirty = true;
		}

		//頂点ごとの位置オフセット
		void SetOffset(VERT_IDX VertIdx, Vec2<float>Offset)
		{
			offset[VertIdx] = Offset;
			dirty = true;
		}

		//バッファ送信即時実行
		void SendBuff();

		void Render(const std::vector<RegisterDescriptorData>& DescDatas, const int& InstanceNum = 1);
	};
}