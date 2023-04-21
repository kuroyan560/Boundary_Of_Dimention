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
	KuroEngine::Transform m_prevTransform;
	KuroEngine::Transform m_transform;
	KuroEngine::Transform m_initTransform;

	//移動量
	KuroEngine::Vec3<float> m_rowMoveVec;

	//カメラインスタンス
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//カメラのコントローラー
	CameraController m_camController;

	//カメラ感度
	float m_camSensitivity = 1.0f;
	int m_cameraMode;
	std::array<const float, 3> CAMERA_MODE = { -20.0f,-40.0f,-70.0f };

	//植物を繁殖させる点光源
	GrowPlantLight_Point m_growPlantPtLig;

	//草を生やす際の散らし量。
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y軸回転量
	float m_cameraRotY;	//カメラのY軸回転量。この角度をもとにプレイヤーの移動方向を判断する。
	float m_cameraRotYStorage;	//カメラのY軸回転量。
	float m_cameraJumpLerpStorage;	//プレイヤーがジャンプ中に回転を補間する際の補間元。
	float m_cameraJumpLerpAmount;	//プレイヤーがジャンプ中に回転する際に補間する量。
	float m_playerRotY;	//プレイヤーのY軸回転量。これは見た目上移動方向に向かせるため。正面ベクトルとかに影響が出ないようにしなければならない。
	XMVECTOR m_cameraQ;	//プレイヤーのY軸回転量を抜いた、カメラ方向のみを向いた場合の回転
	XMVECTOR m_moveQ;	//プレイヤーのY軸回転量を抜いた、プレイヤーの移動方向のみを向いた場合の回転
	XMVECTOR m_normalSpinQ;	//デフォルトの上ベクトルと現在いる地形の回転量をあらわしたクォータニオン

	//移動ベクトルのスカラー
	KuroEngine::Vec3<float> m_moveSpeed;			//移動速度
	float m_moveAccel = 0.05f;
	float m_maxSpeed = 0.5f;
	float m_brake = 0.07f;

	//ギミックの移動量
	KuroEngine::Vec3<float> m_gimmickVel;

	//壁移動の距離
	const float WALL_JUMP_LENGTH = 3.0f;

	//接地フラグ
	bool m_isFirstOnGround;	//開始時に空中から始まるので、着地済みだということを判断する用変数。
	bool m_onGround;		//接地フラグ
	bool m_prevOnGround;	//1F前の接地フラグ
	bool m_prevOnGimmick;	//ギミックの上にいるかどうか
	bool m_onGimmick;		//ギミックの上にいるかどうか
	bool m_isDeath;			//プレイヤーが死んだかどうか 死んだらリトライされる
	int m_deathTimer;		//このフレームの間死んでいたら死んだ判定にする。
	const int DEATH_TIMER = 1;

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

	//プレイヤーの動きのステータス
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//通常移動中
		JUMP,	//ジャンプ中(補間中)
		ZIP,	//ジップライン移動中
	}m_playerMoveStatus;

	//ギミックにより操作不能のときのステータス
	enum class GIMMICK_STATUS {
		APPEAR,
		NORMAL,
		EXIT
	}m_gimmickStatus;

	//プレイヤーのジャンプに関する変数
	KuroEngine::Vec3<float> m_jumpStartPos;			//ジャンプ開始位置
	KuroEngine::Vec3<float> m_jumpEndPos;			//ジャンプ終了位置
	KuroEngine::Vec3<float> m_bezierCurveControlPos;//ジャンプベジェ曲線の制御点	
	XMVECTOR m_jumpStartQ;							//ジャンプ開始時のクォータニオン
	XMVECTOR m_jumpEndQ;							//ジャンプ終了時のクオータニオン
	float m_jumpTimer;								//ジャンプの計測時間を図るタイマー
	const float JUMP_TIMER = 0.08f;
	bool m_canJump;									//ジャンプができるかのフラグ
	int m_canJumpDelayTimer;						//ジャンプができるようになるまでの引っ掛かり
	const int CAN_JUMP_DELAY = 20;
	const int CAN_JUMP_DELAY_FAST = 1;

	//ジップライン関係のステータス
	const int ZIP_LINE_MOVE_TIMER_START = 30;
	const int ZIP_LINE_MOVE_TIMER_END = 30;
	int m_ziplineMoveTimer;							//ジップラインに入ったり出たりするときに使うタイマー
	KuroEngine::Vec3<float> m_zipInOutPos;			//ジップラインに出たり入ったりするときの場所。イージングに使用する。
	std::weak_ptr<IvyZipLine> m_refZipline;

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo = nullptr);

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
	//KuroEngine::Transform& GetCameraControllerParentTransform() { return m_camController.GetParentTransform(); }

	KuroEngine::Transform& GetTransform() { return m_transform; }
	KuroEngine::Vec2<float>& GetGrassPosScatter() { return m_grassPosScatter; }

	void SetGimmickVel(const KuroEngine::Vec3<float>& arg_gimmickVel) { m_gimmickVel = arg_gimmickVel; }

	//点光源ゲッタ
	KuroEngine::Light::Point* GetPointLig() { return &m_ptLig; }
	bool GetOnGround() { return m_onGround; }
	bool GetOnGimmick() { return m_onGimmick; }
	bool GetIsStatusMove() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE; }

	//ジャンプ終了地点
	KuroEngine::Vec3<float> GetJumpEndPos() { return m_jumpEndPos; }
	void SetJumpEndPos(KuroEngine::Vec3<float> arg_jumpEndPos) { m_jumpEndPos = arg_jumpEndPos; }
	bool GetIsJump() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP; }

	//死亡フラグを取得。
	bool GetIsDeath() { return m_isDeath; }

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

		GROUND,	//地上向かって飛ばすレイ。設置判定で使用する。
		AROUND,	//周囲に向かって飛ばすレイ。壁のぼり判定で使用する。
		CHECK_DEATH,	//死亡確認用
		CHECK_CLIFF,	//崖チェック用

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
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal), m_isActive(true), m_isFastJump(false) {};
	};

	//CastRayに渡すデータ用構造体
	struct CastRayArgument {
		std::weak_ptr<StageParts> m_stage;			//ステージ
		std::vector<TerrianHitPolygon> m_mesh;		//判定を行う対象のメッシュ
		std::vector<ImpactPointData> m_impactPoint;	//前後左右のレイの当たった地点。
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
		StageParts::STAGE_PARTS_TYPE m_stageType;
		std::array<bool, 6> m_checkDeathCounter;
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

	//移動させる。
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//当たり判定
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage);

	//ベジエ曲線を求める。
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//移動方向を正しくさせるための処理
	void AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp);

	//周囲用の当たり判定
	void CheckDeath(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckCliff(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);
	void CheckCanJump(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);

	//ジップラインとの当たり判定
	void CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage);

	//ジップライン中の更新処理
	void UpdateZipline();

};

