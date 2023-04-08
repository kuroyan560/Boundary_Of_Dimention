#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/Stage.h"
#include"Render/RenderObject/Light.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;
struct Terrian;

class Player : public KuroEngine::Debugger
{
	//プレイヤーのモデル
	std::shared_ptr<KuroEngine::Model>m_model;
	std::shared_ptr<KuroEngine::Model>m_axisModel;

	//カメラのモデル（デバッグ用）
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//点光源
	KuroEngine::Light::Point m_ptLig;

	//トランスフォーム
	KuroEngine::Transform m_transform;

	//移動量
	KuroEngine::Vec3<float> m_rowMoveVec;

	//カメラインスタンス
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//カメラのコントローラー
	CameraController m_camController;

	//カメラ感度
	float m_camSensitivity = 1.0f;

	//草を生やす際の散らし量。
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y軸回転量
	float m_cameraRotY;	//カメラのY軸回転量。この角度をもとにプレイヤーの移動方向を判断する。
	float m_cameraRotYStorage;	//カメラのY軸回転量。
	float m_playerRotY;	//プレイヤーのY軸回転量。これは見た目上移動方向に向かせるため。正面ベクトルとかに影響が出ないようにしなければならない。
	XMVECTOR m_cameraQ;	//プレイヤーのY軸回転量を抜いた、カメラ方向のみを向いた場合の回転
	XMVECTOR m_moveQ;	//プレイヤーのY軸回転量を抜いた、プレイヤーの移動方向のみを向いた場合の回転
	XMVECTOR m_normalSpinQ;	//デフォルトの上ベクトルと現在いる地形の回転量をあらわしたクォータニオン
	bool m_isNoLerpCamera;	//カメラを補間するかどうか

	//移動ベクトルのスカラー
	KuroEngine::Vec3<float> m_moveSpeed;			//移動速度
	float m_moveAccel = 0.05f;
	float m_maxSpeed = 0.5f;
	float m_brake = 0.07f;

	//壁移動の距離
	const float WALL_JUMP_LENGTH = 6.0f;

	//接地フラグ
	bool m_isFirstOnGround;	//開始時に空中から始まるので、着地済みだということを判断する用変数。
	bool m_onGround;		//接地フラグ
	bool m_prevOnGround;	//1F前の接地フラグ

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

	//プレイヤーの動きのステータス
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//通常移動中
		JUMP,	//ジャンプ中(補間中)
	}m_playerMoveStatus;

	//プレイヤーのジャンプに関する変数
	KuroEngine::Vec3<float> m_jumpStartPos;
	KuroEngine::Vec3<float> m_jumpEndPos;
	KuroEngine::Vec3<float> m_bezierCurveControlPos;
	XMVECTOR m_jumpStartQ;
	XMVECTOR m_jumpEndQ;
	float m_jumpTimer;
	const float JUMP_TIMER = 0.07f;


	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, const std::vector<Terrian>& arg_terrianArray, HitCheckResult* arg_hitInfo = nullptr);

	//プレイヤーの大きさ（半径）
	float GetPlayersRadius()
	{
		return m_transform.GetScale().x;
	}

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update(const std::weak_ptr<Stage>arg_nowStage);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw = false);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }

	//カメラコントローラーのデバッガポインタ取得
	KuroEngine::Debugger* GetCameraControllerDebugger() { return &m_camController; }

	KuroEngine::Transform& GetTransform() { return m_transform; }
	KuroEngine::Vec2<float>& GetGrassPosScatter() { return m_grassPosScatter; }

	//点光源ゲッタ
	KuroEngine::Light::Point* GetPointLig() { return &m_ptLig; }
	bool GetOnGround() { return m_onGround; }
	bool GetIsStatusMove() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE; }

private:
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

		CHECK_CLIFF,	//崖かどうかをチェックする用
		GROUND,	//地上向かって飛ばすレイ。設置判定で使用する。
		AROUND,	//周囲に向かって飛ばすレイ。壁のぼり判定で使用する。

	};
	/// <summary>
	/// レイとメッシュの当たり判定
	/// </summary>
	/// <param name="arg_rayPos"> レイの射出地点 </param>
	/// <param name="arg_rayDir"> レイの射出方向 </param>
	/// <param name="arg_targetMesh"> 判定を行う対象のメッシュ </param>
	/// <param name="arg_targetTransform"> 判定を行う対象のトランスフォーム </param>
	/// <returns> 当たり判定結果 </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh, KuroEngine::Transform arg_targetTransform);

	/// <summary>
	/// 重心座標を求める。
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//衝突点用構造体
	struct ImpactPointData {
		KuroEngine::Vec3<float> m_impactPos;
		KuroEngine::Vec3<float> m_normal;
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal) {};
	};

	//CastRayに渡すデータ用構造体
	struct CastRayArgument {
		std::vector<Terrian::Polygon> m_mesh;		//判定を行う対象のメッシュ
		KuroEngine::Transform m_targetTransform;	//判定を行う対象のトランスフォーム
		std::vector<ImpactPointData> m_impactPoint;	//前後左右のレイの当たった地点。
		bool m_onGround;							//接地フラグ
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
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
	bool CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID);

	//移動させる。
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//当たり判定
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, const std::weak_ptr<Stage>arg_nowStage);

	//ベジエ曲線を求める。
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//移動方向を正しくさせるための処理
	void AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp);

};

