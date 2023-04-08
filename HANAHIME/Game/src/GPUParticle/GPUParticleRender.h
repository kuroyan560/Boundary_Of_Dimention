#pragma once
#include"../KazLibrary/Render/DrawExcuteIndirect.h"
#include"../KazLibrary/Helper/KazBufferHelper.h"
#include"../KazLibrary/Helper/KazRenderHelper.h"
#include"../KazLibrary/Helper/ResouceBufferHelper.h"
#include"../KazLibrary/Helper/ISinglton.h"

//���o�Ŏg���Ă���S�Ẵp�[�e�B�N���̏����X�^�b�N���Ĉ�C�ɕ`�悷��N���X�ł�
class GPUParticleRender
{
public:
	GPUParticleRender(int MAXNUM = 3000000);

	void InitCount();
	void Draw(const DirectX::XMUINT3 &NUM = { 3000,1,1 });

	const ResouceBufferHelper::BufferData &GetStackBuffer()const;

	struct InputData
	{
		DirectX::XMMATRIX worldMat;
		DirectX::XMFLOAT4 color;
	};
private:

	
	struct OutputData
	{
		DirectX::XMMATRIX mat;
		DirectX::XMFLOAT4 color;
	};


	int particleMaxNum = 3000000;
	ResouceBufferHelper computeCovertWorldMatToDrawMat;
	RESOURCE_HANDLE worldMatHandle, outputHandle, viewProjMatHandle;

	DirectX::XMMATRIX viewProjMat;

	std::unique_ptr<DrawExcuteIndirect> excuteIndirect;
	RESOURCE_HANDLE vertexBufferHandle;
	RESOURCE_HANDLE indexBufferHandle;
	std::unique_ptr<KazRenderHelper::ID3D12ResourceWrapper> vertexBuffer, indexBuffer;
	KazRenderHelper::ID3D12ResourceWrapper gpuVertexBuffer, gpuIndexBuffer;



	KazRenderHelper::ID3D12ResourceWrapper copyBuffer;
};

