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

	//カメラインスタンス
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//カメラのコントローラー
	CameraController m_camController;

	//カメラ感度
	float m_camSensitivity = 1.0f;

	float m_moveScalar = 0.5f;

	//接地フラグ
	bool m_onGround;		//接地フラグ
	bool m_prevOnGround;	//1F前の接地フラグ

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

	bool HitCheck(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_to, const std::vector<Terrian>& arg_terrianArray, KuroEngine::Vec3<float>* arg_terrianNormal = nullptr);

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

	/// <summary>
	/// ベクトルに行列を乗算する。
	/// </summary>
	KuroEngine::Vec3<float> MulMat(const KuroEngine::Vec3<float>& arg_target, const DirectX::XMMATRIX arg_mat);

	/// <summary>
	/// レイを発射し、その後の一連の処理をまとめた関数
	/// </summary>
	/// <param name="arg_rayPos"> レイの射出地点 </param>
	/// <param name="arg_rayDir"> レイの射出方向 </param>
	/// <param name="arg_rayLength"> レイの長さ </param>
	/// <param name="arg_targetMesh"> 判定を行う対象のメッシュ </param>
	/// <param name="arg_targetTransform"> 判定を行う対象のトランスフォーム </param>
	/// <param name="arg_onGround"> 接地フラグ </param>
	/// <param name="arg_isHitWall"> レイが壁に当たったかどうか </param>
	/// <param name="arg_hitNormal"> レイが当たった地点の法線 </param>
	/// <param name="arg_rayID"> レイの種類 </param>
	void CastRay(KuroEngine::Vec3<float>& arg_rayPos, KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform, bool& arg_onGround, bool& arg_isHitWall, KuroEngine::Vec3<float>& arg_hitNormal, RAY_ID arg_rayID);

};

