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
	//ハンドルの番号決定
	if (1 <= deleteHandleNumber.size())
	{
		//最初に解放された場所に優先的に確保する
		handle = deleteHandleNumber[0];
	}
	else
	{
		handle = setHandle;
		setHandle++;
	}


	//配列の末尾から確保していくので末尾から消していく
	if (deleteHandleNumber.size() != 0)
	{
		deleteHandleNumber.erase(deleteHandleNumber.begin());
	}

	//ハンドルが最大サイズを超えていたらエラー値を返す
	if (maxSize <= handle)
	{
		return -1;
	}

	return handle;
}

void HandleMaker::DeleteHandle(RESOURCE_HANDLE HANDLE)
{
	//ハンドルが生成された最大値の内&&削除されていない値
	if (KazHelper::IsitInAnArray(HANDLE, setHandle) && !IsItDeleted(HANDLE))
	{
		deleteHandleNumber.push_back(HANDLE);
	}
	else
	{
		ErrorCheck("危険:そのハンドルは既に削除済みか、元々無いハンドルです");
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
