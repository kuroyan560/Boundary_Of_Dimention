#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"
#include"../Player/CollisionDetectionOfRayAndMesh.h"
#include"../TimeScaleMgr.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//�����Ă��邩�̔���R���s���[�g�p�C�v���C������
	{
		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "����̌��ʂ��i�[����o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "���̍��W�z��o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�����������Ă���͈͂̃}�b�v"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�J�������"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�v���C���[���"),
		};

		//�����Ă��邩
		auto cs_checkBright = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_CheckBright.hlsl", "CheckBright", "cs_6_4");
		m_cPipeline[CHECK_IS_BRIGHT] = D3D12App::Instance()->GenerateComputePipeline(cs_checkBright, rootParam, { WrappedSampler(true,true) });
	}

	//�����ʒu�v�Z�R���s���[�g�p�C�v���C������
	{
		//���[�g�p�����[�^
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "����̌��ʂ��i�[����o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "���̍��W�z��o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "���[���h���W"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�@���}�b�v"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�����������Ă���͈͂̃}�b�v"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�萔�o�b�t�@"),
		};

		//�����ʒu����p�p�C�v���C��
		auto cs_searchPlant = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_SearchPlantPos.hlsl", "SearchPlantPos", "cs_6_4");
		m_cPipeline[SEARCH_PLANT_POS] = D3D12App::Instance()->GenerateComputePipeline(cs_searchPlant, rootParam, { WrappedSampler(true,true) });
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
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�����������̃��[���h�s��o�b�t�@�[(StructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�J�������"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�S���ނ�ŋ��ʂ���萔�o�b�t�@"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "���ނ�ȊO�̃g�����X�t�H�[���f�[�^"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "�e�N�X�`��"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "�}�e���A��"),
		};

		//�����_�[�^�[�Q�b�g�`�����
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//�ʏ�`��
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//�G�~�b�V�u�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//�[�x�}�b�v
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//�G�b�W�J���[�}�b�v
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//���ނ�}�b�v
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

	//�v���C���[���p�萔�o�b�t�@
	m_playerInfoBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(PlayerInfo),
		1,
		nullptr,
		"Grass - PlayerInfo - ConstantBuffer");

	//�����ʒu���v�Z����̂Ɏg�p����萔�o�b�t�@
	m_searchPlantPosConstBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(SearchPlantPosConstData),
		1,
		nullptr,
		"Grass - SearchPlantPos - ConstantBuffer");

	//�����ʒu�T���̌��ʂ̊i�[�p�o�b�t�@
	m_searchPlantResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(SearchPlantResult),
		GRASSF_SEARCH_COUNT,
		nullptr,
		"Grass - SearchPlantResult - RWStructuredBuffer");

	//�����Ă��邩�̔��茋�ʂ̊m��o�b�t�@
	m_checkBrightResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(int),
		s_plantGrassMax,
		nullptr,
		"Grass - CheckBrightResult - RWStructuredBuffer");

	//GPU��̑��f�[�^
	m_plantGrassPosArray.reserve(s_plantGrassMax);
	m_plantGrassPosArrayBuffer = D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(KuroEngine::Vec3<float>),
		s_plantGrassMax,
		nullptr,
		"Grass - PlantGrassPosArray - StructuredBuffer");

	//���f��
	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		std::string fileName = "Grass_" + std::to_string(modelIdx) + ".glb";
		m_modelArray[modelIdx] = Importer::Instance()->LoadModel("resource/user/model/", fileName);

		//���̕`��p���[���h�s��z��o�b�t�@
		m_grassWorldMatriciesBuffer[modelIdx] = D3D12App::Instance()->GenerateStructuredBuffer(
			sizeof(GrassInfo),
			s_plantGrassMax,
			nullptr,
			"Grass - GrassIndicies - StructuredBuffer");

		m_grassWorldMatricies[modelIdx].reserve(s_plantGrassMax);
	}
}

void Grass::Init()
{
	using namespace KuroEngine;

	m_plantGrassDataArray.clear();
	m_plantGrassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, bool arg_isPlayerOverheat, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack, KuroEngine::Vec3<float> arg_moveSpeed)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[������GPU�ɑ��M
	PlayerInfo playerInfo;
	playerInfo.m_pos = arg_playerTransform.GetPos();
	playerInfo.m_plantLighrRante = arg_plantInfluenceRange;
	m_playerInfoBuffer->Mapping(&playerInfo);

	//�v���C���[�̍��W��ۑ��B
	m_playerPos = arg_playerTransform.GetPos();

	//����������
	int generateCount = 0;
	//�폜������
	int consumeCount = 0;

	//if (m_plantTimer.IsTimeUp() && 0.01f < KuroEngine::Vec3<float>(m_oldPlayerPos - arg_playerTransform.GetPos()).Length())
	if (0.1f < TimeScaleMgr::s_inGame.GetTimeScale() && !arg_isPlayerOverheat)
	{
		//�g�����X�t�H�[���ɗ�������
		Transform grassTransform;
		grassTransform.SetPos(arg_playerTransform.GetPos());
		grassTransform.SetRotate(arg_playerTransform.GetRotate());
		grassTransform.SetScale({ 1.0f,1.0f,1.0f });

		//�����͂₷�ꏊ���擾�B
		std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> plantData = SearchPlantPos(arg_playerTransform);

		for (int count = 0; count < GRASSF_SEARCH_COUNT; ++count)
		{
			if (plantData[count].m_isSuccess != 1) continue;
			if (plantData[count].m_plantNormal.Length() <= FLT_EPSILON)continue;

			//���̐���
			m_plantGrassDataArray.emplace_back();
			m_plantGrassDataArray.back().m_pos = plantData[count].m_plantPos;
			m_plantGrassDataArray.back().m_normal = plantData[count].m_plantNormal;
			m_plantGrassDataArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;

			//0-10�̗����𐶐����A�K��l�ȉ��������瑐�𐶂₷�B
			int random = KuroEngine::GetRand(10);
			const int GRASS_VALUE = 8;
			if (random < GRASS_VALUE) {
				random = KuroEngine::GetRand(2);
			}
			else {
				random = KuroEngine::GetRand(3, s_modelNumMax);
			}

			m_plantGrassDataArray.back().m_modelIdx = std::clamp(random, 0, s_modelNumMax - 1);



			m_plantGrassDataArray.back().m_appearY = 0.1f;			//0�Ő��₷�Ƃ��������Ă��܂��B
			m_plantGrassDataArray.back().m_appearYTimer = 0.1f;
			m_plantGrassDataArray.back().m_isCheckNear = false;

			//�����������̃J�E���g
			generateCount++;
		}

		//������B
		if (s_plantGrassMax < m_plantGrassDataArray.size())
		{
			KuroEngine::AppearMessageBox("Grass : Update() ���s", "�����ł������𒴂�����");
			exit(1);
		}

		m_plantTimer.Reset(0);
	}
	m_plantTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	m_oldPlayerPos = arg_playerTransform.GetPos();

	//�A�������ނ�̍X�V
	if (!m_plantGrassDataArray.empty())
	{
		std::vector<RegisterDescriptorData>descData =
		{
			{m_checkBrightResultBuffer,UAV},
			{m_plantGrassPosArrayBuffer,SRV},
			{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
			{arg_cam.lock()->GetBuff(),CBV},
			{m_playerInfoBuffer,CBV},
		};

		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[CHECK_IS_BRIGHT],
			{ static_cast<int>(m_plantGrassDataArray.size()),1,1 },
			descData);

		for (auto& grassIdx : m_plantGrassDataArray) {

			//����ł����珈�����΂��B
			if (grassIdx.m_isDead) continue;

			//�߂��ɑ����Ȃ������m�F����������͂̑����`�F�b�N�B
			if (!grassIdx.m_isCheckNear) {

				//�d�Ȃ��Ă��鑐���폜�B
				for (auto& nearGrass : m_plantGrassDataArray)
				{

					if (nearGrass.m_isDead) continue;
					if (static_cast<int>(&grassIdx - &m_plantGrassDataArray[0]) == static_cast<int>(&nearGrass - &m_plantGrassDataArray[0])) continue;

					float DEADLINE = 1.0f;

					//���������߂�B
					float distance = KuroEngine::Vec3<float>(grassIdx.m_pos - nearGrass.m_pos).Length();
					if (DEADLINE < distance) continue;

					nearGrass.m_isDead = true;

				}

				grassIdx.m_isCheckNear = true;

			}

			auto grass = grassIdx;

			//���̃C�[�W���O�̍X�V����
			UpdateGrassEasing(grass, static_cast<int>(&grassIdx - &m_plantGrassDataArray[0]), arg_moveSpeed);

			//���̓����蔻��
			GrassCheckHit(grass, arg_nowStage);

			//���Ƀg�����X�t�H�[����K���B
			//Index��-1�������珈�����΂��B
			if (grass.m_terrianIdx != -1 && 0 < arg_nowStage.lock()->GetGimmickArray().size()) {

				auto terrian = arg_nowStage.lock()->GetGimmickArray().begin();
				std::advance(terrian, grass.m_terrianIdx);

				auto move = terrian->get()->GetMoveAmount();
				grass.m_pos += move;

			}

			if (grass.m_isDead) {
				consumeCount++;
			}

			grassIdx = grass;
		}

		auto result = std::remove_if(m_plantGrassDataArray.begin(), m_plantGrassDataArray.end(), [](GrassData grass)
			{
				return grass.m_isDead;
			});
		consumeCount = static_cast<int>(std::distance(result, m_plantGrassDataArray.end()));
		m_plantGrassDataArray.erase(result, m_plantGrassDataArray.end());
	}


	//if (consumeCount || generateCount)
	//{
	for (auto& worldMatricies : m_grassWorldMatricies)worldMatricies.clear();
	m_plantGrassPosArray.clear();

	Transform grassTransform;
	for (auto& grassIdx : m_plantGrassDataArray)
	{
		auto& grass = grassIdx;

		if (grass.m_appearY != 1.0f) {
			int a = 0;
		}

		float POSITION_SCALE = 3.0f;
		float appear = (-(1.0f - grass.m_appearY) * POSITION_SCALE);
		grassTransform.SetPos(grass.m_pos + grass.m_normal * appear);
		grassTransform.SetUp(grass.m_normal);
		GrassInfo info;
		info.m_worldMat = grassTransform.GetMatWorld();
		info.m_grassTimer = sinf(grassIdx.m_sineTimer) * grassIdx.m_sineLength;
		m_grassWorldMatricies[grass.m_modelIdx].emplace_back(info);
		m_plantGrassPosArray.emplace_back(grass.m_pos + grass.m_normal * appear);
	}

	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		m_grassWorldMatriciesBuffer[modelIdx]->Mapping(m_grassWorldMatricies[modelIdx].data(), static_cast<int>(m_grassWorldMatricies[modelIdx].size()));
	}
	m_plantGrassPosArrayBuffer->Mapping(m_plantGrassPosArray.data(), static_cast<int>(m_plantGrassPosArray.size()));
	//}
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
				{m_grassWorldMatriciesBuffer[modelIdx],SRV},
				{arg_cam.GetBuff(),CBV},
				{mesh.material->buff,CBV},
				{m_playerInfoBuffer,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
			};

			KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				descData,
				0,
				true,
				static_cast<int>(m_grassWorldMatricies[modelIdx].size()));
		}
	}
}

std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> Grass::SearchPlantPos(KuroEngine::Transform arg_playerTransform)
{
	using namespace KuroEngine;

	//���茋�ʂ̏�����
	auto checkResultPtr = m_searchPlantResultBuffer->GetResource()->GetBuffOnCpu<SearchPlantResult>();
	checkResultPtr->m_plantPos = KuroEngine::Vec3<float>();

	//�K�v�ȃf�[�^�𑗐M
	SearchPlantPosConstData constData;
	constData.m_grassCount = static_cast<int>(m_plantGrassDataArray.size());
	constData.m_seed = KuroEngine::GetRand(0, 100000) / 100.0f;
	m_searchPlantPosConstBuffer->Mapping(&constData);

	//����p�R���s���[�g�p�C�v���C�����s
	//�o�^����f�B�X�N���v�^�̏��z��
	std::vector<RegisterDescriptorData>descData =
	{
		{m_searchPlantResultBuffer,UAV},
		{m_plantGrassPosArrayBuffer,SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_searchPlantPosConstBuffer,CBV},
	};

	//�������Ă��鑐�ނ�̐����擾
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[SEARCH_PLANT_POS],
		{ GRASSF_SEARCH_DISPATCH_X ,GRASSF_SEARCH_DISPATCH_Y,1 },
		descData);

	//���茋�ʂ̎擾
	std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> result;

	checkResultPtr = m_searchPlantResultBuffer->GetResource()->GetBuffOnCpu<SearchPlantResult>();

	for (auto& index : result) {

		int grassIndex = static_cast<int>(&index - &result[0]);
		index.m_isSuccess = checkResultPtr[grassIndex].m_isSuccess;
		index.m_plantNormal = checkResultPtr[grassIndex].m_plantNormal.GetNormal();
		index.m_plantPos = checkResultPtr[grassIndex].m_plantPos + checkResultPtr[grassIndex].m_plantNormal;	//���܂��Ă��܂��̂Ŗ@�������ɏ��������������B

	}

	return result;

}

void Grass::UpdateGrassEasing(Grass::GrassData& arg_grass, int arg_index, KuroEngine::Vec3<float> arg_moveSpeed)
{

	//�R���s���[�g�V�F�[�_�[�Ōv�Z�������ʂ��擾
	bool isBright = m_checkBrightResultBuffer->GetResource()->GetBuffOnCpu<int>()[arg_index];

	if (isBright)
	{
		static const float appearEaseSpeed = 0.05f;
		//�C�[�W���O�^�C�}�[�X�V
		arg_grass.m_appearYTimer = std::min(arg_grass.m_appearYTimer + appearEaseSpeed * TimeScaleMgr::s_inGame.GetTimeScale(), 1.0f);
	}
	else
	{
		static const float deadEaseSpeed = 0.03f;
		//�C�[�W���O�^�C�}�[�X�V
		arg_grass.m_appearYTimer = std::max(arg_grass.m_appearYTimer - deadEaseSpeed * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f);

		//0�ȉ��ɂȂ�����t���O��܂�B
		if (arg_grass.m_appearYTimer <= FLT_EPSILON)
		{
			arg_grass.m_isDead = true;
		}
	}

	//���̑��x�B
	const float WIND_SPEED = 0.05f;
	arg_grass.m_sineTimer += WIND_SPEED;

	//�����v���C���[�̎��͂ɂ���A�v���C���[�������Ă�����K�T�K�T������B
	const float GRASS_MOVE = 0.4f;
	const float GRASS_DEADLINE = 5.0f;
	float distance = KuroEngine::Vec3<float>(arg_grass.m_pos - m_playerPos).Length();
	if(distance < GRASS_DEADLINE && 0.1f < arg_moveSpeed.Length()){
		arg_grass.m_sineTimer += GRASS_MOVE;
	}


	//���_�t�߂̑��͋����폜�B
	if (arg_grass.m_pos.Length() < 1.0f) {
		arg_grass.m_isDead = true;
	}

	//�C�[�W���O�ʂ����߂�
	if (1.0f < arg_grass.m_appearY)
	{
		arg_grass.m_appearY += (1.0f - arg_grass.m_appearY) / 10.0f;
	}
	else
	{
		arg_grass.m_appearY = arg_grass.m_appearYTimer;
	}

}

void Grass::GrassCheckHit(Grass::GrassData& arg_grass, const std::weak_ptr<Stage>arg_nowStage) {

	if (!arg_grass.m_isCheckGround) {

		//�������Ƀ��C���΂��āA�������������ꂾ������e�q�֌W�����ԁB
		int terrianIdx = 0;
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//��������łȂ�
			if (terrian->GetType() != StageParts::MOVE_SCAFFOLD) {
				++terrianIdx;
				continue;
			}

			//��������Ƃ��ăL���X�g
			auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);
			//���f�����擾
			auto model = terrian->GetModel();

			//���b�V���𑖍�
			for (auto& modelMesh : model.lock()->m_meshes)
			{

				//�����蔻��p���b�V��
				auto checkHitMesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_grass.m_pos, -arg_grass.m_normal, checkHitMesh);

				//�������Ă�����
				if (output.m_isHit && 0 < output.m_distance && output.m_distance <= 1.0f) {

					//�e�q�֌W����������B
					arg_grass.m_terrianIdx = terrianIdx;

				}
			}
			++terrianIdx;
		}
		arg_grass.m_isCheckGround = true;

	}

}