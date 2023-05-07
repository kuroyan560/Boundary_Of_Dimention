#pragma once
#include"KuroEngine.h"
#include<string>
#include<map>
#include"Render/RenderObject/Animation.h"
#include<array>

namespace KuroEngine
{
	class Bone
	{
	public:
		static size_t GetSizeWithOutName()
		{
			return sizeof(char) + sizeof(int) + sizeof(Vec3<float>) + sizeof(Matrix);
		}

		std::string name;
		char parent = -1;	//親ボーン
		std::vector<char>children;	//子ボーン
		int transLayer = 0;	//変形階層
		Vec3<float> pos = { 0.0f,0.0f,0.0f };
		Matrix invBindMat = DirectX::XMMatrixIdentity();
	};

	class Skeleton
	{
	public:
		static const std::string DEFINE_PARENT_BONE_NAME;
		struct BoneAnimation
		{
		public:
			Animation<Vec3<float>>posAnim;
			Animation<XMVECTOR>rotateAnim;
			Animation<Vec3<float>>scaleAnim;
		};
		struct ModelAnimation
		{
			//全てのボーンのアニメーションが終了する時間
			int finishTime;
			//ボーン単位のアニメーション
			std::map<std::string, BoneAnimation>boneAnim;
		};

		//座標系変換を修正するための行列
		Matrix coordinateSystemConvertMat = XMMatrixIdentity();

		std::vector<Bone>bones;
		std::map<std::string, int>boneIdxTable;
		/*
			アニメーション情報（Skeletonがアニメーションを行う訳では無い。Animatorからの参照用）
			キーは アニメーション名
		*/
		std::map<std::string, ModelAnimation>animations;

		void CreateBoneTree(const Matrix CoordinateSysConvertMat);
		int GetIndex(const std::string& BoneName);
	};
}