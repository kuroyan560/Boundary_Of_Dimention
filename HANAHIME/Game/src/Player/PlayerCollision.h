#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/StageParts.h"
#include"Render/RenderObject/Light.h"
#include"../Plant/GrowPlantLight.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;
class Player;

struct PlayerCollision {

	Player* m_refPlayer;	//�v���C���[�̎Q��

	//�R�ɓ������Ă��邩�H���̓��[�J���ϐ��B
	std::array<bool, 4> m_isHitCliff;
	std::array<std::vector<KuroEngine::Vec3<float>>, 4> m_impactPoint;
	KuroEngine::Vec3<float> m_impactPointBuff;	//���W�ꎞ�ۑ��p

	//�����蔻��p�̃��C�̒����B
	float m_checkUnderRayLength;
	float m_checkCliffRayLength;

	//���˂��郌�C��ID
	enum class RAY_ID {

		GROUND,	//�n��������Ĕ�΂����C�B�ݒu����Ŏg�p����B
		AROUND,	//���͂Ɍ������Ĕ�΂����C�B�ǂ̂ڂ蔻��Ŏg�p����B
		CLIFF,	//�R�ۂ̉����߂��p�̃��C�B���̕ǂ������Ă���̉����߂�������s���B
		CHECK_GIMMICK,	//�M�~�b�N�ɏ���Ă��邩����p�BonGimmick��true�ɂ���B�����߂��͂��Ȃ��B
		CHECK_DEATH,	//���S�m�F�p
		CHECK_CLIFF,	//�R�`�F�b�N�p
		CHECK_IVY,

	};
	//���C�̕���ID
	enum class RAY_DIR_ID {
		RIGHT,
		LEFT,
		FRONT,
		BEHIND,
		TOP,
		BOTTOM,
	};

	//�Փ˓_�p�\����
	struct ImpactPointData {
		KuroEngine::Vec3<float> m_impactPos;
		KuroEngine::Vec3<float> m_normal;
		bool m_isActive;
		bool m_isFastJump;	//�Ԃ����Ă���b�����ăW�����v����̂ł͂Ȃ��A�����ɔ�я��t���O�B�����ɏ�肽���I�u�W�F�N�g������ꍇ�A�����true�ɂ���B
		bool m_isAppearWall;
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal), m_isActive(true), m_isFastJump(false), m_isAppearWall(false) {};
	};

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo = nullptr);

	//CastRay�ɓn���f�[�^�p�\����
	struct CastRayArgument {
		std::weak_ptr<StageParts> m_stage;			//�X�e�[�W
		std::vector<TerrianHitPolygon> m_mesh;		//������s���Ώۂ̃��b�V��
		std::vector<ImpactPointData> m_impactPoint;	//�O�㍶�E�̃��C�̓��������n�_�B
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
		StageParts::STAGE_PARTS_TYPE m_stageType;
		std::array<bool, 6> m_checkDeathCounter;
		std::array<bool, 4> m_checkHitAround;
	};

	//�S�ẴI�u�W�F�N�g�𑖍�
	template <typename Func>
	void CheckHitAllObject(Func arg_func, KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage);

	/// <summary>
	/// ���C�𔭎˂��A���̌�̈�A�̏������܂Ƃ߂��֐�
	/// </summary>
	/// <param name="arg_rayCastPos"> �L�����N�^�[�̍��W </param>
	/// <param name="arg_rayCastPos"> ���C�̎ˏo�n�_ </param>
	/// <param name="arg_rayDir"> ���C�̎ˏo���� </param>
	/// <param name="arg_rayLength"> ���C�̒��� </param>
	/// <param name="arg_collisionData"> �������܂Ƃ߂��\���� </param>
	/// <param name="arg_rayID"> ���C�̎�� </param>
	bool CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID = RAY_DIR_ID::RIGHT);

	//�����蔻��
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage);

	//���͗p�̓����蔻��
	void CheckDeath(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment);
	void CheckCliff(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);
	void CheckCanJump(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);

	//�W�b�v���C���Ƃ̓����蔻��
	void CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage);

	//�M�~�b�N�ɂ��ړ����I��点��B
	void FinishGimmickMove();

private:

	/*===== �ȉ��֐��|�C���^�p =====*/

	//�����蔻��_(�R �l�����牺�����Ƀ��C���΂��B)
	void CheckHitCliff_Under(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage);
	void CheckHitCliff_SearchImpactPoint(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage);
	void CheckHitDeath_Around(KuroEngine::Vec3<float>& arg_newPos, const  KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage);
	void CheckHitAround_Around(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage);

};
