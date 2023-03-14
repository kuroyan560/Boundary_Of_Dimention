#pragma once
//エンジン本体==========================================================
#include"KuroEngine.h"
#include<string>
#include<memory>
#include<map>

#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/AudioApp.h"
#include"FrameWork/ImguiApp.h"
#include"FrameWork/Importer.h"
#include"Render/GraphicsManager.h"

namespace KuroEngine
{
	class Fps;

	//シーンの基底クラス
	class BaseScene
	{
	protected:
		BaseScene() {}
		virtual void OnInitialize() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnDraw() = 0;
		virtual void OnImguiDebug() = 0;
		virtual void OnFinalize() = 0;
	public:
		virtual ~BaseScene() {};
		void Initialize();
		void Update();
		void Draw();
		void ImguiDebug();
		void Finalize();
	};

	//シーン遷移（基本は継承して利用するが、１フレ切り替え遷移は継承なしで利用できる）
	class SceneTransition
	{
	protected:
		bool m_nowTrans = false;
		virtual void OnStart() { }
	public:
		virtual ~SceneTransition() {}
		//シーン遷移スタート
		void Start() { m_nowTrans = true; OnStart(); }
		//シーンを切り替えるタイミングでtrueを返す
		virtual bool Update() { m_nowTrans = false; return true; }
		//シーン遷移描画
		virtual void Draw() {}
		//シーン遷移が完全に終了したか
		bool Finish() { return !m_nowTrans; }
	};

	//アプリの初期設定
	struct EngineOption
	{
		//ウィンドウ関連
		std::string m_windowName;	//ウィンドウ名
		Vec2<int>m_windowSize;	//ウィンドウサイズ
		bool m_fullScreen = false;	//フルスクリーン設定
		bool m_showCursor = true;	//マウス表示設定
		bool m_clipCursor = true;	//マウスをウィンドウ領域内に限定するか
		const wchar_t* m_iconPath = nullptr;	//ウィンドウアイコンファイル名

		//バックバッファ関連
		Color m_backBuffClearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
		bool m_useHDR = false;	//HDRを使うか（HDR対応していない場合使用しない）

		//フレームレート
		int m_frameRate = 60;
	};

	//エンジン本体
	class KuroEngineDevice : public D3D12AppUser
	{
	private:
		static KuroEngineDevice* s_instance;
	public:
		static KuroEngineDevice* Instance()
		{
			assert(s_instance != nullptr);
			return s_instance;
		}

	private:
		//初期化されているか
		bool m_invalid = true;

		//XAudio
		HMODULE m_xaudioLib;
		//ウィンドウアプリ
		std::unique_ptr<WinApp>m_winApp;
		//D3D12アプリ
		std::unique_ptr<D3D12App>m_d3d12App;
		//入力管理
		std::unique_ptr<UsersInput>m_usersInput;
		//音声関連アプリ
		std::unique_ptr<AudioApp>m_audioApp;
		//Imguiアプリ
		std::unique_ptr<ImguiApp>m_imguiApp;
		//インポーター
		std::unique_ptr<Importer>m_importer;

		//グラフィックスマネージャ（レンダリング統括、Zソートなどを行う）
		GraphicsManager m_gManager;
		std::shared_ptr<ConstantBuffer>m_parallelMatProjBuff;

		//シーンリスト
		std::map<std::string, BaseScene*>m_scenes;
		std::string m_nowScene;	//現在のシーンキー
		std::string m_nextScene = "";	//次のシーンキー
		SceneTransition* m_nowSceneTransition;	//現在セットされているシーン遷移

		//終了フラグ
		bool m_end = false;

		void Render()override;
	public:
		KuroEngineDevice()
		{
			assert(s_instance == nullptr);
			s_instance = this;
		}
		~KuroEngineDevice();

		/// <summary>
		/// エンジン初期化
		/// </summary>
		/// <param name="Option">起動設定</param>
		/// <returns>エラー発生</returns>
		bool Initialize(const EngineOption& Option);
		void SetSceneList(const std::map<std::string, BaseScene*>& SceneList, const std::string& AwakeSceneKey);
		void Update();
		void Draw();
		bool End() { return m_end; }

		//ゲーム終了
		void GameEnd() { m_end = true; }
		//シーンチェンジ
		void ChangeScene(const std::string& SceneKey, SceneTransition* SceneTransition = nullptr)
		{
			if (m_scenes.find(SceneKey) == m_scenes.end())
			{
				printf("The scene key wasn't be found.\n");
				assert(0);
				return;
			}
			m_nextScene = SceneKey;
			m_nowSceneTransition = SceneTransition;
			if (m_nowSceneTransition != nullptr)	m_nowSceneTransition->Start();
		}

		//グラフィックスマネージャゲッタ
		GraphicsManager& Graphics() { return m_gManager; }

		//平行投影行列定数バッファ
		const std::shared_ptr<ConstantBuffer>& GetParallelMatProjBuff()
		{
			return m_parallelMatProjBuff;
		}

	};
}