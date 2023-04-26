#pragma once
#include"DirectX12/D3D12Data.h"

namespace KuroEngine
{
	//グラフィックスマネージャ
	class GraphicsManager
	{
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

#pragma region 専用のグラフィックスコマンド
		//グラフィックスコマンド基底クラス
		class GraphicsCommandBase
		{
		public:
			virtual void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList) = 0;
		};

		//レンダーターゲットセットコマンド
		class SetRenderTargetsCommand : public GraphicsCommandBase
		{
			SetRenderTargetsCommand() = delete;

			const std::vector<std::weak_ptr<RenderTarget>>m_renderTargets;
			const std::weak_ptr<DepthStencil>m_depthStencil;
		public:
			SetRenderTargetsCommand(const std::vector<std::weak_ptr<RenderTarget>>& RTs, const std::weak_ptr<DepthStencil>& DS) :m_renderTargets(RTs), m_depthStencil(DS) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//グラフィックスパイプラインセットコマンド
		class SetGraphicsPipelineCommand : public GraphicsCommandBase
		{
			SetGraphicsPipelineCommand() = delete;

			std::weak_ptr<GraphicsPipeline> m_gPipeline;
		public:
			SetGraphicsPipelineCommand(std::weak_ptr<GraphicsPipeline> Pipeline) :m_gPipeline(Pipeline) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override
			{
				m_gPipeline.lock()->SetPipeline(CmdList);
			}
			const int& GetPipelineHandle() { return m_gPipeline.lock()->GetPipelineHandle(); }
		};

		//コンピュートパイプラインセットコマンド
		class SetComputePipelineCommand : public GraphicsCommandBase
		{
			SetComputePipelineCommand() = delete;

			std::weak_ptr<ComputePipeline> m_cPipeline;
		public:
			SetComputePipelineCommand(std::weak_ptr<ComputePipeline> Pipeline) :m_cPipeline(Pipeline) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override
			{
				m_cPipeline.lock()->SetPipeline(CmdList);
			}
			const int& GetPipelineHandle() { return m_cPipeline.lock()->GetPipelineHandle(); }
		};

		//レンダーターゲットクリアコマンド
		class ClearRTVCommand : public GraphicsCommandBase
		{
			ClearRTVCommand() = delete;
			std::weak_ptr<RenderTarget>m_renderTarget;

		public:
			ClearRTVCommand(const std::weak_ptr<RenderTarget>& RenderTarget) :m_renderTarget(RenderTarget) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//デプスステンシルクリアコマンド
		class ClearDSVCommand : public GraphicsCommandBase
		{
			ClearDSVCommand() = delete;
			std::weak_ptr<DepthStencil>m_depthStencil;
		public:
			ClearDSVCommand(const std::weak_ptr<DepthStencil>& DepthStencil) :m_depthStencil(DepthStencil) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//レンダリング情報（レンダーコマンドに渡す情報）
		//オブジェクトのレンダリングコマンド
		class RenderCommand : public GraphicsCommandBase
		{
			RenderCommand() = delete;

			std::weak_ptr<VertexBuffer>m_vertexBuff;
			std::weak_ptr<IndexBuffer>m_idxBuff;
			std::vector<RegisterDescriptorData>m_registerDescDatas;
			const int m_instanceNum = 1;	//インスタンス数（インスタンシング描画用）

		public:
			int m_layer = 0;	//レイヤー（増えると描画が後回しに）
			const bool m_trans = false;	//透過オブジェクト

			//インデックスなし
			RenderCommand(const std::weak_ptr<VertexBuffer>& VertexBuff,
				const std::vector<RegisterDescriptorData>& DescDatas,
				const int& Layer,
				const bool& TransFlg,
				const int& InstanceNum = 1)
				:m_vertexBuff(VertexBuff), m_registerDescDatas(DescDatas), m_layer(Layer), m_trans(TransFlg), m_instanceNum(InstanceNum) {}

			//インデックスあり
			RenderCommand(const std::weak_ptr<VertexBuffer>& VertexBuff,
				const std::weak_ptr<IndexBuffer>& IndexBuff,
				const std::vector<RegisterDescriptorData>& DescDatas,
				const int& Layer,
				const bool& TransFlg,
				const int& InstanceNum = 1)
				:m_vertexBuff(VertexBuff), m_idxBuff(IndexBuff), m_registerDescDatas(DescDatas), m_layer(Layer), m_trans(TransFlg), m_instanceNum(InstanceNum) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//コンピュートシェーダ用Dispatchコマンド
		class DispatchCommand : public GraphicsCommandBase
		{
			DispatchCommand() = delete;

			const Vec3<int>m_threadNum;
			std::vector<RegisterDescriptorData>m_registerDescDatas;

		public:
			DispatchCommand(const Vec3<int>& ThreadNum,
				const std::vector<RegisterDescriptorData>& DescDatas)
				:m_threadNum(ThreadNum), m_registerDescDatas(DescDatas) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//テクスチャのコピー
		class CopyTex : public GraphicsCommandBase
		{
			std::weak_ptr<TextureBuffer>m_destTex;
			std::weak_ptr<TextureBuffer>m_srcTex;
		public:
			CopyTex(const std::weak_ptr<TextureBuffer>& DestTex, const std::weak_ptr<TextureBuffer>& SrcTex)
				:m_destTex(DestTex), m_srcTex(SrcTex) {}
			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

		//ExcuteIndirect
		class ExcuteIndirectCommand : public GraphicsCommandBase
		{
			std::weak_ptr<IndirectCommandBuffer>m_cmdBuff;
			std::weak_ptr<IndirectDevice>m_indirectDevice;
			UINT m_argBufferOffset = 0;
			std::weak_ptr<VertexBuffer>m_vertBuff;
			std::weak_ptr<IndexBuffer>m_idxBuff;
			std::vector<RegisterDescriptorData>m_registerDescDatas;

		public:
			ExcuteIndirectCommand(const std::weak_ptr<IndirectCommandBuffer>& CmdBuff, const std::weak_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset,
				const std::weak_ptr<VertexBuffer>& VertBuff = std::weak_ptr<VertexBuffer>(), const std::weak_ptr<IndexBuffer>& IdxBuff = std::weak_ptr<IndexBuffer>(),
				const std::vector<RegisterDescriptorData> &DescDatas = {})
				:m_cmdBuff(CmdBuff), m_indirectDevice(IndirectDevice), m_argBufferOffset(ArgBufferOffset), m_vertBuff(VertBuff), m_idxBuff(IdxBuff),m_registerDescDatas(DescDatas) {}

			void Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)override;
		};

#pragma endregion


	private:
		//グラフィックスコマンドリスト
		std::list<std::shared_ptr<GraphicsCommandBase>>m_gCommands;

		//レンダリングコマンドリスト（ソートのため一時グラフィックスコマンドリストとは別で積み上げる）
		std::list<std::shared_ptr<RenderCommand>>m_renderCommands;

		//最後にセットされたいパイプラインがグラフィックスかコンピュートか
		enum PIPELINE_TYPE { GRAPHICS, COMPUTE, NONE }m_recentPipelineType = NONE;

		//最後にセットされたパイプラインハンドル
		int m_recentPipelineHandle = -1;

		//最後にセットされたレンダーターゲットのフォーマット
		std::vector<DXGI_FORMAT>m_recentRenderTargetFormat;

		//Zバッファ、透過するかどうかでソート
		void StackRenderCommands();

		//shader_ptr配列をweak_ptr配列に変換
		template<typename T>
		std::vector<std::weak_ptr<T>>ConvertToWeakPtrArray(const std::vector<std::shared_ptr<T>>& Array)
		{
			std::vector<std::weak_ptr<T>>result;
			for (int i = 0; i < Array.size(); ++i)
			{
				result.emplace_back(std::weak_ptr<T>(Array[i]));
			}
			return result;
		}

	public:
		//レンダーターゲットのセットコマンド積み上げ
		void SetRenderTargets(std::vector<std::weak_ptr<RenderTarget>> RTs, std::weak_ptr<DepthStencil> DS = std::weak_ptr<DepthStencil>());

		//グラフィックスパイプラインのセットコマンド積み上げ
		void SetGraphicsPipeline(std::weak_ptr<GraphicsPipeline> Pipeline);

		//コンピュートパイプラインのセットコマンド積み上げ
		void SetComputePipeline(std::weak_ptr<ComputePipeline> Pipeline);

		//レンダーターゲットのクリアコマンド積み上げ
		void ClearRenderTarget(std::weak_ptr<RenderTarget> RenderTarget);

		//デプスステンシルのクリアコマンド積み上げ
		void ClearDepthStencil(std::weak_ptr<DepthStencil> DepthStencil);

		//テクスチャコピーコマンド積み上げ
		void CopyTexture(std::weak_ptr<TextureBuffer> DestTex, std::weak_ptr<TextureBuffer> SrcTex);

		//オブジェクトのレンダリングコマンド積み上げ（インデックスなし）
		void ObjectRender(std::weak_ptr<VertexBuffer> VertexBuff,
			std::vector<RegisterDescriptorData> DescDatas,
			const int& Layer, const bool& TransFlg, const int& InstanceNum = 1);

		//オブジェクトのレンダリングコマンド積み上げ（インデックスなし）
		void ObjectRender(std::weak_ptr<VertexBuffer> VertexBuff, std::weak_ptr<IndexBuffer> IndexBuff,
			std::vector<RegisterDescriptorData> DescDatas,
			const int& Layer, const bool& TransFlg, const int& InstanceNum = 1);

		//ディスパッチコマンド積み上げ
		void Dispatch(const Vec3<int>& ThreadNum, std::vector<RegisterDescriptorData> DescDatas);

		//ExecuteIndirectコマンド積み上げ
		void ExecuteIndirectDispatch(const std::shared_ptr<IndirectCommandBuffer>& CmdBuff, const std::shared_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset = 0);
		void ExecuteIndirectDraw(const std::shared_ptr<VertexBuffer> &VertexBuff,
			const std::shared_ptr<IndirectCommandBuffer> &CmdBuff, const std::shared_ptr<IndirectDevice> &IndirectDevice, const UINT &ArgBufferOffset = 0);
		void ExecuteIndirectDrawIndexed(const std::shared_ptr<VertexBuffer>& VertexBuff, const std::shared_ptr<IndexBuffer>& IndexBuff,
			const std::shared_ptr<IndirectCommandBuffer>& CmdBuff, const std::shared_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset = 0,
			const std::vector<RegisterDescriptorData> &DescDatas = {});

		//コマンドリスト全実行
		void CommandsExcute(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& CmdList);

		//最後にセットされたレンダーターゲットのフォーマット
		const DXGI_FORMAT& GetRecentRenderTargetFormat(const int& Idx);
	};
}