#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"
#include"../Player/CollisionDetectionOfRayAndMesh.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//�R���s���[�g�p�C�v���C������
	{
		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�����������̃o�b�t�@"),
		};

		//�������p�p�C�v���C��
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Init", "cs_6_4");
		m_cPipeline[INIT] = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "��������\��̃X�^�b�N�����C�j�V�����C�U�z��o�b�t�@�[(StructuredBuffer)");

		//�����p�p�C�v���C��
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Appear", "cs_6_4");
		m_cPipeline[APPEAR] = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "���[���h���W");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�@���}�b�v");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�����������Ă���͈͂̃}�b�v");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�S���ނ�ŋ��ʂ���萔�o�b�t�@");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�폜���鑐�̃J�E���g");

		//�폜�p�p�C�v���C��
		auto cs_disappear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Disappear", "cs_6_4");
		m_cPipeline[DISAPPEAR] = D3D12App::Instance()->GenerateComputePipeline(cs_disappear, rootParam, { WrappedSampler(true,true) });

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
		PipelineInitializeOption pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�V�F�[�_�[���
		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "VSmain", "vs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "PSmain", "ps_6_4");

		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�����������̃o�b�t�@�[(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�S���ނ�ŋ��ʂ���萔�o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�e�N�X�`��"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�}�e���A��"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "���ނ�̃C���f�b�N�X�z��"),
		};

		//�����_�[�^�[�Q�b�g�`�����
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//�ʏ�`��
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//�G�~�b�V�u�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//�[�x�}�b�v
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//�G�b�W�J���[�}�b�v
		};

		//�ݒ����Ƀp�C�v���C������
		m_pipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			pipelineOption,
			shaders,
			ModelMesh::Vertex::GetInputLayout(),
			rootParam,
			renderTargetInfo,
			{ WrappedSampler(true,true) });
	}

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

	//�\�[�g�ƍ폜�����Ŏg��unsigned int �̃o�b�t�@�[
	int initZero = 0;
	m_consumeCountBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(int),
		1,
		&initZero,
		"Grass - ConsumeCount - ConstantBuffer"
	);

	//���茋�ʂ̊i�[�p�o�b�t�@
	std::array<CheckResult, GRASSF_SEARCH_COUNT> checkResultInit;
	m_checkResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(CheckResult),
		GRASSF_SEARCH_COUNT,
		checkResultInit.data(),
		"Grass - CheckResult - RWStructuredBuffer");

	//���f��
	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		std::string fileName = "Grass_" + std::to_string(modelIdx) + ".glb";
		m_modelArray[modelIdx] = Importer::Instance()->LoadModel("resource/user/model/", fileName);

		//���̃C���f�b�N�X�z��o�b�t�@
		m_grassIndiciesBuffer[modelIdx] = D3D12App::Instance()->GenerateStructuredBuffer(
			sizeof(int),
			m_plantGrassMax,
			nullptr,
			"Grass - GrassIndicies - StructuredBuffer");

		m_grassIndicies[modelIdx].reserve(m_plantGrassMax);
	}
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

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[������GPU�ɑ��M
	TransformCBVData transformData;
	transformData.m_playerPos = arg_playerTransform.GetPos();
	transformData.m_playerPlantLightRange = arg_plantInfluenceRange;
	transformData.m_camPos = { arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[0],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[1],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[2] };
	m_otherTransformConstBuffer->Mapping(&transformData);

	//�v���C���[�̍��W��ۑ��B
	m_playerPos = arg_playerTransform.GetPos();

	//if (m_plantTimer.IsTimeUp() && 0.01f < KuroEngine::Vec3<float>(m_oldPlayerPos - arg_playerTransform.GetPos()).Length())
	if (true)
	{
		//�g�����X�t�H�[���ɗ�������
		Transform grassTransform;
		grassTransform.SetPos(arg_playerTransform.GetPos());
		grassTransform.SetRotate(arg_playerTransform.GetRotate());
		grassTransform.SetScale({ 1.0f,1.0f,1.0f });

		Plant(grassTransform, arg_playerTransform);
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
	//�J�����̏�������B
	m_constData.matView = arg_cam.lock()->GetViewMat();
	m_constData.matProjection = arg_cam.lock()->GetProjectionMat();
	m_constData.eye = arg_cam.lock()->GetEye();
	//�萔�o�b�t�@1��GPU�ɓ]���B
	m_constBuffer->Mapping(&m_constData);

	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer, SRV},
	};

	//�A�������ނ�̔z��̃|�C���^�擾
	auto aliveGrassArrayBufferPtr = m_plantGrassBuffer->GetResource()->GetBuffOnCpu<PlantGrass>();
	//�A�������ނ�̃J�E���g�̃|�C���^�擾
	auto plantGrassCountPtr = m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

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
			m_cPipeline[APPEAR],
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

	//�A�������ނ�̍X�V
	if (*plantGrassCountPtr)
	{

		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS), SRV);
		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL_GRASS), SRV);
		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT), SRV);
		descData.emplace_back(m_otherTransformConstBuffer, CBV);
		descData.emplace_back(m_constBuffer, CBV);
		descData.emplace_back(m_consumeCountBuffer, CBV);

		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[UPDATE],
			{ *plantGrassCountPtr,1,1 },
			descData);

		//�������Ƀ��C���΂��āA�������������ꂾ������e�q�֌W�����ԁB
		for (int grassIdx = 0; grassIdx < *plantGrassCountPtr;++grassIdx) 
		{
			auto& index = aliveGrassArrayBufferPtr[grassIdx];

			if (!index.m_isAlive)continue;
			if (index.m_isCheckGround) continue;

			int terrianIdx = 0;
			for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
			{
				//��������łȂ�
				if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

				//��������Ƃ��ăL���X�g
				auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);
				//���f�����擾
				auto model = terrian->GetModel();

				//���b�V���𑖍�
				for (auto& modelMesh : model.lock()->m_meshes)
				{

					//�����蔻��p���b�V��
					auto checkHitMesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

					CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(index.m_worldPos, -index.m_normal, checkHitMesh);

					//�������Ă�����
					if (output.m_isHit && 0 < output.m_distance && output.m_distance <= 1.0f) {

						//�e�q�֌W����������B
						index.m_terrianIdx = terrianIdx;

					}


				}
				++terrianIdx;
			}
			index.m_isCheckGround = true;

			//���Ƀg�����X�t�H�[����K���B
			//Index��-1�������珈�����΂��B
			if (index.m_terrianIdx == -1) continue;

			auto terrian = arg_nowStage.lock()->GetGimmickArray().begin();
			std::advance(terrian, index.m_terrianIdx);

			index.m_worldPos += terrian->get()->GetMoveAmount();

			//���_�t�߂ɐ����鑐�͍폜
			//�U�����ŁA����͈͓̔���������AppearY�𒴂ł�������B
			if (arg_isAttack && (arg_playerTransform.GetPos() - index.m_worldPos).Length() < arg_plantInfluenceRange) {
				index.m_appearY = 10.0f;
			}

			if (5.0f <= index.m_worldPos.Length()) continue;
			index.m_isAlive = false;
		}

		//GPU��Ń\�[�g�����瑐������o�O�u�����̂�CPU���Ń\�[�g�B�����Ώ�����I�I�I�I�I�I�I�I�I�I�I
		std::vector<PlantGrass>aliveGrassArray;
		//GPU��̑��f�[�^��vector�ɂ����B
		aliveGrassArray.resize(*plantGrassCountPtr);
		std::memcpy(aliveGrassArray.data(), aliveGrassArrayBufferPtr, sizeof(PlantGrass)* (*plantGrassCountPtr));

		//�폜���鐔�̃J�E���g
		int consumeCount = static_cast<int>(std::count_if(aliveGrassArray.begin(), aliveGrassArray.end(), [](PlantGrass& grass)
			{
				return grass.m_isAlive == 0;
			}));

		//�t���O��false�ɂȂ��������Ō���փ\�[�g
		std::sort(aliveGrassArray.begin(), aliveGrassArray.end(), [](PlantGrass& a, PlantGrass& b) {
			return a.m_isAlive > b.m_isAlive;
			});

		//GPU�ɑ��M
		m_plantGrassBuffer->Mapping(aliveGrassArray.data(), *plantGrassCountPtr);
		m_consumeCountBuffer->Mapping(&consumeCount);

		//����ł�����̂��폜
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[DISAPPEAR],
			{ 1,1,1 },
			descData);

		//���ނ�̃C���f�b�N�X�̔z��
		for (auto& indicies : m_grassIndicies)
		{
			indicies.clear();
		}
		//���f�����Ƃ̑��̃C���f�b�N�X�z��쐬
		for (int grassIdx = 0; grassIdx < *plantGrassCountPtr; ++grassIdx)
		{
			auto& grass = aliveGrassArrayBufferPtr[grassIdx];
			m_grassIndicies[grass.m_modelIdx].emplace_back(grassIdx);
		}
		//�C���f�b�N�X�z����𑗐M
		for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
		{
			m_grassIndiciesBuffer[modelIdx]->Mapping(m_grassIndicies[modelIdx].data(), static_cast<int>(m_grassIndicies[modelIdx].size()));
		}
	}
}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, float arg_plantInfluenceRange, bool arg_isAttack)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_pipeline);

	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		for (auto& mesh : m_modelArray[modelIdx]->m_meshes)
		{
			std::vector<RegisterDescriptorData>descData =
			{
				{m_plantGrassBuffer,UAV},
				{arg_cam.GetBuff(),CBV},
				{m_constBuffer,CBV},
				{m_otherTransformConstBuffer,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
				{mesh.material->buff,CBV},
				{m_grassIndiciesBuffer[modelIdx],SRV},
			};

			KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				descData,
				0,
				true,
				static_cast<int>(m_grassIndicies[modelIdx].size()));
		}
	}
}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Transform arg_playerTransform)
{

	//�����͂₷�ꏊ���擾�B
	std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> plantData = SearchPlantPos(arg_playerTransform);

	for (int count = 0; count < GRASSF_SEARCH_COUNT; ++count)
	{

		if (plantData[count].m_isSuccess != 1) continue;

		//�C�j�V�����C�U�̃X�^�b�N
		m_grassInitializerArray.emplace_back();
		m_grassInitializerArray.back().m_pos = plantData[count].m_plantPos;
		m_grassInitializerArray.back().m_up = plantData[count].m_plantNormal;
		//�Ƃ肠���������Ńe�N�X�`������
		//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
		m_grassInitializerArray.back().m_modelIdx = KuroEngine::GetRand(3 - 1);
		m_grassInitializerArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;
	}

	//�C���N�}�X�N�𗎂Ƃ�
	//arg_waterPaintBlend.DropMaskInk(arg_transform.GetPos() + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}

std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> Grass::SearchPlantPos(KuroEngine::Transform arg_playerTransform)
{
	using namespace KuroEngine;

	//�A�������ނ�̃J�E���g�擾
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//���茋�ʂ̏�����
	auto checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();
	checkResultPtr->m_plantPos = KuroEngine::Vec3<float>();

	//�K�v�ȃf�[�^�𑗐M
	auto transformCBVPtr = m_otherTransformConstBuffer->GetResource()->GetBuffOnCpu<TransformCBVData>();
	transformCBVPtr->m_seed = KuroEngine::GetRand(0, 100000) / 100.0f;
	transformCBVPtr->m_grassCount = plantGrassCount;
	transformCBVPtr->m_playerPos = arg_playerTransform.GetPosWorld();

	//����p�R���s���[�g�p�C�v���C�����s
	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer,SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
		{m_consumeCountBuffer,CBV},
		{m_checkResultBuffer,UAV}
	};

	//�������Ă��鑐�ނ�̐����擾
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[SEARCH_PLANT_POS],
		{ GRASSF_SEARCH_DISPATCH_X ,GRASSF_SEARCH_DISPATCH_Y,1 },
		descData);

	//���茋�ʂ̎擾
	std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> result;

	checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();

	for (int index = 0; index < GRASSF_SEARCH_COUNT; ++index) {

		result[index].m_isSuccess = checkResultPtr[index].m_isSuccess;
		result[index].m_plantNormal = checkResultPtr[index].m_plantNormal.GetNormal();
		result[index].m_plantPos = checkResultPtr[index].m_plantPos + checkResultPtr[index].m_plantNormal;	//���܂��Ă��܂��̂Ŗ@�������ɏ��������������B

	}

	return result;

}