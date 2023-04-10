#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//�R���s���[�g�p�C�v���C������
	{
		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�����������̃o�b�t�@(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"���[���h���W"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�@���}�b�v"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�����������Ă���͈͂̃}�b�v"),
		};

		//�������p�p�C�v���C��
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Init", "cs_6_4");
		m_cPipeline[INIT] = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "��������\��̃X�^�b�N�����C�j�V�����C�U�z��o�b�t�@�[(StructuredBuffer)");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�S���ނ�ŋ��ʂ���萔�o�b�t�@");

		//�����p�p�C�v���C��
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Appear", "cs_6_4");
		m_cPipeline[GENERATE] = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, { WrappedSampler(true,true) });

		//�X�V�p�p�C�v���C��
		auto cs_update = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Update", "cs_6_4");
		m_cPipeline[UPDATE] = D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "����̌��ʂ��i�[����o�b�t�@(RWStructuredBuffer)");
		//����p�p�C�v���C��
		auto cs_check = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "SearchPlantPos", "cs_6_4");
		m_cPipeline[SEARCH_PLANT_POS] = D3D12App::Instance()->GenerateComputePipeline(cs_check, rootParam, { WrappedSampler(true,true) });
	}

	//�`��p�O���t�B�b�N�X�p�C�v���C���p�C�v���C������
	{
		//�p�C�v���C���ݒ�
		PipelineInitializeOption pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//�V�F�[�_�[���
		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "VSmain", "vs_6_4");
		shaders.m_gs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "GSmain", "gs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "PSmain", "ps_6_4");

		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�����������̃o�b�t�@�[(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�S���ނ�ŋ��ʂ���萔�o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^"),
		};
		//�e�N�X�`���o�b�t�@�p���[�g�p�����[�^�ݒ� + �@���e�N�X�`��
		for (int texIdx = 0; texIdx < s_textureNumMax * 2.0f; ++texIdx)
		{
			rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�e�N�X�`�����");
		}

		//�����_�[�^�[�Q�b�g�`�����
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//�ʏ�`��
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//�G�~�b�V�u�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//�[�x�}�b�v
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//�G�b�W�J���[�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//���ނ�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//�m�[�}���}�b�v
		};

		//�ݒ����Ƀp�C�v���C������
		m_pipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			pipelineOption,
			shaders,
			{
				{InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT) },
			},
			rootParam,
			renderTargetInfo,
			{ WrappedSampler(true,true) });
	}

	//���_�o�b�t�@
	Vec3<float>vertex = { 0,0,0 };
	m_vertBuffer = D3D12App::Instance()->GenerateVertexBuffer(
		sizeof(vertex),
		1,
		&vertex,
		"Grass - VertexBuffer");

	//�v���C���[�̃g�����X�t�H�[�����p�萔�o�b�t�@
	m_otherTransformConstBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(TransformCBVData),
		1,
		nullptr,
		"Grass - PlayerTransform - ConstantBuffer");

	//�s��ȊO�̃f�[�^�p�\����
	m_constBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(CBVdata),
		1,
		&m_constData,
		"Grass - Common - ConstantBuffer");

	//�����\��̑��ނ�̃C�j�V�����C�U���X�^�b�N���Ă����o�b�t�@
	m_stackGrassInitializerBuffer = D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(GrassInitializer),
		GENERATE_MAX_ONCE,
		nullptr,
		"Grass - InitializerArray - StructuredBuffer");

	//�����������ނ�̃o�b�t�@
	D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_plantGrassBuffer, &m_plantGrassCounterBuffer,
		sizeof(PlantGrass),
		m_plantGrassMax,
		nullptr,
		"Grass - PlantGrass - RWStructuredBuffer");

	//���茋�ʂ̊i�[�p�o�b�t�@
	CheckResult checkResultInit;
	m_checkResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(CheckResult),
		PLANT_ONCE_COUNT,
		&checkResultInit,
		"Grass - CheckResult - RWStructuredBuffer");

	//�e�N�X�`��
	m_texBuffer[0] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[1] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[2] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter03.png");
	m_texBuffer[3] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[4] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");

	//�@���e�N�X�`��
	m_normalTexBuffer[0] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02_normal.png");
	m_normalTexBuffer[1] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02_normal.png");
	m_normalTexBuffer[2] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter03_normal.png");
	m_normalTexBuffer[3] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02_normal.png");
	m_normalTexBuffer[4] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02_normal.png");
}

void Grass::Init()
{
	using namespace KuroEngine;

	//�A�������̃J�E���g�擾
	auto plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//���ނ�̏������i�S�����j
	if (plantGrassCount)
	{
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[INIT],
			{ plantGrassCount,1,1 },
			{
				{m_plantGrassBuffer,UAV},
				{BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),SRV},
				{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
				{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
			});
	}

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[������GPU�ɑ��M
	TransformCBVData transformData;
	transformData.m_camPos = { arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[0],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[1],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[2] };
	m_otherTransformConstBuffer->Mapping(&transformData);

	if (m_plantTimer.IsTimeUp())
	{
		//�g�����X�t�H�[���ɗ�������
		Transform grassTransform;
		grassTransform.SetPos(arg_playerTransform.GetPos());
		grassTransform.SetRotate(arg_playerTransform.GetRotate());
		grassTransform.SetScale({ 1.0f,1.0f,1.0f });

		Plant(grassTransform, arg_playerTransform, arg_grassPosScatter, arg_waterPaintBlend);
		m_plantTimer.Reset(0);
	}
	m_plantTimer.UpdateTimer();

	m_oldPlayerPos = arg_playerTransform.GetPos();

	//�萔�o�b�t�@1�̑��̗h�����X�V�B
	m_constData.m_sineWave += 0.02f;
	//���W��ۑ��B
	m_constData.m_playerPos = arg_playerTransform.GetPos() + arg_playerTransform.GetUp() * arg_playerTransform.GetScale().y;
	//�o�ꑬ�x��ݒ�B
	m_constData.m_appearEaseSpeed = 0.1f;
	//�v���C���[�̍��W���擾�B
	m_constData.m_playerPos = arg_playerTransform.GetPos();
	//�萔�o�b�t�@1��GPU�ɓ]���B
	m_constBuffer->Mapping(&m_constData);

	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_stackGrassInitializerBuffer,SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
	};

	//�A�������ނ�̃J�E���g�̃|�C���^�擾
	auto plantGrassCountPtr = m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//�A�������ނ�̍X�V
	if (*plantGrassCountPtr)
	{
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[UPDATE],
			{ *plantGrassCountPtr,1,1 },
			descData);
	}

	//�X�^�b�N���Ă��������ނ�𐶂₷
	if (!m_grassInitializerArray.empty())
	{
		//��x�ɐ����ł���ʂ𒴂��Ă�
		int generateNum = static_cast<int>(m_grassInitializerArray.size());
		if (GENERATE_MAX_ONCE < generateNum)
		{
			AppearMessageBox("Grass : Update() ���s", "��x�ɐ����ł���ʂ𒴂��Ă��");
			exit(1);
		}

		//�C�j�V�����C�U�z��𑗐M
		m_stackGrassInitializerBuffer->Mapping(m_grassInitializerArray.data(), generateNum);

		//����
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[GENERATE],
			{ generateNum,1,1 },
			descData);

		//������B
		if (m_plantGrassMax < *plantGrassCountPtr)
		{
			AppearMessageBox("Grass : Update() ���s", "�����ł������𒴂�����");
			exit(1);
		}

		//�X�^�b�N�����C�j�V�����C�U�����Z�b�g
		m_grassInitializerArray.clear();
	}

}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_pipeline);

	std::vector<RegisterDescriptorData>descData =
	{
			{m_plantGrassBuffer,UAV},
			{arg_cam.GetBuff(),CBV},
			{m_constBuffer,CBV},
			{m_otherTransformConstBuffer,CBV},
	};
	//�e�N�X�`�������Z�b�g
	for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)descData.emplace_back(m_texBuffer[texIdx], SRV);
	for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)descData.emplace_back(m_normalTexBuffer[texIdx], SRV);

	//�A�������ނ�̃J�E���g�擾
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();
	KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuffer,
		descData,
		0.0f,
		true,
		plantGrassCount);
}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Transform arg_playerTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{

	//�����͂₷�ꏊ���擾�B
	std::array<CheckResult, PLANT_ONCE_COUNT> plantData = SearchPlantPos(arg_playerTransform);

	for (int count = 0; count < PLANT_ONCE_COUNT; ++count)
	{

		if (plantData[count].m_isSuccess != 1) continue;

		//�C�j�V�����C�U�̃X�^�b�N
		m_grassInitializerArray.emplace_back();
		m_grassInitializerArray.back().m_pos = plantData[count].m_plantPos;
		m_grassInitializerArray.back().m_up = plantData[count].m_plantNormal;
		//�Ƃ肠���������Ńe�N�X�`������
		//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
		m_grassInitializerArray.back().m_texIdx = KuroEngine::GetRand(3 - 1);
		m_grassInitializerArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;
	}


	//�C���N�}�X�N�𗎂Ƃ�
	//arg_waterPaintBlend.DropMaskInk(arg_transform.GetPos() + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}

std::array<Grass::CheckResult, Grass::PLANT_ONCE_COUNT> Grass::SearchPlantPos(KuroEngine::Transform arg_playerTransform)
{
	using namespace KuroEngine;

	//�A�������ނ�̃J�E���g�擾
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//���茋�ʂ̏�����
	auto checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();
	checkResultPtr->m_plantPos = KuroEngine::Vec3<float>();

	//�K�v�ȃf�[�^�𑗐M
	auto transformCBVPtr = m_otherTransformConstBuffer->GetResource()->GetBuffOnCpu<TransformCBVData>();
	transformCBVPtr->m_seed = KuroEngine::GetRand(0, 1000);
	transformCBVPtr->m_grassCount = plantGrassCount;
	transformCBVPtr->m_plantOnceCount = PLANT_ONCE_COUNT;

	//����p�R���s���[�g�p�C�v���C�����s
	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_stackGrassInitializerBuffer,SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
		{m_checkResultBuffer,UAV}
	};

	//�������Ă��鑐�ނ�̐����擾
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[SEARCH_PLANT_POS],
		{ 1 ,1,1 },
		descData);

	//���茋�ʂ̎擾
	std::array<Grass::CheckResult, Grass::PLANT_ONCE_COUNT> result;

	for (int index = 0; index < PLANT_ONCE_COUNT; ++index) {

		result[index].m_isSuccess = checkResultPtr[index].m_isSuccess;
		result[index].m_plantNormal = checkResultPtr[index].m_plantNormal;
		result[index].m_plantPos = checkResultPtr[index].m_plantPos + checkResultPtr[index].m_plantNormal;	//���܂��Ă��܂��̂Ŗ@�������ɏ��������������B

	}

	return result;

}