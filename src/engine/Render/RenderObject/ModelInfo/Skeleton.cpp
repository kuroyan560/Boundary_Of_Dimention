#include "Skeleton.h"
#include"KuroEngine.h"

const std::string KuroEngine::Skeleton::DEFINE_PARENT_BONE_NAME = "Defined_Parent";

void KuroEngine::Skeleton::CreateBoneTree(const Matrix CoordinateSysConvertMat)
{
	//ボーンがないなら無視
	if (bones.empty())return;

	//全ての親となるボーンを追加
	int parentBoneIdx = static_cast<int>(bones.size());
	bones.emplace_back();
	bones.back().name = "Defined_Parent";

	//ボーンノードマップを作る
	for (int idx = 0; idx < bones.size(); ++idx)
	{
		auto& bone = bones[idx];
		boneIdxTable[bone.name] = idx;

		if (bone.parent != -1)continue;	//既に親がいる
		if (idx == parentBoneIdx)continue;	//自信が全ての親
		bone.parent = parentBoneIdx;
	}

	//親子関係構築
	for (int i = 0; i < bones.size(); ++i)
	{
		auto& bone = bones[i];
		//親インデックスをチェック(ありえない番号ならとばす)
		if (bone.parent < 0 || bones.size() < bone.parent)
		{
			continue;
		}
		bones[bone.parent].children.emplace_back(i);
	}

	coordinateSystemConvertMat = CoordinateSysConvertMat;
}

int KuroEngine::Skeleton::GetIndex(const std::string& BoneName)
{
	assert(!bones.empty());
	assert(boneIdxTable.contains(BoneName));
	return boneIdxTable[BoneName];
}