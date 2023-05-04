#pragma once
#include"../StageParts.h"

class MiniBug :public StageParts
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent)
		:StageParts(MINI_BUG, arg_model, arg_initTransform, arg_parent) {}
	void Update(Player& arg_player)override;
};

class DossunRing : public StageParts
{
public:
	DossunRing(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent)
		:StageParts(DOSSUN_RING, arg_model, arg_initTransform, arg_parent) {}
	void Update(Player& arg_player)override;
};