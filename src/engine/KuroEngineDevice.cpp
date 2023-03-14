#include"KuroEngineDevice.h"
#include<ctime>
#include"FrameWork/Fps.h"
#include"ForUser/ImguiDebugInterface.h"

KuroEngine::KuroEngineDevice* KuroEngine::KuroEngineDevice::s_instance = nullptr;

void KuroEngine::BaseScene::Initialize()
{
	OnInitialize();
}

void KuroEngine::BaseScene::Update()
{
	OnUpdate();
}

void KuroEngine::BaseScene::Draw()
{
	OnDraw();
}

void KuroEngine::BaseScene::ImguiDebug()
{
	OnImguiDebug();
}

void KuroEngine::BaseScene::Finalize()
{
	OnFinalize();
}

void KuroEngine::KuroEngineDevice::Render()
{
	m_scenes[m_nowScene]->Draw();

	//シーン遷移描画
	if (m_nowSceneTransition != nullptr)
	{
		m_nowSceneTransition->Draw();
	}

	//グラフィックスマネージャのコマンドリスト全実行
	m_gManager.CommandsExcute(m_d3d12App->GetCmdList());

#ifdef _DEBUG
	//Imgui
	m_d3d12App->SetBackBufferRenderTarget();
	m_imguiApp->BeginImgui(m_d3d12App->GetCmdList());
	m_scenes[m_nowScene]->ImguiDebug();

	ImguiDebugInterface::DrawImguiDebugger();

	m_imguiApp->EndImgui(m_d3d12App->GetCmdList());
#endif
}

KuroEngine::KuroEngineDevice::~KuroEngineDevice()
{
	//XAudio2の解放
	FreeLibrary(m_xaudioLib);

	//シーンの削除
	for (auto& scene : m_scenes)
	{
		delete scene.second;
	}
	m_scenes.clear();

	printf("KuroEngineシャットダウン\n");

	s_instance = nullptr;
}

bool KuroEngine::KuroEngineDevice::Initialize(const EngineOption& Option)
{
	if (!m_invalid)
	{
		printf("エラー：KuroEngineは起動済です\n");
		return m_invalid;
	}
	printf("KuroEngineを起動します\n");

	//XAudioの読み込み
	m_xaudioLib = LoadLibrary(L"XAudio2_9.dll");

	//乱数取得用シード生成
	srand(static_cast<unsigned int>(time(NULL)));

	//ウィンドウアプリ生成
	m_winApp = std::make_unique<WinApp>(Option.m_windowName, Option.m_windowSize, Option.m_fullScreen, Option.m_iconPath);

	//D3D12アプリ生成
	m_d3d12App = std::make_unique<D3D12App>(m_winApp->GetHwnd(), Option.m_windowSize, Option.m_useHDR, Option.m_backBuffClearColor);

	//インプット管理アプリ生成
	m_usersInput = std::make_unique<UsersInput>(m_winApp->GetWinClass(), m_winApp->GetHwnd());

	//音声関連アプリ
	m_audioApp = std::make_unique<AudioApp>();

	//Imguiアプリ
	m_imguiApp = std::make_unique<ImguiApp>(m_d3d12App->GetDevice(), m_winApp->GetHwnd());

	//インポーター
	m_importer = std::make_unique<Importer>();

	//FPS固定機能
	Fps::Instance()->LoopInit(Option.m_frameRate);

	//平行投影行列定数バッファ生成
	auto parallelMatProj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, m_winApp->GetExpandWinSize().x, m_winApp->GetExpandWinSize().y, 0.0f, 0.0f, 1.0f);
	m_parallelMatProjBuff = m_d3d12App->GenerateConstantBuffer(sizeof(XMMATRIX), 1, 
		&parallelMatProj);

	//マウスの設定
	ShowCursor(Option.m_showCursor);
	RECT winRect;
	GetWindowRect(m_winApp->GetHwnd(), &winRect);
	if (Option.m_clipCursor)ClipCursor(&winRect);


	printf("KuroEngine起動成功\n");
	m_invalid = false;

	return m_invalid;
}

void KuroEngine::KuroEngineDevice::SetSceneList(const std::map<std::string, BaseScene*>& SceneList, const std::string& AwakeSceneKey)
{
	//シーンリスト移動
	m_scenes = SceneList;
	m_nowScene = AwakeSceneKey;
	m_scenes[m_nowScene]->Initialize();
}

void KuroEngine::KuroEngineDevice::Update()
{
	//FPS更新
	Fps::Instance()->Update();

	//音声関連アプリ更新
	m_audioApp->Update();

	//次のシーンがセットされている
	if (!m_nextScene.empty())
	{
		//シーン切り替えフラグ
		bool sceneChangeFlg = true;

		if (m_nowSceneTransition != nullptr) //シーン遷移中
		{
			//シーン遷移クラスの更新関数は、シーン切り替えのタイミングで true を還す
			sceneChangeFlg = m_nowSceneTransition->Update();

			//シーン遷移終了
			if (m_nowSceneTransition->Finish())
			{
				m_nowSceneTransition = nullptr;
			}
		}

		//シーン切り替え
		if (sceneChangeFlg)
		{
			m_scenes[m_nowScene]->Finalize();	//切り替え前のシーン終了処理
			m_nowScene = m_nextScene;		//シーン切り替え
			m_scenes[m_nowScene]->Initialize();	//切り替え後のシーン初期化処理
			m_nextScene.clear();
		}
	}

	//入力更新
	m_usersInput->Update(m_winApp->GetHwnd(), m_winApp->GetExpandWinSize());

	m_scenes[m_nowScene]->Update();
}

void KuroEngine::KuroEngineDevice::Draw()
{
	m_d3d12App->Render(this);
}