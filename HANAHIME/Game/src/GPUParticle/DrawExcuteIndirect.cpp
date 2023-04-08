#include "DrawExcuteIndirect.h"
#include"../KazLibrary/DirectXCommon/DirectX12Device.h"
#include"../KazLibrary/DirectXCommon/DirectX12CmdList.h"
#include"../KazLibrary/Pipeline/GraphicsRootSignature.h"
#include"../KazLibrary/RenderTarget/RenderTargetStatus.h"

DrawExcuteIndirect::DrawExcuteIndirect(const InitDrawIndexedExcuteIndirect &INIT_DATA) :initData(INIT_DATA)
{
	drawType = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	//コマンドシグネチャ---------------------------
	D3D12_COMMAND_SIGNATURE_DESC desc{};
	desc.pArgumentDescs = INIT_DATA.argument.data();
	desc.NumArgumentDescs = static_cast<UINT>(INIT_DATA.argument.size());
	desc.ByteStride = sizeof(DrawIndexedIndirectCommand);

	HRESULT lResult =
		DirectX12Device::Instance()->dev->CreateCommandSignature(&desc, GraphicsRootSignature::Instance()->GetRootSignature(INIT_DATA.rootsignatureName).Get(), IID_PPV_ARGS(&commandSig));
	//コマンドシグネチャ---------------------------
	if (lResult != S_OK)
	{
		assert(0);
	}

	//Indirect用のバッファ生成
	drawCommandHandle = buffers.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(sizeof(DrawIndexedIndirectCommand)));

	DrawIndexedIndirectCommand command;
	command.drawArguments.IndexCountPerInstance = INIT_DATA.indexNum;
	command.drawArguments.InstanceCount = INIT_DATA.elementNum;
	command.drawArguments.StartIndexLocation = 0;
	command.drawArguments.StartInstanceLocation = 0;
	command.drawArguments.BaseVertexLocation = 0;
	command.view = INIT_DATA.updateView;
	buffers.TransData(drawCommandHandle, &command, sizeof(DrawIndexedIndirectCommand));
}

DrawExcuteIndirect::DrawExcuteIndirect(const InitDrawExcuteIndirect &INIT_DATA)
{
	drawType = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

	//コマンドシグネチャ---------------------------
	D3D12_COMMAND_SIGNATURE_DESC desc{};
	desc.pArgumentDescs = INIT_DATA.argument.data();
	desc.NumArgumentDescs = static_cast<UINT>(INIT_DATA.argument.size());
	desc.ByteStride = sizeof(DrawIndirectCommand);

	HRESULT lResult =
		DirectX12Device::Instance()->dev->CreateCommandSignature(&desc, GraphicsRootSignature::Instance()->GetRootSignature(INIT_DATA.rootsignatureName).Get(), IID_PPV_ARGS(&commandSig));
	//コマンドシグネチャ---------------------------
	if (lResult != S_OK)
	{
		assert(0);
	}


	//Indirect用のバッファ生成
	drawCommandHandle = buffers.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(sizeof(DrawIndirectCommand)));

	DrawIndirectCommand command;
	command.drawArguments.VertexCountPerInstance = INIT_DATA.vertNum;
	command.drawArguments.InstanceCount = INIT_DATA.elementNum;
	command.drawArguments.StartVertexLocation = 0;
	command.drawArguments.StartInstanceLocation = 0;
	command.view = INIT_DATA.updateView;
	buffers.TransData(drawCommandHandle, &command, sizeof(DrawIndirectCommand));
}

void DrawExcuteIndirect::Draw(PipeLineNames PIPELINE_NAME,const Microsoft::WRL::ComPtr<ID3D12Resource> &COUNTER_BUFFER)
{
	GraphicsPipeLineMgr::Instance()->SetPipeLineAndRootSignature(PIPELINE_NAME);
	DirectX12CmdList::Instance()->cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画情報
	switch (drawType)
	{
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
		DirectX12CmdList::Instance()->cmdList->IASetVertexBuffers(0, 1, &initData.vertexBufferView);
		break;
	case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
		DirectX12CmdList::Instance()->cmdList->IASetVertexBuffers(0, 1, &initData.vertexBufferView);
		DirectX12CmdList::Instance()->cmdList->IASetIndexBuffer(&initData.indexBufferView);
		break;
	default:
		break;
	}

	RenderTargetStatus::Instance()->ChangeBarrier(
		buffers.GetBufferData(drawCommandHandle).Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT
	);

	DirectX12CmdList::Instance()->cmdList->ExecuteIndirect
	(
		commandSig.Get(),
		1,
		buffers.GetBufferData(drawCommandHandle).Get(),
		0,
		COUNTER_BUFFER.Get(),
		0
	);

	RenderTargetStatus::Instance()->ChangeBarrier(
		buffers.GetBufferData(drawCommandHandle).Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

}
