#pragma once
#include"FrameWork/WinApp.h"
#include"DirectX12/D3D12App.h"
#include<array>

namespace KazGPUParticle
{
	typedef int RESOURCE_HANDLE;
	typedef int BUFFER_SIZE;


	class ID3D12ResourceWrapper
	{
	public:
		ID3D12ResourceWrapper();

		/// <summary>
		/// �o�b�t�@����
		/// </summary>
		/// <param name="BUFFER_OPTION">�o�b�t�@�𐶐�����ׂɕK�v�ȍ\����</param>
		void CreateBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION);

		/// <summary>
		/// �f�[�^���o�b�t�@�ɓ]�����܂�
		/// </summary>
		/// <param name="DATA">���肽���f�[�^�̃A�h���X</param>
		/// <param name="DATA_SIZE">���肽���f�[�^�̃T�C�Y</param>
		void TransData(void *DATA, const unsigned int &DATA_SIZE);
		void TransData(void *DATA, unsigned int START_DATA_SIZE, unsigned int DATA_SIZE);

		/// <summary>
		/// �o�b�t�@���J�����܂�
		/// </summary>
		/// <param name="HANDLE">�J���������o�b�t�@�̃n���h��</param>
		void Release();

		/// <summary>
		/// �o�b�t�@��GPU�A�h���X���󂯎��܂�
		/// </summary>
		/// <param name="HANDLE">�n���h��</param>
		/// <returns>�o�b�t�@��GPU�A�h���X</returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress();

		void *GetMapAddres(int BB_INDEX = -1)const;

		void CopyBuffer(
			const Microsoft::WRL::ComPtr<ID3D12Resource> &SRC_BUFFER,
			D3D12_RESOURCE_STATES BEFORE_STATE,
			D3D12_RESOURCE_STATES AFTER_STATE
		)const
		{
			for (int i = 0; i < buffer.size(); ++i)
			{
				KuroEngine::D3D12App::Instance()->GetCmdList()->ResourceBarrier(
					1,
					&CD3DX12_RESOURCE_BARRIER::Transition(buffer[i].Get(),
						BEFORE_STATE,
						AFTER_STATE
					)
				);

				KuroEngine::D3D12App::Instance()->GetCmdList()->CopyResource(buffer[i].Get(), SRC_BUFFER.Get());

				KuroEngine::D3D12App::Instance()->GetCmdList()->ResourceBarrier(
					1,
					&CD3DX12_RESOURCE_BARRIER::Transition(buffer[i].Get(),
						AFTER_STATE,
						BEFORE_STATE
					)
				);
			}
		}

		const Microsoft::WRL::ComPtr<ID3D12Resource> &GetBuffer(int INDEX = -1) const
		{
			if (INDEX == -1)
			{
				return buffer[GetIndex()];
			}
			else
			{
				return buffer[INDEX];
			}
		}

		void operator=(const ID3D12ResourceWrapper &rhs)
		{
			for (int i = 0; i < buffer.size(); ++i)
			{
				rhs.buffer[i].CopyTo(&buffer[i]);
			}
		};

	private:
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 1>buffer;
		UINT GetIndex()const
		{
			return 0;
		}
	};

}


namespace KazBufferHelper
{
	struct BufferResourceData
	{
		D3D12_HEAP_PROPERTIES heapProperties;
		D3D12_HEAP_FLAGS heapFlags;
		D3D12_RESOURCE_DESC resourceDesc;
		D3D12_RESOURCE_STATES resourceState;
		D3D12_CLEAR_VALUE *clearValue;
		std::string BufferName;

		BufferResourceData
		(
			const D3D12_HEAP_PROPERTIES &HEAP_PROP,
			const D3D12_HEAP_FLAGS &HEAP_FLAGS,
			const D3D12_RESOURCE_DESC &RESOURCE_DESC,
			const D3D12_RESOURCE_STATES &RESOURCE_STATE,
			D3D12_CLEAR_VALUE *CLEAR_VALUE,
			const std::string &BUFFER_NAME
		)
			:heapProperties(HEAP_PROP),
			heapFlags(HEAP_FLAGS),
			resourceDesc(RESOURCE_DESC),
			resourceState(RESOURCE_STATE),
			clearValue(CLEAR_VALUE),
			BufferName(BUFFER_NAME)
		{
		};
	};


	/// <summary>
	/// �萔�o�b�t�@�𐶐�����ۂɕK�v�Ȑݒ���ȈՂɓZ�߂���
	/// </summary>
	/// <param name="KazGPUParticle::BUFFER_SIZE">�萔�o�b�t�@�̃T�C�Y</param>
	/// <returns>�萔�o�b�t�@�̐����ɕK�v�Ȑݒ�</returns>
	KazBufferHelper::BufferResourceData SetConstBufferData(const unsigned int &BUFFER_SIZE, const std::string &BUFFER_NAME = "ConstBuffer");

	/// <summary>
	/// �V�F�[�_�[���\�[�X�o�b�t�@�𐶐�����ۂɕK�v�Ȑݒ���ȈՂɓZ�߂���
	/// <param name="TEXTURE_DATA">�ǂݍ��񂾃e�N�X�`���̐ݒ�</param>
	/// <returns>�V�F�[�_�[���\�[�X�o�b�t�@�̐����ɕK�v�Ȑݒ�</returns>
	KazBufferHelper::BufferResourceData SetShaderResourceBufferData(const D3D12_RESOURCE_DESC &TEXTURE_DATA, const std::string &BUFFER_NAME = "ShaderResourceBuffer");


	KazBufferHelper::BufferResourceData SetVertexBufferData(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "VertexBuffer");


	KazBufferHelper::BufferResourceData SetIndexBufferData(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "IndexBuffer");


	KazBufferHelper::BufferResourceData SetRenderTargetData(const D3D12_HEAP_PROPERTIES &HEAP_PROPERTIES, const D3D12_RESOURCE_DESC &RESOURCE, D3D12_CLEAR_VALUE *CLEAR_COLOR, const std::string &BUFFER_NAME = "RenderTarget");

	KazBufferHelper::BufferResourceData SetStructureBuffer(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "StructureBuffer");

	KazBufferHelper::BufferResourceData SetRWStructuredBuffer(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "RWStructureBuffer");

	KazBufferHelper::BufferResourceData SetOnlyReadStructuredBuffer(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "OnlyReadStructureBuffer");


	KazBufferHelper::BufferResourceData SetCommandBufferData(const unsigned int &BUFFER_SIZE, const std::string &BUFFER_NAME = "CommandBuffer");

	D3D12_VERTEX_BUFFER_VIEW SetVertexBufferView(const D3D12_GPU_VIRTUAL_ADDRESS &GPU_ADDRESS, KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const unsigned int &ONE_VERTICES_SIZE);

	D3D12_INDEX_BUFFER_VIEW SetIndexBufferView(const D3D12_GPU_VIRTUAL_ADDRESS &GPU_ADDRESS, KazGPUParticle::BUFFER_SIZE BUFFER_SIZE);

	D3D12_UNORDERED_ACCESS_VIEW_DESC SetUnorderedAccessView(KazGPUParticle::BUFFER_SIZE STRUCTURE_BYTE_SIZE, UINT NUM_ELEMENTS);

	KazBufferHelper::BufferResourceData SetGPUBufferData(KazGPUParticle::BUFFER_SIZE BUFFER_SIZE, const std::string &BUFFER_NAME = "IndexBuffer");


	template<typename T>
	T GetBufferSize(size_t BUFFER_SIZE, unsigned long long STRUCTURE_SIZE)
	{
		return static_cast<T>(BUFFER_SIZE * static_cast<int>(STRUCTURE_SIZE));
	};
}