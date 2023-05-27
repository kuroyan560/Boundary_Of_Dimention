#pragma once
#include<memory>
#include<string>
#include<list>
#include<array>
#include"Render/RenderObject/ModelInfo/Skeleton.h"
#include<vector>
#include"Common/Transform.h"
#include"ForUser/Timer.h"

namespace KuroEngine
{
	class Model;
	class ConstantBuffer;
	class TimeScale;

	class ModelAnimator
	{
		static const int MAX_BONE_NUM = 256;

		//対応するスケルトンの参照
		std::weak_ptr<Skeleton>attachSkelton;
		//ボーンのローカル行列
		std::shared_ptr<ConstantBuffer>boneBuff;
		//ボーン行列（GPU送信用）
		std::array<Matrix, MAX_BONE_NUM>boneMatricies;
		//各ボーンのTransform
		std::vector<Transform>m_boneTransform;

		struct PlayAnimation
		{
			std::string name;	//アニメーション名
			float past = 0.0f;
			bool loop = false;
			bool finish = false;

			PlayAnimation(const std::string& Name, const bool& Loop) :name(Name), loop(Loop) {}
		};

		std::list<PlayAnimation>playAnimations;

		//ボーンTransform一括計算
		void CalculateTransform(Transform& BoneTransform, const KuroEngine::Skeleton::BoneAnimation& BoneAnim, const float& Frame, bool& FinishFlg);
		void BoneMatrixRecursive(const int& BoneIdx, const float& Past, bool* Finish, KuroEngine::Skeleton::ModelAnimation& Anim);

	public:
		ModelAnimator() {}
		ModelAnimator(std::weak_ptr<Model>Model);
		void Attach(std::weak_ptr<Model>Model);

		//単位行列で埋める
		void Reset();

		//指定したアニメーションの始まりの姿勢をセット
		void SetStartPosture(const std::string& AnimationName);

		//指定したアニメーションの終わりの姿勢をセット
		void SetEndPosture(const std::string& AnimationName);

		//アニメーション再生
		void Play(const std::string& AnimationName, const bool& Loop, const bool& Blend, float InitPast = 0.0f);
		//何かしらのアニメーションが現在再生中か
		bool IsPlay()const
		{
			return !playAnimations.empty();
		}

		//指定のアニメーションが現在再生中か
		bool IsPlay(const std::string& AnimationName)
		{
			auto result = std::find_if(playAnimations.begin(), playAnimations.end(), [AnimationName](PlayAnimation& Anim)
				{
					return AnimationName.compare(Anim.name) == 0;
				});
			return result != playAnimations.end();
		}
		//アニメーション更新
		void Update(const float& arg_timeScale);

		//ボーントランスフォームに親設定
		void SetParentTransform(Transform& arg_parent);

		const std::shared_ptr<ConstantBuffer>& GetBoneMatBuff() { return boneBuff; }
		Transform& GetBoneTransform(const std::string& BoneName);
	};
}