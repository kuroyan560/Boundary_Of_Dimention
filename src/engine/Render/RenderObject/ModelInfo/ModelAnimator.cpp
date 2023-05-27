#include "ModelAnimator.h"
#include"KuroEngineDevice.h"
#include"ForUser/Object/Model.h"

void KuroEngine::ModelAnimator::CalculateTransform(Transform& BoneTransform, const KuroEngine::Skeleton::BoneAnimation& BoneAnim, const float& Frame, bool& FinishFlg)
{
	//Translation
	//キーフレーム情報なし
	if (!BoneAnim.posAnim.keyFrames.empty())
	{
		//結果の格納先
		Vec3<float>translation;

		//終了フレームより前なので、アニメーションは終了していない
		if (Frame < BoneAnim.posAnim.endFrame)FinishFlg = false;

		if (Frame < BoneAnim.posAnim.startFrame)translation = BoneAnim.posAnim.keyFrames.front().value;		//範囲外：一番手前を採用
		else if (BoneAnim.posAnim.endFrame < Frame)translation = BoneAnim.posAnim.keyFrames.back().value;	//範囲外：一番最後を採用
		else
		{
			const KeyFrame<Vec3<float>>* firstKey = nullptr;
			const KeyFrame<Vec3<float>>* secondKey = nullptr;
			for (auto& key : BoneAnim.posAnim.keyFrames)
			{
				//同じフレーム数の物があったらそれを採用
				if (key.frame == Frame)
				{
					translation = key.value;
					break;
				}

				if (key.frame < Frame)firstKey = &key;	//補間の開始キーフレーム
				if (secondKey == nullptr && Frame < key.frame)secondKey = &key;	//補間の終了キーフレーム

				//補間の情報が揃ったので線形補間してそれを採用
				if (firstKey != nullptr && secondKey != nullptr)
				{
					translation = KuroEngine::Math::Lerp(firstKey->value, secondKey->value, (Frame - firstKey->frame) / (secondKey->frame - firstKey->frame));
					break;
				}
			}
		}
		//結果をTransformに反映
		BoneTransform.SetPos(translation);
	}

	//Rotation
	//キーフレーム情報なし
	if (!BoneAnim.rotateAnim.keyFrames.empty())
	{
		//結果の格納先
		XMVECTOR rotation{};

		//終了フレームより前なので、アニメーションは終了していない
		if (Frame < BoneAnim.rotateAnim.endFrame)FinishFlg = false;

		if (Frame < BoneAnim.rotateAnim.startFrame)rotation = BoneAnim.rotateAnim.keyFrames.front().value;		//範囲外：一番手前を採用
		else if (BoneAnim.rotateAnim.endFrame < Frame)rotation = BoneAnim.rotateAnim.keyFrames.back().value;	//範囲外：一番最後を採用
		else
		{
			const KeyFrame<XMVECTOR>* firstKey = nullptr;
			const KeyFrame<XMVECTOR>* secondKey = nullptr;
			for (auto& key : BoneAnim.rotateAnim.keyFrames)
			{
				//同じフレーム数の物があったらそれを採用
				if (key.frame == Frame)
				{
					rotation = key.value;
					break;
				}

				if (key.frame < Frame)firstKey = &key;	//補間の開始キーフレーム
				if (secondKey == nullptr && Frame < key.frame)secondKey = &key;	//補間の終了キーフレーム

				//補間の情報が揃ったので線形補間してそれを採用
				if (firstKey != nullptr && secondKey != nullptr)
				{
					float rate = (Frame - firstKey->frame) / (secondKey->frame - firstKey->frame);
					//rotation = XMQuaternionSlerp(firstKey->value, secondKey->value, (Frame - firstKey->frame) / (secondKey->frame - firstKey->frame));
					rotation = XMQuaternionSlerpV(firstKey->value, secondKey->value, XMVectorSet(rate, rate, rate, rate));
					break;
				}
			}
		}
		//結果をTransformに反映
		BoneTransform.SetRotate(rotation);
	}
	//Scaling
	//キーフレーム情報なし
	if (!BoneAnim.scaleAnim.keyFrames.empty())
	{
		//結果の格納先
		Vec3<float>scale;

		//終了フレームより前なので、アニメーションは終了していない
		if (Frame < BoneAnim.scaleAnim.endFrame)FinishFlg = false;

		if (Frame < BoneAnim.scaleAnim.startFrame)scale = BoneAnim.scaleAnim.keyFrames.front().value;		//範囲外：一番手前を採用
		else if (BoneAnim.scaleAnim.endFrame < Frame)scale = BoneAnim.scaleAnim.keyFrames.back().value;	//範囲外：一番最後を採用
		else
		{
			const KeyFrame<Vec3<float>>* firstKey = nullptr;
			const KeyFrame<Vec3<float>>* secondKey = nullptr;
			for (auto& key : BoneAnim.scaleAnim.keyFrames)
			{
				//同じフレーム数の物があったらそれを採用
				if (key.frame == Frame)
				{
					scale = key.value;
					break;
				}

				if (key.frame < Frame)firstKey = &key;	//補間の開始キーフレーム
				if (secondKey == nullptr && Frame < key.frame)secondKey = &key;	//補間の終了キーフレーム

				//補間の情報が揃ったので線形補間してそれを採用
				if (firstKey != nullptr && secondKey != nullptr)
				{
					scale = KuroEngine::Math::Lerp(firstKey->value, secondKey->value, (Frame - firstKey->frame) / (secondKey->frame - firstKey->frame));
					break;
				}
			}
			//結果をTransformに反映
			BoneTransform.SetScale(scale);
		}
	}
}

void KuroEngine::ModelAnimator::BoneMatrixRecursive(const int& BoneIdx, const float& Past, bool* Finish, KuroEngine::Skeleton::ModelAnimation& Anim)
{
	auto skel = attachSkelton.lock();
	const auto& bone = skel->bones[BoneIdx];
	const auto& boneAnim = Anim.boneAnim[bone.name];

	//ボーンアニメーション行列の計算
	bool finish = true;

	CalculateTransform(m_boneTransform[BoneIdx], boneAnim, Past, finish);

	//アニメーションは終了していない
	if (Finish != nullptr && !finish)*Finish = false;

	boneMatricies[BoneIdx] = skel->coordinateSystemConvertMat
		* skel->bones[BoneIdx].invBindMat * m_boneTransform[BoneIdx].GetMatWorld()
		* XMMatrixInverse(nullptr, skel->coordinateSystemConvertMat);

	//子を呼び出して再帰的に計算
	for (auto& child : bone.children)
	{
		BoneMatrixRecursive(child, Past, Finish, Anim);
	}
}

KuroEngine::ModelAnimator::ModelAnimator(std::weak_ptr<Model> Model)
{
	Attach(Model);
}

void KuroEngine::ModelAnimator::Attach(std::weak_ptr<Model> Model)
{
	auto model = Model.lock();
	auto skel = model->m_skelton;
	assert(skel->bones.size() <= MAX_BONE_NUM);

	//バッファ未生成
	if (!boneBuff)
	{
		boneBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), MAX_BONE_NUM);
	}
	
	//バッファのリネーム
	boneBuff->GetResource()->SetName((L"BoneMatricies - " + GetWideStrFromStr(model->m_header.GetFileNameWithoutExt())).c_str());

	Reset();

	//スケルトンをアタッチ
	attachSkelton = skel;

	m_boneTransform.resize(skel->bones.size());
	for (int boneIdx = 0; boneIdx < skel->bones.size(); ++boneIdx)
	{
		//親がいるなら、親ボーンのTransform登録
		if (skel->bones[boneIdx].parent != -1)
		{
			m_boneTransform[boneIdx].SetParent(&m_boneTransform[skel->bones[boneIdx].parent]);
		}
	}

}

void KuroEngine::ModelAnimator::Reset()
{
	//単位行列で埋めてリセット
	std::array<Matrix, MAX_BONE_NUM>initMat;
	initMat.fill(XMMatrixIdentity());
	boneBuff->Mapping(initMat.data());

	//再生中アニメーション名リセット
	playAnimations.clear();
}

void KuroEngine::ModelAnimator::SetStartPosture(const std::string& AnimationName)
{
	auto skel = attachSkelton.lock();
	if (!skel)return;	//スケルトンがアタッチされていない

	//アニメーション情報取得
	auto& anim = skel->animations[AnimationName];

	//ボーン行列を再帰的に計算
	BoneMatrixRecursive(static_cast<int>(skel->bones.size() - 1), 0, nullptr, anim);

	//バッファにデータ転送
	boneBuff->Mapping(boneMatricies.data());
}

void KuroEngine::ModelAnimator::SetEndPosture(const std::string& AnimationName)
{
	auto skel = attachSkelton.lock();
	if (!skel)return;	//スケルトンがアタッチされていない

	//アニメーション情報取得
	auto& anim = skel->animations[AnimationName];

	//ボーン行列を再帰的に計算
	BoneMatrixRecursive(static_cast<int>(skel->bones.size() - 1), static_cast<float>(anim.finishTime), nullptr, anim);

	//バッファにデータ転送
	boneBuff->Mapping(boneMatricies.data());
}

void KuroEngine::ModelAnimator::Play(const std::string& AnimationName, const bool& Loop, const bool& Blend, float InitPast)
{
	if (!Blend)Reset();

	auto skel = attachSkelton.lock();
	assert(skel);
	assert(skel->animations.contains(AnimationName));

	//既に再生中か調べる
	auto already = std::find_if(playAnimations.begin(), playAnimations.end(), [AnimationName](PlayAnimation& Anim)
		{ return !Anim.name.compare(AnimationName); } );
	//再生中ならリセットしておわり
	if (already != playAnimations.end())
	{
		already->past = static_cast<float>(InitPast);
		already->loop = Loop;
		return;
	}

	//新規アニメーション追加
	playAnimations.emplace_back(AnimationName, Loop);
}

void KuroEngine::ModelAnimator::Update(const float& arg_timeScale)
{
	auto skel = attachSkelton.lock();
	if (!skel)return;	//スケルトンがアタッチされていない
	if (playAnimations.empty())return;	//アニメーション再生中でない

	//単位行列で埋めてリセット
	std::fill(boneMatricies.begin(), boneMatricies.end(), XMMatrixIdentity());

	//再生中のアニメーション
	for (auto& playAnim : playAnimations)
	{
		//アニメーション情報取得
		auto& anim = skel->animations[playAnim.name];

		//アニメーションが終了しているかのフラグ
		bool animFinish = true;

		//ボーン行列を再帰的に計算
		BoneMatrixRecursive(static_cast<int>(skel->bones.size() - 1), playAnim.past, &animFinish, anim);

		//フレーム経過
		playAnim.past += arg_timeScale;
		//アニメーションの終了情報記録
		playAnim.finish = animFinish;
	}

	//終了しているアニメーションを調べる
	for (auto itr = playAnimations.begin(); itr != playAnimations.end();)
	{
		//アニメーション終了していないならスルー
		if (!itr->finish) { ++itr; continue; }
		//ループフラグが立っているなら経過フレームをリセットしてスルー
		if (itr->loop) { itr->past = 0; ++itr; continue; }
		//終了しているので削除
		itr = playAnimations.erase(itr);
	}

	//バッファにデータ転送
	boneBuff->Mapping(boneMatricies.data());
}

void KuroEngine::ModelAnimator::SetParentTransform(Transform& arg_parent)
{
	for (auto& boneTransform : m_boneTransform)
	{
		boneTransform.SetParent(&arg_parent);
	}
}

KuroEngine::Transform& KuroEngine::ModelAnimator::GetBoneTransform(const std::string& BoneName)
{
	assert(attachSkelton.lock()->boneIdxTable.contains(BoneName));
	return m_boneTransform[attachSkelton.lock()->boneIdxTable[BoneName]];
}