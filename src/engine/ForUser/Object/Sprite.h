#pragma once
#include<memory>
#include"Common/Color.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"Common/Transform2D.h"

namespace KuroEngine
{
	class GraphicsPipeline;
	class ConstantBuffer;

	class Sprite
	{
	private:
		static std::shared_ptr<GraphicsPipeline>s_pipeline[AlphaBlendModeNum];
		//白のベタ塗りテクスチャ
		static std::shared_ptr<TextureBuffer>s_defaultTex;

	private:


		//定数バッファ送信用データ
		struct ConstantData
		{
			Matrix m_mat;
			Color m_color;
		}m_constData;

		//定数バッファ
		std::shared_ptr<ConstantBuffer>m_constBuff;

		//テクスチャバッファ
		std::shared_ptr<TextureBuffer>m_texBuff;

	public:
		//名前
		const std::string m_name;
		//トランスフォーム
		Transform2D m_transform;
		//メッシュ（頂点情報）
		SpriteMesh m_mesh;

		//テクスチャ、スプライト名
		Sprite(const std::shared_ptr<TextureBuffer>& Texture = nullptr, const char* Name = nullptr);

		//テクスチャセット
		void SetTexture(const std::shared_ptr<TextureBuffer>& Texture);

		//色セット（描画の際にこの値が乗算される）
		void SetColor(const Color& Color);

		//描画
		void Draw(const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);

		//直ちにトランスフォームバッファを送信
		//※ 現状、Dirtyフラグの性質上コンストラクタでのトランスフォーム変化に対応出来ない。
		void SendTransformBuff();

		//ゲッタ
		const std::shared_ptr<TextureBuffer>& GetTex()const { return m_texBuff; }
	};
}