#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/StageParts.h"
#include"Render/RenderObject/Light.h"
#include"../Plant/GrowPlantLight.h"
#include"PlayerCollision.h"
#include"ForUser/Timer.h"
#include"../AI/EnemySearch.h"
#include<memory>
#include"../Item/GameItem.h"

namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;

class Player : public KuroEngine::Debugger
{

	friend PlayerCollision;

	KuroEngine::Vec3<float> m_debug;

	//プレイヤーのモデル
	std::shared_ptr<KuroEngine::Model>m_model;
	std::shared_ptr<KuroEngine::Model>m_axisModel;

	//カメラのモデル（デバッグ用）
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//点光源
	KuroEngine::Light::Point m_ptLig;

	//トランスフォーム
	KuroEngine::Transform m_prevTransform;
	KuroEngine::Transform m_transform;		//当たり判定や移動の際の方向の判定に使用するトランスフォーム
	KuroEngine::Transform m_drawTransform;	//描画に使用するトランスフォーム プレイヤーを沈ませたり、カメラの方向を見ないようにするために使用する。
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
	const float MAX_INFLUENCE_RANGE = 8.0f;
	const float MIN_INFLUENCE_RANGE = 1.0f;
	const float ATTACK_INFLUENCE_RANGE = 3.0f;
	const float SUB_INFLUENCE_RANGE = 0.05f;
	const float ADD_INFLUENCE_RANGE = 0.3f;

	//ステージの参照
	std::weak_ptr<Stage> m_stage;

	//草を生やす際の散らし量。
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y軸回転量
	float m_cameraRotY;	//カメラのY軸回転量。この角度をもとにプレイヤーの移動方向を判断する。
	float m_cameraRotYStorage;	//カメラのY軸回転量。
	float m_cameraRotMove;		//カメラをがくッとさせないために使用する変数
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
	float m_deathEffectCameraZ;	//死亡演出中のカメラ
	const float DEATH_EFFECT_CAMERA_Z = -15.0f;
	const float DEATH_EFFECT_TIMER_SCALE = 0.1f;

	//攻撃判定
	int m_attackTimer;
	const int ATTACK_TIMER = 30;

	//潜る関連
	bool m_isInputUnderGround;			//沈む際の入力がされているかを判断する用。
	bool m_isUnderGround;				//地中の状態か否か
	bool m_canUnderGroundRelease;		//潜る状態が解除できるか。潜っているときに上にフェンスがあると潜りが解除できない。
	bool m_canOldUnderGroundRelease;	//潜る状態が解除できるか。潜っているときに上にフェンスがあると潜りが解除できない。
	const float UNDERGROUND_Y = 6.5f;	//沈む量。
	float m_underGroundEaseTimer;		//沈むときや浮上するときに使用するイージングのタイマー
	const float ADD_UNDERGROUND_EASE_TIMER = 0.04f;

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

	//プレイヤーの動きのステータス
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//通常移動中
		JUMP,	//ジャンプ中(補間中)
		ZIP,	//ジップライン移動中
		DEATH,	//死亡中。
	}m_playerMoveStatus;
	//１フレーム前の動きのステータス
	PLAYER_MOVE_STATUS m_beforePlayerMoveStatus;

	//ギミックにより操作不能のときのステータス
	enum class GIMMICK_STATUS {
		APPEAR,
		NORMAL,
		EXIT
	}m_gimmickStatus;

	//死亡演出のステータス
	enum class DEATH_STATUS {
		APPROACH,
		STAY,
		LEAVE,
	}m_deathStatus;
	int m_deathEffectTimer;
	const int DEATH_EFFECT_APPROACH_TIMER = 30;
	const int DEATH_EFFECT_STAY_TIMER = 30;
	const int DEATH_EFFECT_FINISH_TIMER = 150;
	float m_deathShakeAmount;
	KuroEngine::Vec3<float> m_deathShake;
	const float DEATH_SHAKE_AMOUNT = 3.0f;
	const float SUB_DEATH_SHAKE_AMOUNT = 0.2f;
	bool m_isCameraInvX;

	//プレイヤーのジャンプに関する変数
	KuroEngine::Vec3<float> m_jumpStartPos;			//ジャンプ開始位置
	KuroEngine::Vec3<float> m_jumpEndPos;			//ジャンプ終了位置
	KuroEngine::Vec3<float> m_bezierCurveControlPos;//ジャンプベジェ曲線の制御点	
	XMVECTOR m_jumpStartQ;							//ジャンプ開始時のクォータニオン
	XMVECTOR m_jumpEndQ;							//ジャンプ終了時のクオータニオン
	float m_jumpTimer;								//ジャンプの計測時間を図るタイマー
	const float JUMP_TIMER = 0.05f;
	bool m_canJump;									//ジャンプができるかのフラグ
	int m_canJumpDelayTimer;						//ジャンプができるようになるまでの引っ掛かり
	const int CAN_JUMP_DELAY = 10;
	const int CAN_JUMP_DELAY_FAST = 1;

	//ジップライン関係のステータス
	const int ZIP_LINE_MOVE_TIMER_START = 30;
	const int ZIP_LINE_MOVE_TIMER_END = 30;
	int m_ziplineMoveTimer;							//ジップラインに入ったり出たりするときに使うタイマー
	KuroEngine::Vec3<float> m_zipInOutPos;			//ジップラインに出たり入ったりするときの場所。イージングに使用する。
	std::weak_ptr<IvyZipLine> m_refZipline;
	bool m_canZip;
	std::vector<KuroEngine::Vec3<float>> m_gimmickExitPos;
	std::vector<KuroEngine::Vec3<float>> m_gimmickExitNormal;

	//当たり判定用ステージの参照
	std::weak_ptr<Stage> m_nowStage;

	//死亡スプライト関連の変数
	KuroEngine::Timer m_deathSpriteAnimTimer;		//死亡演出のアニメーションのタイマー
	const float DEATH_SPRITE_TIMER = 4;			//アニメーションを切り替えるタイマー
	int m_deathSpriteAnimNumber;					//アニメーションの連番番号
	static const int DEATH_SPRITE_ANIM_COUNT = 10;	//アニメーションの数
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, DEATH_SPRITE_ANIM_COUNT> m_deathAnimSprite;
	bool m_isFinishDeathAnimation;					//死亡時のアニメーションが終わったか。

	//アニメーター
	std::shared_ptr<KuroEngine::ModelAnimator>m_modelAnimator;
	//モデルのアニメーションパターン
	enum ANIM_PATTERN { ANIM_PATTERN_WAIT, ANIM_PATTERN_INTEREST, ANIM_PATTERN_WALK, ANIM_PATTERN_CLEAR, ANIM_PATTERN_JUMP, ANIM_PATTERN_NUM };
	//モデルのアニメーション名
	const std::array<std::string, ANIM_PATTERN_NUM>m_animNames =
	{
		"Wait","Interest","Walk","Happy","Jump"
	};
	static const int ANIM_INTEREST_CYCLE = 2;
	//待機アニメーションキョロキョロする周期のカウンター
	int m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;

	//アニメーション指定
	void AnimationSpecification(const KuroEngine::Vec3<float>& arg_beforePos, const KuroEngine::Vec3<float>& arg_newPos);

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
	void DrawUI();
	void Finalize();

	//当たり判定クラス
	PlayerCollision m_collision;

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
	bool GetIsFinishDeathAnimation() { return m_isFinishDeathAnimation; }

	bool GetIsUnderGround() { return m_isUnderGround; }

	//座標を返す系。
	KuroEngine::Vec3<float> GetNowPos() { return m_transform.GetPosWorld(); }
	KuroEngine::Vec3<float> GetOldPos() { return m_prevTransform.GetPosWorld(); }

	//ギミックによる移動を終わらせる。
	void FinishGimmickMove();

	//ダメージを与える。とりあえずはこれが呼ばれたらプレイヤーは死ぬ。
	void Damage();

	//攻撃中か？
	bool GetIsAttack() { return 0 < m_attackTimer; }

	void DisactiveLight()
	{
		m_growPlantPtLig.Disactive();
	}
	GrowPlantLight_Point GetGrowPlantLight() { return m_growPlantPtLig; }

	//アウトラインの点線を描画するときに使用する値
	KuroEngine::Vec3<float> GetOutlineStandardVec() {
		return KuroEngine::Math::TransformVec3(KuroEngine::Vec3<float>(0, 0, 1), m_normalSpinQ);
	}

	//草を生やす球との当たり判定
	bool CheckHitGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize);

	Sphere m_sphere;
	float m_radius;

	void GetItemEffect(std::shared_ptr<ItemOnGame::ItemData> itemData)
	{
		HealData healData;
		switch (itemData->m_itemEnum)
		{
		case ITEM_NONE:
			break;
		case ITEM_HEAL:
			healData = dynamic_cast<ItemInterface<HealData>*>(itemData->m_itemInfomation)->m_itemData;
			break;
		default:
			break;
		}
	};

private:

	//移動させる。
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//ベジエ曲線を求める。
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//ジップライン中の更新処理
	void UpdateZipline();

	//死亡中の更新処理
	void UpdateDeath();

};

