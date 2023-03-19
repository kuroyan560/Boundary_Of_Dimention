#include "Sprite.h"
#include"KuroEngineDevice.h"

std::shared_ptr<KuroEngine::GraphicsPipeline>KuroEngine::Sprite::s_pipeline[AlphaBlendModeNum];
std::shared_ptr<KuroEngine::TextureBuffer>KuroEngine::Sprite::s_defaultTex;

KuroEngine::Sprite::Sprite(const std::shared_ptr<TextureBuffer>& Texture, const char* Name) : m_mesh(Name), m_name(Name)
{
	if (!s_pipeline[0])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/Sprite.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/Sprite.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"テクスチャシェーダーリソース"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カラー & 行列定数バッファ")
		};

		//レンダーターゲット描画先情報
		for (int i = 0; i < AlphaBlendModeNum; ++i)
		{
			std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)i) };
			//パイプライン生成
			s_pipeline[i] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, KuroEngine::SpriteMesh::Vertex::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(true, false) });
		}

		//白テクスチャ
		s_defaultTex = D3D12App::Instance()->GenerateTextureBuffer(Color(1.0f, 1.0f, 1.0f, 1.0f));
	}

	//デフォルトのテクスチャバッファ
	m_texBuff = s_defaultTex;

	//テクスチャセット
	SetTexture(Texture);

	//行列初期化
	m_constData.m_mat = m_transform.GetMatWorld();

	//定数バッファ生成
	m_constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(m_constData), 1, &m_constData, Name);
}

void KuroEngine::Sprite::SetTexture(const std::shared_ptr<TextureBuffer>& Texture)
{
	if (Texture == nullptr)return;
	m_texBuff = Texture;
	m_mesh.SetSize(m_texBuff->GetGraphSize().Float());
}

void KuroEngine::Sprite::SetColor(const Color& Color)
{
	if (m_constData.m_color == Color)return;
	m_constData.m_color = Color;
	m_constBuff->Mapping(&m_constData);
}

void KuroEngine::Sprite::Draw(const AlphaBlendMode& BlendMode)
{
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(s_pipeline[(int)BlendMode]);

	if (m_transform.IsDirty())
	{
		SendTransformBuff();
	}

	m_mesh.Render({
		{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},	//平行投影行列
		{m_texBuff,SRV},			//テクスチャリソース
		{m_constBuff,CBV}		//カラー & ワールド行列
		});
}

void KuroEngine::Sprite::SendTransformBuff()
{
	m_constData.m_mat = m_transform.GetMatWorld();
	m_constBuff->Mapping(&m_constData);
}
