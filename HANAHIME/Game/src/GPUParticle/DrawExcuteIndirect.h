#pragma once
#include"DirectX12/D3D12Data.h"

struct InitDrawIndexedExcuteIndirect
{
	std::vector<D3D12_INDIRECT_ARGUMENT_DESC> argument;
	D3D12_GPU_VIRTUAL_ADDRESS updateView;
	UINT elementNum;
	std::shared_ptr<KuroEngine::VertexBuffer> particleVertex;
	std::shared_ptr<KuroEngine::IndexBuffer> particleIndex;
	UINT indexNum;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootsignature;
};

struct InitDrawExcuteIndirect
{
	std::vector<D3D12_INDIRECT_ARGUMENT_DESC> argument;
	D3D12_GPU_VIRTUAL_ADDRESS updateView;
	UINT elementNum;
	std::shared_ptr<KuroEngine::VertexBuffer> particleVertex;
	UINT vertNum;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootsignature;
};

/// <summary>
/// ExcuteIndirectを簡単に使用する為のクラス
/// </summary>
class DrawExcuteIndirect
{
public:
	DrawExcuteIndirect(const InitDrawIndexedExcuteIndirect &INIT_DATA);
	DrawExcuteIndirect(const InitDrawExcuteIndirect &INIT_DATA);
	void Draw(KuroEngine::GraphicsPipeline &pipeline, const Microsoft::WRL::ComPtr<ID3D12Resource> &COUNTER_BUFFER);

private:

	struct DrawIndexedIndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS view;
		D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
	};
	struct DrawIndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS view;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_cmdBuffer;


	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSig;
	InitDrawIndexedExcuteIndirect initData;
	D3D12_INDIRECT_ARGUMENT_TYPE drawType;
};

