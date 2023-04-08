#include "ResouceBufferHelper.h"
#include"../KazLibrary/Buffer/DescriptorHeapMgr.h"
#include"../KazLibrary/Buffer/UavViewHandleMgr.h"
#include"../KazLibrary/RenderTarget/RenderTargetStatus.h"

const int ResouceBufferHelper::SWAPCHAIN_NUM = 1;

ResouceBufferHelper::ResouceBufferHelper() :counterBufferData(
	CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	D3D12_HEAP_FLAG_NONE,
	CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)),
	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	nullptr,
	"CounterBuffer"
)
{
	counterBufferData.resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	//counterBufferData = KazBufferHelper::SetRWStructuredBuffer(sizeof(UINT), "CounterBuffer");
}

RESOURCE_HANDLE ResouceBufferHelper::CreateBuffer(UINT STRUCTURE_BYTE_STRIDE, GraphicsRangeType RANGE, GraphicsRootParamType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
{
	RESOURCE_HANDLE lHandle = handle.GetHandle();
	RESOURCE_HANDLE lViewHandle = 0;
	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;

	bufferArrayData.emplace_back(ResouceBufferHelper::BufferData());
	bufferArrayData[lHandle].rangeType = RANGE;
	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
	bufferArrayData[lHandle].bufferSize = lBufferSize;
	bufferArrayData[lHandle].elementNum = ELEMENT_NUM;

	switch (RANGE)
	{
	case GRAPHICS_RANGE_TYPE_CBV_VIEW:
		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_VIEW:
		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_DESC:
		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));


		std::vector<RESOURCE_HANDLE>lViewHandleArray;
		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
		{
			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
			bufferArrayData[lHandle].CreateViewHandle(lViewHandleArray[i]);

			if (GENERATE_COUNTER_BUFFER_FLAG)
			{
				bufferArrayData[lHandle].counterWrapper.CreateBuffer(counterBufferData);

				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
					bufferArrayData[lHandle].counterWrapper.GetBuffer(i).Get()
				);
			}
			else
			{
				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
					nullptr
				);
			}
		}
		break;
	}

	return lHandle;
}

RESOURCE_HANDLE ResouceBufferHelper::CreateBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA, GraphicsRangeType RANGE, GraphicsRootParamType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
{
	RESOURCE_HANDLE lHandle = handle.GetHandle();

	bufferArrayData.push_back(ResouceBufferHelper::BufferData());
	bufferArrayData[lHandle].rangeType = RANGE;
	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
	bufferArrayData[lHandle].bufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;
	bufferArrayData[lHandle].bufferWrapper.CreateBuffer(BUFFER_OPTION_DATA);

	switch (RANGE)
	{
	case GRAPHICS_RANGE_TYPE_UAV_DESC:
		std::vector<RESOURCE_HANDLE>lViewHandleArray;
		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
		{
			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
			bufferArrayData[lHandle].CreateViewHandle(lViewHandleArray[i]);

			if (GENERATE_COUNTER_BUFFER_FLAG)
			{
				bufferArrayData[lHandle].counterWrapper.CreateBuffer(counterBufferData);

				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
					bufferArrayData[lHandle].counterWrapper.GetBuffer(i).Get()
				);
			}
			else
			{
				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
					nullptr
				);
			}
		}
		break;
	}

	return lHandle;
}

ResouceBufferHelper::BufferData ResouceBufferHelper::CreateAndGetBuffer(UINT STRUCTURE_BYTE_STRIDE, GraphicsRangeType RANGE, GraphicsRootParamType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
{
	ResouceBufferHelper::BufferData lBufferData;

	RESOURCE_HANDLE lViewHandle = -1;
	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;

	lBufferData.rangeType = RANGE;
	lBufferData.rootParamType = ROOTPARAM;
	lBufferData.bufferSize = lBufferSize;
	lBufferData.elementNum = ELEMENT_NUM;

	switch (RANGE)
	{
	case GRAPHICS_RANGE_TYPE_CBV_VIEW:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_VIEW:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_DESC:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));

		std::vector<RESOURCE_HANDLE>lViewHandleArray;
		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
		{
			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
			lBufferData.CreateViewHandle(lViewHandleArray[i]);

			if (GENERATE_COUNTER_BUFFER_FLAG)
			{
				lBufferData.counterWrapper.CreateBuffer(counterBufferData);

				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					lBufferData.bufferWrapper.GetBuffer(i).Get(),
					lBufferData.counterWrapper.GetBuffer(i).Get()
				);
			}
			else
			{
				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					lBufferData.bufferWrapper.GetBuffer(i).Get(),
					nullptr
				);
			}
		}

		break;
	}

	return lBufferData;
}

ResouceBufferHelper::BufferData ResouceBufferHelper::CreateAndGetBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA, GraphicsRangeType RANGE, GraphicsRootParamType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
{
	RESOURCE_HANDLE lViewHandle = 0;
	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;

	ResouceBufferHelper::BufferData lBufferData;
	lBufferData.rangeType = RANGE;
	lBufferData.rootParamType = ROOTPARAM;
	lBufferData.bufferSize = lBufferSize;
	lBufferData.elementNum = ELEMENT_NUM;

	switch (RANGE)
	{
	case GRAPHICS_RANGE_TYPE_CBV_VIEW:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_VIEW:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
		break;
	case GRAPHICS_RANGE_TYPE_UAV_DESC:
		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));

		std::vector<RESOURCE_HANDLE>lViewHandleArray;
		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
		{
			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
			lBufferData.CreateViewHandle(lViewHandleArray[i]);

			if (GENERATE_COUNTER_BUFFER_FLAG)
			{
				lBufferData.counterWrapper.CreateBuffer(counterBufferData);

				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					lBufferData.bufferWrapper.GetBuffer(i).Get(),
					lBufferData.counterWrapper.GetBuffer(i).Get()
				);
			}
			else
			{
				DescriptorHeapMgr::Instance()->CreateBufferView(
					lViewHandleArray[i],
					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
					lBufferData.bufferWrapper.GetBuffer(i).Get(),
					nullptr
				);
			}
		}
		break;
	}

	return lBufferData;
}

RESOURCE_HANDLE ResouceBufferHelper::SetBuffer(const ResouceBufferHelper::BufferData &DATA, GraphicsRootParamType ROOTPARAM)
{
	RESOURCE_HANDLE lHandle = handle.GetHandle();
	if (lHandle <= bufferArrayData.size())
	{
		bufferArrayData.push_back(DATA);
	}
	else
	{
		bufferArrayData[lHandle] = DATA;
	}
	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
	return lHandle;
}

void ResouceBufferHelper::TransData(RESOURCE_HANDLE HANDLE, void *TRANS_DATA, UINT TRANSMISSION_DATA_SIZE)
{
	bufferArrayData[HANDLE].bufferWrapper.TransData(TRANS_DATA, TRANSMISSION_DATA_SIZE);
}

void ResouceBufferHelper::StackToCommandListAndCallDispatch(ComputePipeLineNames NAME, const DispatchCallData &DISPATCH_DATA, UINT ADJ_NUM)
{
	GraphicsPipeLineMgr::Instance()->SetComputePipeLineAndRootSignature(NAME);
	std::vector<RootSignatureParameter>lParamData = GraphicsRootSignature::Instance()->GetRootParam(static_cast<int>(NAME + ADJ_NUM));

	for (int i = 0; i < bufferArrayData.size(); ++i)
	{
		const int L_PARAM = KazRenderHelper::SetBufferOnCmdList(lParamData, bufferArrayData[i].rangeType, bufferArrayData[i].rootParamType);

		//デスクリプタヒープにコマンドリストに積む。余りが偶数ならデスクリプタヒープだと判断する
		if (bufferArrayData[i].rangeType % 2 == 0)
		{
			DirectX12CmdList::Instance()->cmdList->SetComputeRootDescriptorTable(L_PARAM, DescriptorHeapMgr::Instance()->GetGpuDescriptorView(bufferArrayData[i].GetViewHandle()));
			continue;
		}

		//ビューで積む際はそれぞれの種類に合わせてコマンドリストに積む
		switch (bufferArrayData[i].rangeType)
		{
		case GRAPHICS_RANGE_TYPE_SRV_VIEW:
			DirectX12CmdList::Instance()->cmdList->SetComputeRootShaderResourceView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
			break;
		case GRAPHICS_RANGE_TYPE_UAV_VIEW:
			DirectX12CmdList::Instance()->cmdList->SetComputeRootUnorderedAccessView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
			break;
		case GRAPHICS_RANGE_TYPE_CBV_VIEW:
			DirectX12CmdList::Instance()->cmdList->SetComputeRootConstantBufferView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
			break;
		default:
			break;
		}
	}

	DirectX12CmdList::Instance()->cmdList->Dispatch(DISPATCH_DATA.x, DISPATCH_DATA.y, DISPATCH_DATA.z);
}

void ResouceBufferHelper::StackToCommandList(PipeLineNames NAME)
{
}

void ResouceBufferHelper::DeleteAllData()
{
	handle.DeleteAllHandle();
	bufferArrayData.clear();
	bufferArrayData.shrink_to_fit();
}

void ResouceBufferHelper::InitCounterBuffer(const Microsoft::WRL::ComPtr<ID3D12Resource> &INIT_DATA)
{
	for (int i = 0; i < bufferArrayData.size(); ++i)
	{
		if (bufferArrayData[i].counterWrapper.GetBuffer())
		{
			DirectX12CmdList::Instance()->cmdList->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(bufferArrayData[i].counterWrapper.GetBuffer().Get(),
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_COPY_DEST
				)
			);

			DirectX12CmdList::Instance()->cmdList->CopyResource(bufferArrayData[i].counterWrapper.GetBuffer().Get(), INIT_DATA.Get());

			DirectX12CmdList::Instance()->cmdList->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(bufferArrayData[i].counterWrapper.GetBuffer().Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				)
			);
		}
	}
}

const ResouceBufferHelper::BufferData &ResouceBufferHelper::GetBufferData(RESOURCE_HANDLE HANDLE)const
{
	return bufferArrayData[HANDLE];
}

void ResouceBufferHelper::SetRootParam(RESOURCE_HANDLE HANDLE, GraphicsRootParamType ROOT_PARAM)
{
	bufferArrayData[HANDLE].rootParamType = ROOT_PARAM;
}