#pragma once
#include"../DirectXCommon/Base.h"
#include"../Helper/KazBufferHelper.h"


/// <summary>
/// �g�p���Ă��Ȃ��n���h����Ԃ��A�w��̃n���h�����폜�ł���N���X
/// </summary>
class HandleMaker
{
public:
	HandleMaker();

	/// <summary>
	///�Ԃ�l�Ńn���h����Ԃ��Ă���܂�
	/// </summary>
	/// <returns>�g�p���Ă��Ȃ��n���h��</returns>
	RESOURCE_HANDLE GetHandle();

	/// <summary>
	/// �w��̃n���h�����폜���܂�
	/// </summary>
	/// <param name="HANDLE">�w��̃n���h��</param>
	void DeleteHandle(RESOURCE_HANDLE HANDLE);
	void DeleteAllHandle();


	bool CheckHandleWasDeleteOrNot(RESOURCE_HANDLE HANDLE);
	bool CheckHandleWasUsedOrNot(RESOURCE_HANDLE HANDLE);

	void SetHandleSize(const BufferMemorySize &SIZE);


	RESOURCE_HANDLE CaluNowHandle(RESOURCE_HANDLE HANDLE);

	int minSize, maxSize;
private:
	RESOURCE_HANDLE handle;
	RESOURCE_HANDLE setHandle;
	std::vector<RESOURCE_HANDLE> deleteHandleNumber;

	/// <summary>
	/// �n���h�����ߋ��ɍ폜����Ă������ǂ����������܂�
	/// </summary>
	/// <param name="HANDLE">�n���h��</param>
	/// <returns>true...�폜����Ă��܂���,false...�폜����Ă܂���ł���</returns>
	bool IsItDeleted(RESOURCE_HANDLE HANDLE);
};