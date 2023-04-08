#pragma once
#include"../DirectXCommon/Base.h"
#include"../Helper/KazBufferHelper.h"


/// <summary>
/// 使用していないハンドルを返し、指定のハンドルを削除できるクラス
/// </summary>
class HandleMaker
{
public:
	HandleMaker();

	/// <summary>
	///返り値でハンドルを返してくれます
	/// </summary>
	/// <returns>使用していないハンドル</returns>
	RESOURCE_HANDLE GetHandle();

	/// <summary>
	/// 指定のハンドルを削除します
	/// </summary>
	/// <param name="HANDLE">指定のハンドル</param>
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
	/// ハンドルが過去に削除されていたかどうか検索します
	/// </summary>
	/// <param name="HANDLE">ハンドル</param>
	/// <returns>true...削除されていました,false...削除されてませんでした</returns>
	bool IsItDeleted(RESOURCE_HANDLE HANDLE);
};