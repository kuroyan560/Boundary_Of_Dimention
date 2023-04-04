#pragma once
#include"Common/Transform.h"
#include"Player/CameraController.h"

struct MovieCameraData
{
	KuroEngine::Transform transform;	//次の場所に向かう座標
	bool skipInterpolationFlag;			//座標と角度の補間をスキップするかどうか
	int interpolationTimer;				//
	int stopTimer;						//カメラが定位置についてどれくらい止まるか
};

class MovieCamera
{
public:
	MovieCamera();
	void Update();

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_cam;
	}
private:
	std::shared_ptr<KuroEngine::Camera>m_cam;
};

