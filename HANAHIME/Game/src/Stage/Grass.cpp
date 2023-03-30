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

	//��====================================
		//���u���̑��u���b�N���f��
	m_grassBlockModel = Importer::Instance()->LoadModel("resource/user/model/", "GrassBlock.gltf");
	//=====================================

	//�R���s���[�g�p�C�v���C������
	{
		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�����������̃o�b�t�@(RWStructuredBuffer)"),
		};

		//�������p�p�C�v���C��
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Init", "cs_6_4");
		m_cPipeline[INIT] = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, {WrappedSampler(true,true)});

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "��������\��̃X�^�b�N�����C�j�V�����C�U�z��o�b�t�@�[(StructuredBuffer)");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�S���ނ�ŋ��ʂ���萔�o�b�t�@");

		//�����p�p�C�v���C��
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Appear", "cs_6_4");
		m_cPipeline[GENERATE] = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, {WrappedSampler(true,true)});

		//�X�V�p�p�C�v���C��
		auto cs_update = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Update", "cs_6_4");
		m_cPipeline[UPDATE] = D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, {WrappedSampler(true,true)});

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "����̌��ʂ��i�[����o�b�t�@(RWStructuredBuffer)");
		//����p�p�C�v���C��
		auto cs_check = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Check", "cs_6_4");
		m_cPipeline[CHECK_AROUND] = D3D12App::Instance()->GenerateComputePipeline(cs_check, rootParam, { WrappedSampler(true,true) });
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
		//�e�N�X�`���o�b�t�@�p���[�g�p�����[�^�ݒ�
		for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)
		{
			rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�e�N�X�`�����");
		}

		//�����_�[�^�[�Q�b�g�`�����
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//�ʏ�`��
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//�G�~�b�V�u�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//�[�x�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//�m�[�}���}�b�v
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//�G�b�W�J���[�}�b�v
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
		1,
		&checkResultInit,
		"Grass - CheckResult - RWStructuredBuffer");

	//�e�N�X�`��
	m_texBuffer[0] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
	m_texBuffer[1] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[2] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter03.png");
	m_texBuffer[3] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter04.png");
	m_texBuffer[4] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
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
			});
	}

	//���[���h�s��z�񏉊���
	m_grassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[���ɗ�������
	Transform grassTransform;
	grassTransform.SetPos(arg_playerPos);
	grassTransform.SetRotate(arg_playerRotate);
	grassTransform.SetScale({ 1.0f,1.0f,1.0f });

	//�g�����X�t�H�[������GPU�ɑ��M
	TransformCBVData transformData;
	transformData.m_playerPos = arg_playerPos;
	transformData.m_playerUp = grassTransform.GetUp();
	transformData.m_camPos = { arg_camTransform.GetMatWorld().r[3].m128_f32[0],arg_camTransform.GetMatWorld().r[3].m128_f32[1],arg_camTransform.GetMatWorld().r[3].m128_f32[2] };
	m_otherTransformConstBuffer->Mapping(&transformData);

	//�v���C���[���ړ����� and ����ɑ����Ȃ��B
	bool isMovePlayer = !((arg_playerPos - m_oldPlayerPos).Length() < 0.1f);
	if (isMovePlayer && !IsGrassAround(arg_playerPos))
	{
		if (m_plantTimer.IsTimeUp())
		{
			PlantGrassBlock(grassTransform, arg_grassPosScatter, arg_waterPaintBlend);
			m_plantTimer.Reset(3);
		}
		m_plantTimer.UpdateTimer();
	}

	m_oldPlayerPos = arg_playerPos;

	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
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

	//�萔�o�b�t�@1�̑��̗h�����X�V�B
	m_constData.m_sineWave += 0.02f;
	//�萔�o�b�t�@1��GPU�ɓ]���B
	m_constBuffer->Mapping(&m_constData);

}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	if (s_instanceMax <= static_cast<int>(m_grassWorldMatArray.size()))
	{
		KuroEngine::AppearMessageBox("Grass : Draw() ���s", "�C���X�^���X�̏�������������");
		exit(1);
	}
	if (m_grassWorldMatArray.empty())return;

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

	//�A�������ނ�̃J�E���g�擾
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();
	KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuffer,
		descData,
		0.0f,
		true,
		plantGrassCount);

	m_drawParam = IndividualDrawParameter::GetDefault();
	//�}�X�N���C���[�ɕ`�����ސݒ�ɂ���
	m_drawParam.m_drawMask = 1;

	//BasicDraw::Instance()->InstancingDraw(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_grassBlockModel,
	//	m_grassWorldMatArray,
	//	m_drawParam,
	//	false,
	//	AlphaBlendMode_Trans);
}

void Grass::PlantGrassBlock(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	//�~�����𐶂₷����
	m_grassWorldMatArray.push_back(arg_transform.GetMatWorld());

	//�|���𐶐��B
	for (int count = 0; count < 3; ++count) {
			Plant(arg_transform, arg_grassPosScatter, arg_waterPaintBlend);
	}
}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	KuroEngine::Vec3<float> pos = arg_transform.GetPos();

	//���𐶂₷�ʒu�������_���ŎU�炷�B
	KuroEngine::Vec3<float>posScatter = arg_transform.GetRight() * KuroEngine::GetRand(-arg_grassPosScatter.x, arg_grassPosScatter.x);
	posScatter += arg_transform.GetFront() * KuroEngine::GetRand(-arg_grassPosScatter.y, arg_grassPosScatter.y);

	//�C�j�V�����C�U�̃X�^�b�N
	m_grassInitializerArray.emplace_back();
	m_grassInitializerArray.back().m_posScatter = posScatter;
	//�Ƃ肠���������Ńe�N�X�`������
	//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
	m_grassInitializerArray.back().m_texIdx = KuroEngine::GetRand(3 - 1);
	m_grassInitializerArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;

	arg_waterPaintBlend.DropMaskInk(pos + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}

bool Grass::IsGrassAround(const KuroEngine::Vec3<float> arg_playerPos)
{
	using namespace KuroEngine;

	//�A�������ނ�̃J�E���g�擾
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//���������ĂȂ��̂Ŕ���̕K�v�Ȃ�
	if (!plantGrassCount)return false;

	auto checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();

	//���茋�ʂ̏�����
	auto initializer = *checkResultPtr;
	initializer.m_aroundGrassCount = 0;
	m_checkResultBuffer->Mapping(&initializer);

	//����p�R���s���[�g�p�C�v���C�����s
	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer,SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
		{m_checkResultBuffer,UAV}
	};

	//�������Ă��鑐�ނ�̐����擾
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[CHECK_AROUND],
		{ plantGrassCount ,1,1 },
		descData);

	//���茋�ʂ̎擾
	return 0 < checkResultPtr->m_aroundGrassCount;
}