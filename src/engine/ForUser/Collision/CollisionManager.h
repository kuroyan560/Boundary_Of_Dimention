#pragma once
#include<map>
#include<string>
#include<list>
#include<memory>
#include<limits>
#include<vector>
#include"Common/Vec.h"
#include"ForUser/Debugger.h"

namespace KuroEngine
{
	class CollisionResultInfo;
	class Collider;
	class Camera;

	//レイキャストによる当たり判定の情報を格納した構造体
	struct RaycastHitInfo
	{
		//衝突したコライダー
		std::weak_ptr<Collider>m_otherCol;
		//衝突点
		Vec3<float>m_inter;
		//衝突点までの距離
		float m_distToInter;
	};

	class CollisionManager : public Debugger
	{
		//登録されたコライダーリスト
		std::list<std::shared_ptr<Collider>>m_colliderList;

		//衝突していた場合の処理
		void OnHit(const std::shared_ptr<Collider>& arg_myCollider, const std::shared_ptr<Collider>& arg_otherCollider, const CollisionResultInfo& arg_info);
		void OnImguiItems()override;

	public:
		CollisionManager() : Debugger("CollisionManager") {}

		void Update();
		void DebugDraw(Camera& Cam);

		/// <summary>
		/// レイを発射して登録されている全てのコライダーと当たり判定
		/// ※CollisionLineと違い、一番近いコライダーとの判定情報を返す
		/// </summary>
		/// <param name="arg_start">レイの始点</param>
		/// <param name="arg_dir">レイの方向</param>
		/// <param name="arg_hitInfo">判定情報</param>
		/// <param name="arg_targetTag">当たり判定の対象となるコライダーのタグ指定</param>
		/// <param name="arg_maxDist">レイの長さ</param>
		/// <returns>当たり判定結果</returns>
		bool RaycastHit(Vec3<float>arg_start, Vec3<float>arg_dir, RaycastHitInfo* arg_hitInfo, std::string arg_targetTag = "", float arg_maxDist = FLT_MAX);

		//コライダーの登録
		void Register(const std::shared_ptr<Collider>& arg_collider);
		void Register(const std::vector<std::shared_ptr<Collider>>& arg_colliderArray)
		{
			for (auto& col : arg_colliderArray)Register(col);
		}
		//コライダーの登録解除
		void Remove(const std::shared_ptr<Collider>& arg_collider);
	};
}