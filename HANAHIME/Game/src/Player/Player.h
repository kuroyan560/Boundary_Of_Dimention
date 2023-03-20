#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"

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

	//Y軸回転量
	float m_cameraRotY;	//カメラのY軸回転量。この角度をもとにプレイヤーの移動方向を判断する。
	float m_playerRotY;	//プレイヤーのY軸回転量。これは見た目上移動方向に向かせるため。正面ベクトルとかに影響が出ないようにしなければならない。
	XMVECTOR m_cameraQ;	//プレイヤーのY軸回転量を抜いた、カメラ方向のみを向いた場合の回転

	//移動ベクトルのスカラー
	float m_moveScalar = 0.5f;

	//接地フラグ
	bool m_onGround;		//接地フラグ
	bool m_prevOnGround;	//1F前の接地フラグ

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_interPos;
		KuroEngine::Vec3<float>m_terrianNormal;
		KuroEngine::Vec3<float>m_bottmRayTerrianNormal;
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
		CLIFF,	//崖で明日もとに向かって飛ばすレイ。崖を降りる際に使用する。
	};
	/// <summary>
	/// レイとメッシュの当たり判定
	/// </summary>
	/// <param name="arg_rayPos"> レイの射出地点 </param>
	/// <param name="arg_rayDir"> レイの射出方向 </param>
	/// <param name="arg_targetMesh"> 判定を行う対象のメッシュ </param>
	/// <param name="arg_targetTransform"> 判定を行う対象のトランスフォーム </param>
	/// <returns> 当たり判定結果 </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform);

	/// <summary>
	/// 重心座標を求める。
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//CastRayに渡すデータ用構造体
	struct CastRayArgument {
		KuroEngine::ModelMesh m_mesh;				//判定を行う対象のメッシュ
		KuroEngine::Transform m_targetTransform;	//判定を行う対象のトランスフォーム
		bool& m_onGround;							//接地フラグ
		bool& m_isHitWall;							//レイが壁に当たったかどうか
		HitCheckResult& m_hitResult;				//当たり判定結果データ
		CastRayArgument(bool& arg_onGround, bool& arg_isHitWall, HitCheckResult& arg_hitResult) : m_onGround(arg_onGround), m_isHitWall(arg_isHitWall), m_hitResult(arg_hitResult) {};
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
	void CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument arg_collisionData, RAY_ID arg_rayID);

};

