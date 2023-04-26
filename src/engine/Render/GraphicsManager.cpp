#include "GraphicsManager.h"

void KuroEngine::GraphicsManager::SetRenderTargetsCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
	for (auto ptr : m_renderTargets)
	{
		rtvs.emplace_back(ptr.lock()->AsRTV(CmdList));
	}

	const Vec2<float> targetSize = m_renderTargets[0].lock()->GetGraphSize().Float();
	//ビューポート設定
	auto viewPort = CD3DX12_VIEWPORT(0.0f, 0.0f, targetSize.x, targetSize.y);
	CmdList->RSSetViewports(1, &viewPort);

	//シザー矩形設定
	auto rect = CD3DX12_RECT(0, 0, static_cast<LONG>(targetSize.x), static_cast<LONG>(targetSize.y));
	CmdList->RSSetScissorRects(1, &rect);

	//デプスステンシルがある場合
	if (auto ptr = m_depthStencil.lock())
	{
		CmdList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), &rtvs[0], FALSE, ptr->AsDSV(CmdList));
	}
	//ない場合
	else
	{
		CmdList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), &rtvs[0], FALSE, nullptr);
	}
}

void KuroEngine::GraphicsManager::ClearRTVCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	m_renderTarget.lock()->Clear(CmdList);
}

void KuroEngine::GraphicsManager::ClearDSVCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	m_depthStencil.lock()->Clear(CmdList);
}

void KuroEngine::GraphicsManager::RenderCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	//ディスクリプタセット
	for (int i = 0; i < m_registerDescDatas.size(); ++i)
	{
		if (!m_registerDescDatas[i].m_descData)continue;
		m_registerDescDatas[i].SetAsGraphics(CmdList, i);
	}

	m_vertexBuff.lock()->ChangeBarrierForVertexBuffer(CmdList);

	//頂点ビューセット
	m_vertexBuff.lock()->SetView(CmdList);

	//インデックスなし
	if (m_idxBuff.expired())
	{
		CmdList->DrawInstanced(m_vertexBuff.lock()->m_sendVertexNum, m_instanceNum, 0, 0);
	}
	//インデックスあり
	else
	{
		m_idxBuff.lock()->SetView(CmdList);
		CmdList->DrawIndexedInstanced(m_idxBuff.lock()->m_indexNum, m_instanceNum, 0, 0, 0);
	}
}

void KuroEngine::GraphicsManager::DispatchCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	//ディスクリプタセット
	for (int i = 0; i < m_registerDescDatas.size(); ++i)
	{
		if (!m_registerDescDatas[i].m_descData)continue;
		m_registerDescDatas[i].SetAsCompute(CmdList, i);
	}
	CmdList->Dispatch(static_cast<UINT>(m_threadNum.x), static_cast<UINT>(m_threadNum.y), static_cast<UINT>(m_threadNum.z));
}


void KuroEngine::GraphicsManager::SetRenderTargets(std::vector<std::weak_ptr<RenderTarget>> RTs, std::weak_ptr<DepthStencil> DS)
{
	if (!m_renderCommands.empty())StackRenderCommands();	//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
	m_gCommands.emplace_back(std::make_shared<SetRenderTargetsCommand>(RTs, DS));

	//最新のレンダーターゲットのフォーマットを記録しておく
	m_recentRenderTargetFormat.clear();
	for (auto& rt : RTs)
	{
		m_recentRenderTargetFormat.emplace_back(rt.lock()->GetDesc().Format);
	}
}

void KuroEngine::GraphicsManager::SetGraphicsPipeline(std::weak_ptr<GraphicsPipeline> Pipeline)
{
	//新しいパイプラインのハンドル取得
	const int newPipelineHandle = Pipeline.lock()->GetPipelineHandle();

	//既にセットされていたものと同じならスルー
	if (m_recentPipelineType == GRAPHICS && newPipelineHandle == m_recentPipelineHandle)return;

	if (!m_renderCommands.empty())StackRenderCommands();	//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
	m_gCommands.emplace_back(std::make_shared<SetGraphicsPipelineCommand>(Pipeline));

	//パイプラインタイプ記録
	m_recentPipelineType = GRAPHICS;

	//パイプラインハンドル記録
	m_recentPipelineHandle = newPipelineHandle;
}

void KuroEngine::GraphicsManager::SetComputePipeline(std::weak_ptr<ComputePipeline> Pipeline)
{
	//新しいパイプラインのハンドル取得
	const int newPipelineHandle = Pipeline.lock()->GetPipelineHandle();

	//既にセットされていたものと同じならスルー
	if (m_recentPipelineType == COMPUTE && newPipelineHandle == m_recentPipelineHandle)return;

	if (!m_renderCommands.empty())StackRenderCommands();	//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
	m_gCommands.emplace_back(std::make_shared<SetComputePipelineCommand>(Pipeline));

	//パイプラインタイプ記録
	m_recentPipelineType = COMPUTE;

	//パイプラインハンドル記録
	m_recentPipelineHandle = newPipelineHandle;
}

void KuroEngine::GraphicsManager::ClearRenderTarget(std::weak_ptr<RenderTarget> RenderTarget)
{
	if (!m_renderCommands.empty())StackRenderCommands();	//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
	m_gCommands.emplace_back(std::make_shared<ClearRTVCommand>(RenderTarget));
}

void KuroEngine::GraphicsManager::ClearDepthStencil(std::weak_ptr<DepthStencil> DepthStencil)
{
	if (!m_renderCommands.empty())StackRenderCommands();	//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
	m_gCommands.emplace_back(std::make_shared<ClearDSVCommand>(DepthStencil));
}

void KuroEngine::GraphicsManager::CopyTexture(std::weak_ptr<TextureBuffer> DestTex, std::weak_ptr<TextureBuffer> SrcTex)
{
	m_gCommands.emplace_back(std::make_shared<CopyTex>(DestTex, SrcTex));
}

void KuroEngine::GraphicsManager::ObjectRender(std::weak_ptr<VertexBuffer> VertexBuff, std::vector<RegisterDescriptorData> DescDatas,
	const int& Layer, const bool& TransFlg, const int& InstanceNum)
{
	//ソートするので gCommands ではなく一時的にrenderCommandsに積む
	m_renderCommands.emplace_back(std::make_shared<RenderCommand>(VertexBuff, DescDatas, Layer, TransFlg, InstanceNum));
}

void KuroEngine::GraphicsManager::ObjectRender(std::weak_ptr<VertexBuffer> VertexBuff,
	std::weak_ptr<IndexBuffer> IndexBuff,
	std::vector<RegisterDescriptorData> DescDatas,
	const int& Layer, const bool& TransFlg, const int& InstanceNum)
{
	//ソートするので gCommands ではなく一時的にrenderCommandsに積む
	m_renderCommands.emplace_back(std::make_shared<RenderCommand>(VertexBuff, IndexBuff, DescDatas, Layer, TransFlg, InstanceNum));
}

void KuroEngine::GraphicsManager::Dispatch(const Vec3<int>& ThreadNum, std::vector<RegisterDescriptorData> DescDatas)
{
	m_gCommands.emplace_back(std::make_shared<DispatchCommand>(ThreadNum, DescDatas));
}

void KuroEngine::GraphicsManager::ExecuteIndirectDispatch(const std::shared_ptr<IndirectCommandBuffer>& CmdBuff, const std::shared_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset)
{
	assert(CmdBuff->IndirectType() == DISPATCH);
	assert(IndirectDevice->IndirectType() == DISPATCH);
	m_gCommands.emplace_back(std::make_shared<ExcuteIndirectCommand>(CmdBuff, IndirectDevice, ArgBufferOffset));
}

void KuroEngine::GraphicsManager::ExecuteIndirectDraw(const std::shared_ptr<VertexBuffer>& VertexBuff, const std::shared_ptr<IndirectCommandBuffer>& CmdBuff, const std::shared_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset)
{
	assert(CmdBuff->IndirectType() == DRAW);
	assert(IndirectDevice->IndirectType() == DRAW);
	m_gCommands.emplace_back(std::make_shared<ExcuteIndirectCommand>(CmdBuff, IndirectDevice, ArgBufferOffset, VertexBuff));
}

void KuroEngine::GraphicsManager::ExecuteIndirectDrawIndexed(const std::shared_ptr<VertexBuffer>& VertexBuff, const std::shared_ptr<IndexBuffer>& IndexBuff, const std::shared_ptr<IndirectCommandBuffer>& CmdBuff, const std::shared_ptr<IndirectDevice>& IndirectDevice, const UINT& ArgBufferOffset, const std::vector<RegisterDescriptorData> &DescDatas)
{
	assert(CmdBuff->IndirectType() == DRAW_INDEXED);
	assert(IndirectDevice->IndirectType() == DRAW_INDEXED);
	m_gCommands.emplace_back(std::make_shared<ExcuteIndirectCommand>(CmdBuff, IndirectDevice, ArgBufferOffset, VertexBuff, IndexBuff, DescDatas));
}


void KuroEngine::GraphicsManager::StackRenderCommands()
{
	//ソート
	//Z値より透過するかどうかが優先度高い
	m_renderCommands.sort([](std::shared_ptr<RenderCommand> a, std::shared_ptr<RenderCommand> b)
		{
			if (a->m_trans == b->m_trans)
			{
				//Zソート（添字小さい = 奥）
				return b->m_layer < a->m_layer;
			}
			else
			{
				return !a->m_trans && b->m_trans;
			}
		});

	//一括スタック
	for (auto ptr : m_renderCommands)
	{
		//キャストしてグラフィックコマンドに積む
		m_gCommands.emplace_back(std::static_pointer_cast<GraphicsCommandBase>(ptr));
	}

	//レンダリングコマンドクリア
	m_renderCommands.clear();
}

void KuroEngine::GraphicsManager::CommandsExcute(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	//最後に積み上げられたのがレンダリングコマンドだった場合
	if (!m_renderCommands.empty())
	{
		//Zバッファ＆透過するかどうかでソートしてグラフィックスコマンドリストに一括スタック
		StackRenderCommands();
	}

	int cmdIdx = 0;
	for (auto itr = m_gCommands.begin(); itr != m_gCommands.end(); ++itr)
	{
		(*itr)->Execute(CmdList);
		cmdIdx++;
	}

	//コマンドリストクリア
	m_gCommands.clear();
	m_recentPipelineType = NONE;
	m_recentPipelineHandle = -1;
	m_recentRenderTargetFormat.clear();
}

void KuroEngine::GraphicsManager::CopyTex::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	m_destTex.lock()->CopyTexResource(CmdList, m_srcTex.lock().get());
}

void KuroEngine::GraphicsManager::ExcuteIndirectCommand::Execute(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	//ディスクリプタセット
	for (int i = 0; i < m_registerDescDatas.size(); ++i)
	{
		if (!m_registerDescDatas[i].m_descData)continue;
		m_registerDescDatas[i].SetAsGraphics(CmdList, 1);
	}

	if (auto buff = m_vertBuff.lock())buff->SetView(CmdList);
	if (auto buff = m_idxBuff.lock())buff->SetView(CmdList);

	m_indirectDevice.lock()->Execute(CmdList, m_cmdBuff.lock(), m_argBufferOffset);
}

const DXGI_FORMAT& KuroEngine::GraphicsManager::GetRecentRenderTargetFormat(const int& Idx)
{
	assert(!m_recentRenderTargetFormat.empty());
	assert(m_recentRenderTargetFormat[Idx] != DXGI_FORMAT_UNKNOWN);
	return m_recentRenderTargetFormat[Idx];
}