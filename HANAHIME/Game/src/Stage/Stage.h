#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>
#include<map>
#include"../Effect/GuideInsect.h"
#include"../GPUParticle/GPUParticleRender.h"
#include"FunaCameraPoint.h"

namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

class Switch;

class Stage
{
private:
	//地形のスケール
	float m_terrianScaling = 1.0f;

	//地形情報配列
	std::vector<Terrian>m_terrianArray;
	//ギミック配列（要素のサイズが異なるためlistを利用）
	std::list<std::shared_ptr<StageParts>>m_gimmickArray;
	//敵配列
	std::list<std::shared_ptr<StageParts>>m_enemyArray;
	//スタート地点
	std::shared_ptr<StartPoint>m_startPoint;
	//ゴール地点
	std::shared_ptr<GoalPoint>m_goalPoint;
	//ゲート配列
	std::vector<std::weak_ptr<Gate>>m_gateArray;
	//チェックポイント配列
	std::vector<std::weak_ptr<CheckPoint>>m_checkPointArray;

	//マップピンを指すパーツのデータの一時格納用
	struct MapPinPointData
	{
		std::weak_ptr<StageParts>m_part;
		int m_order;	//順番
	};
	//マップピンを指すパーツのデータ配列
	std::vector<std::weak_ptr<StageParts>>m_mapPinPoint;
	//全ての目的地を巡回済
	bool m_isCompleteMapPin = false;

	//スターコインのポインタ配列
	std::vector<std::weak_ptr<StarCoin>>m_starCoinArray;

	//FunaCameraの配列
	std::vector<FunaCameraPoint>m_funaCamPtArray;

	//モデル
		//地形モデルの存在するディレクトリ
	static std::string s_stageModelDir;

	//画像
		//地面
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

	std::shared_ptr<KuroEngine::Model>m_skydomeModel;

	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

	//全てをオン状態にすることがクリア条件となるレバーの識別番号
	int m_goalLeverID = Lever::INVALID_ID;
	//クリアのスイッチ
	std::shared_ptr<Switch>m_goalSwitch;

	//キーがjsonファイルに含まれているか、含まれていなかったらエラーで終了
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

	//座標系を考慮した座標読み込み
	KuroEngine::Vec3<float>GetConsiderCoordinate(nlohmann::json arg_json);

	//座標配列の読み込み
	bool LoadTranslationArray(std::string arg_fileName,
		std::vector<KuroEngine::Vec3<float>> *arg_result,
		nlohmann::json arg_json);

	//種別に応じて読み込みを分岐させる
	void LoadWithType(std::string arg_fileName, nlohmann::json arg_json, StageParts *arg_parent, std::vector<MapPinPointData> &arg_mapPinDataArray);

public:
	Stage();

	void Init();
	void Update(Player &arg_player);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	//クリア判定
	bool IsClear()const;

	//ステージ情報読み込み
	void Load(int arg_ownStageIdx, std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//通常の地形の配列取得
	const std::vector<Terrian> &GetTerrianArray()const { return m_terrianArray; }
	//ギミック配列取得
	std::list<std::shared_ptr<StageParts>> &GetGimmickArray() { return m_gimmickArray; }

	//入手したスターコインの数
	int GetStarCoinNum()const;
	//存在するスターコインの数
	int ExistStarCoinNum()const { return static_cast<int>(m_starCoinArray.size()); }

	//ゲートのIDからゲートのトランスフォームを取得
	KuroEngine::Transform GetGateTransform(int arg_gateID)const;

	//プレイヤーの初期化トランスフォームゲッタ
	KuroEngine::Transform GetStartPointTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

	//ゴールのトランスフォームゲッタ
	KuroEngine::Transform GetGoalTransform()const
	{
		if (m_goalPoint)return m_goalPoint->GetTransform();
		return KuroEngine::Transform();
	};

	std::shared_ptr<GoalPoint>GetGoalModel()
	{
		return m_goalPoint;
	}

	void SetCompleteMapPinFlg(bool arg_flg) { m_isCompleteMapPin = arg_flg; }
	const bool &GetCompleteMapPin()const { return m_isCompleteMapPin; }

	//モデルのゲッタ
		//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

	//マップピンを指すパーツの配列
	std::vector<std::weak_ptr<StageParts>>GetMapPinPointArray()const { return m_mapPinPoint; }
	//チェックポイント配列
	std::vector<std::weak_ptr<CheckPoint>>GetCheckPointArray()const { return m_checkPointArray; }
	//FunaCameraPoint配列
	std::vector<FunaCameraPoint>GetFunaCamPtArray()const { return m_funaCamPtArray; }

	//ガイド用の虫
	GuideInsect m_guideInsect;

	void CheckPointReset();
};