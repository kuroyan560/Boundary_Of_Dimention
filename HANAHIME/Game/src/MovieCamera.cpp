#include "MovieCamera.h"
#include<array>

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

	//����_�̊J�n�n�_
	KuroEngine::Matrix matA = m_moveDataArray[m_moveDataIndex].transform.GetMatWorld();
	//����_�̏I���n�_
	KuroEngine::Matrix matB = m_moveDataArray[m_moveDataIndex + 1].transform.GetMatWorld();
	//�s��̕��
	m_nowTransform.SetWorldMat(Ease(matA, matB, m_timerArray[m_moveDataIndex].GetTimeRate(), m_moveDataArray[m_moveDataIndex].easePosData, m_moveDataArray[m_moveDataIndex].easeRotaData));


	//���݂̓v���C���[�J�����̏������̂܂ܑ�����ăJ�����̈ʒu�����Ă��邩�m�F���Ă���
	m_timerArray[m_moveDataIndex].UpdateTimer();


	//�J�����������Ă���~�܂��Ă��鎞��----------------------------------------
	//��~�J�n
	if (m_timerArray[m_moveDataIndex].IsTimeUp())
	{
		m_stopFlag = true;
	}
	//��~��
	if (m_stopFlag)
	{
		++m_stopTimer;
	}
	//��莞�Ԏ~�܂�����
	if (m_stopFlag && m_moveDataArray[m_moveDataIndex].stopTimer * 60 <= m_stopTimer)
	{
		++m_moveDataIndex;

		m_stopTimer = 0;
		m_stopFlag = false;
	}
	//�J�����������Ă���~�܂��Ă��鎞��----------------------------------------

	//���̏����Q�Ƃ��Ă���̂ŁA�ő�10�Ȃ�8�ڂ̎Q�Ƃ��I���������_�ŏI������
	if (m_moveDataArray.size() - 1 <= m_moveDataIndex)
	{
		m_startFlag = false;
	}

	auto &copy = m_cam->GetTransform();
	copy.SetParent(&m_nowTransform);

}

void MovieCamera::StartMovie(KuroEngine::Transform transform, std::vector<MovieCameraData> move_data)
{
	//�f�[�^��������͋@�\���Ȃ�
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


	m_splinePosArray.clear();
	m_splinePosArray.shrink_to_fit();
	m_splinePosArray.emplace_back(move_data[0].transform.GetPosWorld());
	for (int i = 0; i < move_data.size(); ++i)
	{
		m_splinePosArray.emplace_back(move_data[i].transform.GetPosWorld());
	}
	m_splinePosArray.emplace_back(move_data[move_data.size() - 1].transform.GetPosWorld());
}

bool MovieCamera::IsStart()
{
	return m_startFlag;
}

bool MovieCamera::IsFinish()
{
	return m_finishFlag;
}