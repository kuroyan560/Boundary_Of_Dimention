#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

/// <summary>
/// タイトル画面向けの処理
/// </summary>
class Title
{
public:
	Title();
	void Init();
	void Update(KuroEngine::Transform *player_camera);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	bool IsFinish()
	{
		return m_isFinishFlag;
	}
	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_camera.GetCamera();
	}
private:
	//カメラの移動情報
	std::vector<MovieCameraData>titleCameraMoveDataArray;
	//ゲーム開始フラグ
	bool m_startGameFlag;
	//OPフラグ
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	//終了フラグ
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//タイトルロゴ
	KuroEngine::Vec2<float> m_titlePos, m_titleLogoSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_blackTexBuff;
	KuroEngine::Timer m_alphaRate;


};

