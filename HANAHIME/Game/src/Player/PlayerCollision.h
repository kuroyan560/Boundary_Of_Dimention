#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/StageParts.h"
#include"Render/RenderObject/Light.h"
#include"../Plant/GrowPlantLight.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;
class Player;

struct PlayerCollision {

	Player* m_refPlayer;	//プレイヤーの参照

	//レイとメッシュの当たり判定出力用構造体
	struct MeshCollisionOutput {
		bool m_isHit;						// レイがメッシュに当たったかどうか 当たったメッシュまでの距離は考慮されておらず、ただ当たったかどうかを判断する用。
		float m_distance;					// レイがメッシュに当たった場合、衝突地点までの距離が入る。このパラメーターとm_isHitを組み合わせて正しい衝突判定を行う。
		KuroEngine::Vec3<float> m_pos;		// レイの衝突地点の座標
		KuroEngine::Vec3<float> m_normal;	// レイの衝突地点の法線
		KuroEngine::Vec2<float> m_uv;		// レイの衝突地点のUV
	};
	//発射するレイのID
	enum class RAY_ID {

		GROUND,	//地上向かって飛ばすレイ。設置判定で使用する。
		AROUND,	//周囲に向かって飛ばすレイ。壁のぼり判定で使用する。
		CLIFF,	//崖際の押し戻し用のレイ。仮の壁をつくってそれの押し戻し判定を行う。
		CHECK_GIMMICK,	//ギミックに乗っているか判定用。onGimmickをtrueにする。押し戻しはしない。
		CHECK_DEATH,	//死亡確認用
		CHECK_CLIFF,	//崖チェック用
		CHECK_IVY,

	};
	//レイの方向ID
	enum class RAY_DIR_ID {
		RIGHT,
		LEFT,
		FRONT,
		BEHIND,
		TOP,
		BOTTOM,
	};

	/// <summary>
	/// レイとメッシュの当たり判定
	/// </summary>
	/// <param name="arg_rayPos"> レイの射出地点 </param>
	/// <param name="arg_rayDir"> レイの射出方向 </param>
	/// <param name="arg_targetMesh"> 判定を行う対象のメッシュ </param>
	/// <returns> 当たり判定結果 </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<TerrianHitPolygon>& arg_targetMesh);

	/// <summary>
	/// 重心座標を求める。
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//衝突点用構造体
	struct ImpactPointData {
		KuroEngine::Vec3<float> m_impactPos;
		KuroEngine::Vec3<float> m_normal;
		bool m_isActive;
		bool m_isFastJump;	//ぶつかってから暫くしてジャンプするのではなく、すぐに飛び乗るフラグ。すぐに乗りたいオブジェクトがある場合、これをtrueにする。
		bool m_isAppearWall;
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal), m_isActive(true), m_isFastJump(false), m_isAppearWall(false) {};
	};

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo = nullptr);

	//CastRayに渡すデータ用構造体
	struct CastRayArgument {
		std::weak_ptr<StageParts> m_stage;			//ステージ
		std::vector<TerrianHitPolygon> m_mesh;		//判定を行う対象のメッシュ
		std::vector<ImpactPointData> m_impactPoint;	//前後左右のレイの当たった地点。
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
		StageParts::STAGE_PARTS_TYPE m_stageType;
		std::array<bool, 6> m_checkDeathCounter;
		std::array<bool, 4> m_checkHitAround;
	};

	/// <summary>
	/// レイを発射し、その後の一連の処理をまとめた関数
	/// </summary>
	/// <param name="arg_rayCastPos"> キャラクターの座標 </param>
	/// <param name="arg_rayCastPos"> レイの射出地点 </param>
	/// <param name="arg_rayDir"> レイの射出方向 </param>
	/// <param name="arg_rayLength"> レイの長さ </param>
	/// <param name="arg_collisionData"> 引数をまとめた構造体 </param>
	/// <param name="arg_rayID"> レイの種類 </param>
	bool CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID = RAY_DIR_ID::RIGHT);

	//当たり判定
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage);

	//周囲用の当たり判定
	void CheckDeath(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckCliff(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);
	void CheckCanJump(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);

	//ジップラインとの当たり判定
	void CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage);

	//ギミックによる移動を終わらせる。
	void FinishGimmickMove();

	//移動方向を正しくさせるための処理
	void AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp);

};