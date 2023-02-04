#include "CollisionManager.h"
#include"CollisionCallBack.h"
#include"Collider.h"
#include"ForUser/Collision/Collision.h"
#include<stdexcept>

void KuroEngine::CollisionManager::OnHit(const std::shared_ptr<Collider>& arg_myCollider, const std::shared_ptr<Collider>& arg_otherCollider, const CollisionResultInfo& arg_info)
{
	arg_myCollider->m_isHit = true;

	//コールバック関数呼び出し
	std::map<std::string,std::vector<CollisionCallBack*>>::iterator callBackList;

	//タグを走査
	for (auto& tag : arg_otherCollider->m_tags)
	{
		//そのタグが対象のコールバックリストを探す
		if ((callBackList = arg_myCollider->m_callBackList.find(tag)) != arg_myCollider->m_callBackList.end())
		{
			//コールバックを実行
			for (auto& callBack : callBackList->second)
			{
				if (!callBack)continue;

				callBack->OnCollisionEnter(arg_info, arg_myCollider, arg_otherCollider);
				if (!arg_myCollider->m_oldIsHit)callBack->OnCollisionTrigger(arg_info, arg_myCollider, arg_otherCollider);
			}
		}
	}
}

void KuroEngine::CollisionManager::OnImguiItems()
{
	static ImVec4 GRAY = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	static ImVec4 BLACK = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	static ImVec4 RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	int num = 0;
	for (auto& c : m_colliderList)
	{
		ImVec4 textCol = c->m_isActive ? BLACK : GRAY;
		if (c->m_isHit)
		{
			textCol = RED;
		}
		ImGui::TextColored(textCol, "%d - %s : { ", num++, c->m_name.c_str());
		for (auto& tag : c->m_tags)
		{
			ImGui::SameLine();
			ImGui::TextColored(textCol, "%s", tag.c_str());
		}
		ImGui::SameLine();
		ImGui::TextColored(textCol, " }");
	}
}

void KuroEngine::CollisionManager::Update()
{
	//既に寿命切れのコライダーを削除
	m_colliderList.remove_if([](std::shared_ptr<Collider> col)
	{
		return !col;
	});

	//当たり判定記録リセット
	for (auto& c : m_colliderList)
	{
		//直前のフラグ記録
		c->m_oldIsHit = c->m_isHit;
		c->m_isHit = false;
	}

	//総当り衝突判定
	for (auto itrA = m_colliderList.begin(); itrA != m_colliderList.end(); ++itrA)
	{
		auto colA = (*itrA);

		auto itrB = itrA;
		++itrB;
		for (; itrB != m_colliderList.end(); ++itrB)
		{
			auto colB = (*itrB);

			//お互いにコールバック関数が用意されていないなら、当たっても何も起こらないので判定を行う必要は無い
			if (!colA->HaveCallBack(colB) && !colB->HaveCallBack(colA))continue;

			CollisionResultInfo info;
			if (colA->CheckHitCollision(colB, &info))
			{
				OnHit(*itrA, *itrB, info);
				OnHit(*itrB, *itrA, info);
			}
		}
	}
}

void KuroEngine::CollisionManager::DebugDraw(Camera& Cam)
{
	for (auto& c : m_colliderList)
	{
		if (!c->m_isActive)continue;
		c->DebugDraw(Cam);
	}
}

bool KuroEngine::CollisionManager::RaycastHit(Vec3<float> arg_start, Vec3<float> arg_dir, RaycastHitInfo* arg_hitInfo, std::string arg_targetTag, float arg_maxDist)
{
	bool result = false;
	//今までで最も近い衝突したコライダー
	std::weak_ptr<Collider> nearestCol;
	//その際の交点
	Vec3<float> nearestInter;
	//その際の交点までの距離
	float nearestDist = arg_maxDist;
	
	//レイをコリジョン線分に返還
	CollisionLine ray(arg_start, arg_dir, arg_maxDist);
	//衝突点の一時格納先
	CollisionResultInfo info;

	//全てのコライダーと総当たり
	for (auto it = m_colliderList.begin(); it != m_colliderList.end(); ++it)
	{
		//イテレータよりコライダー取得
		auto col = (*it);

		//非アクティブならスルー
		if (!col->m_isActive)continue;

		//当たり判定の対象となるタグが指定されていて、それと同一でないならスルー
		if (!arg_targetTag.empty() && col->HaveTag(arg_targetTag) != 0)continue;

		//全てのプリミティブと判定
		for (auto& primitive : col->m_primitiveArray)
		{
			//当たっていないならスルー
			if (!ray.HitCheckDispatch(XMMatrixIdentity(), col->GetTransformMat(), primitive.get(), &info))continue;

			//距離を取得
			float dist = arg_start.Distance(info.m_inter);
			//最短でないならスルー
			if (nearestDist < dist)continue;

			//衝突情報更新
			nearestCol = col;
			nearestInter = info.m_inter;
			nearestDist = dist;
			result = true;
		}
	}

	//最終的に何かに当たっていたら結果を書き込む
	if (result && arg_hitInfo)
	{
		arg_hitInfo->m_otherCol = nearestCol;
		arg_hitInfo->m_inter = nearestInter;
		arg_hitInfo->m_distToInter = nearestDist;
	}

	return result;
}

void KuroEngine::CollisionManager::Register(const std::shared_ptr<Collider>& arg_collider)
{
	for (auto& c : m_colliderList)
	{
		//登録済
		if (*c == *arg_collider)
		{
			printf("Error : This collider ( %s ) has already registered.", arg_collider->m_name.c_str());
			assert(0);
		}
	}
	m_colliderList.emplace_back(arg_collider);
}

void KuroEngine::CollisionManager::Remove(const std::shared_ptr<Collider>& arg_collider)
{
	m_colliderList.remove_if([arg_collider](std::shared_ptr<Collider> col)
		{
			return *col == *arg_collider;
		});
}