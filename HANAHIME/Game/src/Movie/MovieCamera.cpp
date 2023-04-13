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

	//����_�̊J�n�n�_
	KuroEngine::Matrix matA = m_moveDataArray[m_moveDataIndex].transform.GetMatWorld();
	//����_�̏I���n�_
	KuroEngine::Matrix matB = m_moveDataArray[m_moveDataIndex + 1].transform.GetMatWorld();
	//�s��̕��
	m_nowTransform.SetWorldMat(Ease(matA, matB, m_timerArray[m_moveDataIndex].GetTimeRate(), m_moveDataArray[m_moveDataIndex].easePosData, m_moveDataArray[m_moveDataIndex].easeRotaData));

	//�n�߂Ɏ~�܂鎞��
	if (m_moveDataArray[m_moveDataIndex].preStopTimer * 60 <= m_preStopTimer)
	{
		m_timerArray[m_moveDataIndex].UpdateTimer();
	}
	else
	{
		++m_preStopTimer;
	}



	//�J�����������Ă���~�܂��Ă��鎞��----------------------------------------
	//��~�J�n
	if (m_timerArray[m_moveDataIndex].IsTimeUp())
	{
		m_stopFlag = true;
	}
	//��~��
	if (m_stopFlag)
	{
		++m_afterStopTimer;
	}
	//��莞�Ԏ~�܂�����
	if (m_stopFlag && m_moveDataArray[m_moveDataIndex].afterStopTimer * 60 <= m_afterStopTimer)
	{
		++m_moveDataIndex;

		m_preStopTimer = 0;
		m_afterStopTimer = 0;
		m_stopFlag = false;
	}
	//�J�����������Ă���~�܂��Ă��鎞��----------------------------------------

	//���̏����Q�Ƃ��Ă���̂ŁA�ő�10�Ȃ�8�ڂ̎Q�Ƃ��I���������_�ŏI������
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
	//�f�[�^��������͋@�\���Ȃ�
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