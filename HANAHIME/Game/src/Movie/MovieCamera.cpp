#include "MovieCamera.h"
#include<array>

MovieCamera::MovieCamera() :m_startFlag(false), m_stopFlag(false), m_preStopTimer(0)
{
	std::string cameraName = "MovieCamera";
	m_cam = std::make_shared<KuroEngine::Camera>(cameraName);
	m_cam->SetFarZ(10000.0f);
}

void MovieCamera::Init()
{
	m_startFlag = false;
	m_finishFlag = false;
}

void MovieCamera::Update()
{
	if (!m_startFlag)
	{
		return;
	}

	//制御点の開始地点
	KuroEngine::Matrix matA = m_moveDataArray[m_moveDataIndex].transform.GetMatWorld();
	//制御点の終了地点
	KuroEngine::Matrix matB = m_moveDataArray[m_moveDataIndex + 1].transform.GetMatWorld();
	//行列の補間
	m_nowTransform.SetWorldMat(Ease(matA, matB, m_timerArray[m_moveDataIndex].GetTimeRate(), m_moveDataArray[m_moveDataIndex].easePosData, m_moveDataArray[m_moveDataIndex].easeRotaData));

	//始めに止まる時間
	if (m_moveDataArray[m_moveDataIndex].preStopTimer * 60 <= m_preStopTimer)
	{
		m_timerArray[m_moveDataIndex].UpdateTimer();
	}
	else
	{
		++m_preStopTimer;
	}



	//カメラが動いてから止まっている時間----------------------------------------
	//停止開始
	if (m_timerArray[m_moveDataIndex].IsTimeUp())
	{
		m_stopFlag = true;
	}
	//停止中
	if (m_stopFlag)
	{
		++m_afterStopTimer;
	}
	//一定時間止まった後
	if (m_stopFlag && m_moveDataArray[m_moveDataIndex].afterStopTimer * 60 <= m_afterStopTimer)
	{
		++m_moveDataIndex;

		m_preStopTimer = 0;
		m_afterStopTimer = 0;
		m_stopFlag = false;
	}
	//カメラが動いてから止まっている時間----------------------------------------

	//一個先の情報も参照しているので、最大10個なら8個目の参照が終了した時点で終了する
	bool finishFlag = m_moveDataArray.size() - 1 <= m_moveDataIndex;
	if (finishFlag && m_isLoopFlag)
	{
		m_moveDataIndex = 0;
		for (int i = 0; i < m_timerArray.size(); ++i)
		{
			m_timerArray[i].Reset();
		}
	}
	else if (finishFlag)
	{
		m_startFlag = false;
		m_finishFlag = true;
	}

	
	m_nowTransform.CalucuratePosRotaBasedOnWorldMatrix();
	auto &copy = m_cam->GetTransform();
	copy.SetParent(&m_nowTransform);
}

void MovieCamera::StartMovie(std::vector<MovieCameraData> &move_data,bool loop_flag)
{
	//データが二個未満は機能しない
	if (move_data.size() < 2)
	{
		assert(0);
	}
	m_startFlag = true;
	m_finishFlag = false;
	m_moveDataArray = move_data;
	m_moveDataIndex = 0;

	m_timerArray.clear();
	m_timerArray.shrink_to_fit();
	for (int i = 0; i < move_data.size(); ++i)
	{
		m_timerArray.emplace_back(static_cast<float>(1 + m_moveDataArray[i].interpolationTimer * 60));
	}


	m_splinePosArray.clear();
	m_splinePosArray.shrink_to_fit();
	m_splinePosArray.emplace_back(move_data[0].transform.GetPosWorld());
	for (int i = 0; i < move_data.size(); ++i)
	{
		m_splinePosArray.emplace_back(move_data[i].transform.GetPosWorld());
	}
	m_splinePosArray.emplace_back(move_data[move_data.size() - 1].transform.GetPosWorld());

	m_isLoopFlag = loop_flag;
}

bool MovieCamera::IsStart()
{
	return m_startFlag;
}

bool MovieCamera::IsFinish()
{
	return m_finishFlag;
}