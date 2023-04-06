#include "MovieCamera.h"

MovieCamera::MovieCamera() :m_startFlag(false), m_stopFlag(false), m_stopTimer(0)
{
	std::string cameraName = "MovieCamera";
	m_cam = std::make_shared<KuroEngine::Camera>(cameraName);

	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
}

void MovieCamera::Update()
{
	if (!m_startFlag)
	{
		return;
	}

	KuroEngine::Vec3<float> pos(
		KuroEngine::Math::Ease(
			KuroEngine::In,
			KuroEngine::Cubic,
			m_timerArray[m_moveDataIndex].GetTimeRate(),
			m_moveDataArray[m_moveDataIndex].pos,
			m_moveDataArray[m_moveDataIndex + 1].pos
		));


	//座標の補間
	m_nowTransform.SetPos(
		pos
	);

	//角度の補間
	KuroEngine::Vec3<float> startRotate = m_moveDataArray[m_moveDataIndex].rotation;
	KuroEngine::Vec3<float> endRotate = m_moveDataArray[m_moveDataIndex + 1].rotation;

	m_nowTransform.SetRotate(
		KuroEngine::Math::Ease(
			KuroEngine::In,
			KuroEngine::Cubic,
			m_timerArray[m_moveDataIndex].GetTimeRate(),
			startRotate,
			endRotate
		)
	);

	//現在はプレイヤーカメラの情報をそのまま代入してカメラの位置が取れているか確認している
	m_nowTransform = m_directCameraTransform;

	m_timerArray[m_moveDataIndex].UpdateTimer();


	//カメラが動いてから止まっている時間----------------------------------------
	//停止開始
	if (m_timerArray[m_moveDataIndex].IsTimeUp())
	{
		m_stopFlag = true;
	}
	//停止中
	if (m_stopFlag)
	{
		++m_stopTimer;
	}
	//一定時間止まった後
	if (m_moveDataArray[m_moveDataIndex].stopTimer * 60 <= m_stopTimer)
	{
		++m_moveDataIndex;

		m_stopTimer = 0;
		m_stopFlag = false;
	}
	//カメラが動いてから止まっている時間----------------------------------------


	if (m_moveDataArray.size() - 1 <= m_moveDataIndex)
	{
		m_startFlag = false;
	}

	auto &copy = m_cam->GetTransform();
	copy = m_nowTransform;

}

void MovieCamera::StartMovie(KuroEngine::Vec3<float> camera_pos, KuroEngine::Vec3<float> front_vec, std::vector<MovieCameraData> move_data)
{
	//データが二個未満は機能しない
	if (move_data.size() < 2)
	{
		assert(0);
	}
	m_startFlag = true;
	m_moveDataArray = move_data;
	m_moveDataIndex = 0;

	m_timerArray.clear();
	m_timerArray.shrink_to_fit();
	for (int i = 0; i < move_data.size(); ++i)
	{
		m_timerArray.emplace_back(static_cast<float>(m_moveDataArray[i].interpolationTimer * 60));
	}
}

bool MovieCamera::IsStart()
{
	return m_startFlag;
}

bool MovieCamera::IsFinish()
{
	return m_finishFlag;
}