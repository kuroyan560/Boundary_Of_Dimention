#pragma once
#include"Common/Vec.h"
namespace KuroEngine
{
	class CollisionPrimitive;
	//衝突判定の結果情報
	class CollisionResultInfo
	{
	public:
		//衝突点
		Vec3<float>m_inter;
		//当たり判定の合った相手のプリミティブ
		CollisionPrimitive* m_hitOtherPrimitive = nullptr;
	};
}