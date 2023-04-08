#include "HandleMaker.h"
#include"../Helper/KazHelper.h"

HandleMaker::HandleMaker()
{
	handle = 0;
	setHandle = 0;
	maxSize = 10000;
}

int HandleMaker::GetHandle()
{
	//�n���h���̔ԍ�����
	if (1 <= deleteHandleNumber.size())
	{
		//�ŏ��ɉ�����ꂽ�ꏊ�ɗD��I�Ɋm�ۂ���
		handle = deleteHandleNumber[0];
	}
	else
	{
		handle = setHandle;
		setHandle++;
	}


	//�z��̖�������m�ۂ��Ă����̂Ŗ�����������Ă���
	if (deleteHandleNumber.size() != 0)
	{
		deleteHandleNumber.erase(deleteHandleNumber.begin());
	}

	//�n���h�����ő�T�C�Y�𒴂��Ă�����G���[�l��Ԃ�
	if (maxSize <= handle)
	{
		return -1;
	}

	return handle;
}

void HandleMaker::DeleteHandle(RESOURCE_HANDLE HANDLE)
{
	//�n���h�����������ꂽ�ő�l�̓�&&�폜����Ă��Ȃ��l
	if (KazHelper::IsitInAnArray(HANDLE, setHandle) && !IsItDeleted(HANDLE))
	{
		deleteHandleNumber.push_back(HANDLE);
	}
	else
	{
		ErrorCheck("�댯:���̃n���h���͊��ɍ폜�ς݂��A���X�����n���h���ł�");
	}
}

void HandleMaker::DeleteAllHandle()
{
	setHandle = 0;
	handle = 0;
	deleteHandleNumber.clear();
	deleteHandleNumber.shrink_to_fit();
}

bool HandleMaker::CheckHandleWasDeleteOrNot(RESOURCE_HANDLE HANDLE)
{
	for (int i = 0; i < deleteHandleNumber.size(); i++)
	{
		if (deleteHandleNumber[i] == HANDLE)
		{
			return true;
		}
	}
	return false;
}

bool HandleMaker::CheckHandleWasUsedOrNot(RESOURCE_HANDLE HANDLE)
{
	if (HANDLE < handle)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void HandleMaker::SetHandleSize(const BufferMemorySize &SIZE)
{
	minSize = SIZE.startSize;
	maxSize = SIZE.endSize;

	setHandle = minSize;
}

RESOURCE_HANDLE HandleMaker::CaluNowHandle(RESOURCE_HANDLE HANDLE)
{
	return HANDLE - minSize;
}

bool HandleMaker::IsItDeleted(RESOURCE_HANDLE HANDLE)
{
	for (int i = 0; i < deleteHandleNumber.size(); i++)
	{
		if (deleteHandleNumber[i] == HANDLE)
		{
			return true;
		}
	}
	return false;
}
