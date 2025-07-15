#include "ToolMain.h"
#include "CameraController.h"
#include "Brush.h"

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"

#include <string>


using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;


Game::Game(ToolMain* ToolFrameworkHead)
    :
    FrameworkHead(ToolFrameworkHead)
    , Camera(std::make_shared<CameraController>(FrameworkHead, SimpleMath::Vector3(0.f, 2.f, -10.f)))
    , HighlightScalar(1.05f, 1.01f, 1.05f)
{
    // Instantiate and register the owner of "DeviceResources."

    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    // Clear the container of scene objects to render.

	m_displayList.clear();

    // 

    //m_world = SimpleMath::Matrix::Identity;
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

std::pair<float, float> Game::GetViewportDepth()
{
    std::pair<float, float> ViewportDepth(0.f, 0.f);

    if (m_deviceResources)
    {
        ViewportDepth.first = m_deviceResources->GetScreenViewport().MinDepth;
        ViewportDepth.second = m_deviceResources->GetScreenViewport().MaxDepth;
    }
    return ViewportDepth;
}

ID3D11DeviceContext* Game::GetDeviceContext()
{
    if (m_deviceResources)
    {
        return m_deviceResources->GetD3DDeviceContext();
    }
    return nullptr;
}


std::shared_ptr<CameraController> Game::GetCamera()
{
    return Camera;
}


double Game::GetDeltaTime()
{
    return m_timer.GetElapsedSeconds();
}

double Game::GetGameTime()
{
    return m_timer.GetTotalSeconds();
}


DisplayObject* Game::Pick()
{
    if (FrameworkHead && m_deviceResources)
    {
        if (InputCommands* InputState = FrameworkHead->GetInputCommands())
        {
            // Project the screen-space cursor coordinates to the near
            // and far planes of the viewing frustrum.

            XMFLOAT2 CursorPosition = InputState->GetCursorPosition();

            const XMVECTOR NearSource = XMVectorSet(CursorPosition.x, CursorPosition.y, 0.0f, 1.0f);
            const XMVECTOR FarSource = XMVectorSet(CursorPosition.x, CursorPosition.y, 1.0f, 1.0f);

            // Retrieve the dimensions of the client window.

            RECT Window;

            GetClientRect(WindowHandle, &Window);


            // The following object is finally returned where the section of the "PickingVector,"
            // from "NearPoint" to the intersection with the object, is the shortest of any test.

            DisplayObject* PickedObject = nullptr;

            // (The below magic number should be extrapolated from scene information/
            // The initial "ShortestPickedDistance" is to be the maximum length of the cast).

            float ShortestPickedDistance = 1000.f;


            for (auto& Object : m_displayList)
            {
                // Retrieve the global-space transform of the object.

                XMMATRIX GlobalTransform = Object.GetGlobalTransformation(m_world);

                // Unproject the cursor coordinates from the viewing frustrum,
                // to the local space of this object.

                XMVECTOR NearPoint = XMVector3Unproject(NearSource, 0.0f, 0.0f, Window.right, Window.bottom
                    , m_deviceResources->GetScreenViewport().MinDepth
                    , m_deviceResources->GetScreenViewport().MaxDepth
                    , m_projection
                    , m_view
                    , GlobalTransform);

                XMVECTOR FarPoint = XMVector3Unproject(FarSource, 0.0f, 0.0f, Window.right, Window.bottom
                    , m_deviceResources->GetScreenViewport().MinDepth
                    , m_deviceResources->GetScreenViewport().MaxDepth
                    , m_projection
                    , m_view
                    , GlobalTransform);

                // Compose a vector from the near and far sources, in object-space.

                XMVECTOR PickingVector = XMVector3Normalize(FarPoint - NearPoint);

                if (Model* Model = Object.m_model.get())
                {
                    for (int i = 0; i < Model->meshes.size(); ++i)
                    {
                        if (ModelMesh* Mesh = Model->meshes[i].get())
                        {
                            // 

                            float PickedDistance = 0.f;

                            if (Mesh->boundingBox.Intersects(NearPoint, PickingVector, PickedDistance))
                            {
                                // "PickingVector" intersects with the collision boundaries of "Object."

                                if (PickedDistance < ShortestPickedDistance)
                                {
                                    // This object is the closest to the near plane;
                                    // it is the working "PickedObject."

                                    PickedObject = &Object;

                                    ShortestPickedDistance = PickedDistance;
                                }
                            }
                        }
                    }
                }
            }

            return PickedObject;
        }
    }
    
    return nullptr;
}


void Game::Initialize(HWND window, int width, int height)
{
    // Store a class reference to the window handle.

    WindowHandle = window;


    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(WindowHandle);

    m_deviceResources->SetWindow(WindowHandle, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();


#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}


void Game::SetGridState(bool state)
{
	m_grid = state;
}


#pragma region Frame Update

void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();
}


void Game::Update(DX::StepTimer const& timer)
{
	// Update the view matrix by way of the camera controller
    // (The camera controller responds to user input).

    if (FrameworkHead && Camera)
    {
        if (InputCommands* InputState = FrameworkHead->GetInputCommands())
        {
            // Camera calculations require frame-independency.

            float DeltaTime = static_cast<float>(timer.GetElapsedSeconds());

            m_view = Camera->UpdateViewByInput(WindowHandle
                , DeltaTime, InputState);
        }
    }
    

    m_batchEffect->SetView(m_view);
    m_batchEffect->SetWorld(Matrix::Identity);

	m_displayChunk.m_terrainEffect->SetView(m_view);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

    if (FrameworkHead)
    {
        if (FrameworkHead)
        {
            std::vector<Brush*> Brushes = FrameworkHead->GetBrushes();

            for (auto Tool : Brushes)
            {
                if (auto Effect = Tool->GetIndicatorEffect())
                {
                    // Accordingly, update the view and projection matrices
                    // for the active Brush indicator's rendering settings.

                    Effect->SetView(m_view);
                    Effect->SetProjection(m_projection);
                }
            }
        }
    }

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif
}
#pragma endregion


#pragma region Frame Render

void Game::RenderObjectHighlight(DisplayObject* Object, Vector3 Scalar, XMVECTORF32 Colour)
{
    // To validate: a local reference to the renderer (Device) context.

    ID3D11DeviceContext* DeviceContext = m_deviceResources->GetD3DDeviceContext();

    if (Object && DeviceContext)
    {
        // Retrieve the global-space transform of the object.

        XMMATRIX GlobalTransform = Object->GetGlobalTransformation(m_world);

        // Post-transform the global matrix so as to correctly scale it.

        const XMVECTORF32 Translate = { Object->m_position.x, Object->m_position.y
            , Object->m_position.z };

        XMMATRIX AtOrigin = GlobalTransform * XMMatrixTranslationFromVector(-Translate);

        XMMATRIX ScaleAtOrigin = AtOrigin * XMMatrixScalingFromVector(Scalar);

        XMMATRIX Transform = ScaleAtOrigin * XMMatrixTranslationFromVector(Translate);


        // Relevant effect interfaces are collected for ease-of-access.

        std::vector<IEffectFog*> FogMesh;

        Object->m_model->UpdateEffects([&](IEffect* effect)
            {
                // Cast and validate an "IEffectFog" interface.

                FogMesh.push_back(dynamic_cast<IEffectFog*>(effect));

                if (auto Fog = FogMesh.back())
                {
                    // Enable and assign the colour argument to the fog effect,
                    // which totally colours the object drawing.

                    Fog->SetFogEnabled(true);
                    Fog->SetFogColor(Colour);
                }
                else
                {
                    // Remove the null content from the container.

                    FogMesh.pop_back();
                }
            });

        // Draw the scaled model wireframe, which has been brightly - for example -
        // coloured by the fog effect.

        Object->m_model->Draw(DeviceContext, *m_states, Transform, m_view
            , m_projection, true);


        // Iterate through the container of "IEffectFog" interfaces,
        // and disable any valid fog effects.

        for (auto Fog : FogMesh)
        {
            if (Fog)
            {
                Fog->SetFogEnabled(false);
            }
        }
    }
}

void Game::Render()
{
    // Don't try to render anything before the first "Update."
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // A labelled hierarchy of timing capture instruments the rendering sequence
    // for the benefit of the profiling task.

    m_deviceResources->PIXBeginEvent(L"Render");

    // Provide the renderer (Device) context for the issuing of rendering commands.

    ID3D11DeviceContext* DeviceContext = m_deviceResources->GetD3DDeviceContext();


	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}

    m_sprites->Begin();
	WCHAR   Buffer[256];

	m_font->DrawString(m_sprites.get(), var.c_str() , XMFLOAT2(100, 10), Colors::Yellow);
	m_sprites->End();


	/** Render objects contained in the display list, "m_displayList". */

    for (auto& Object : m_displayList)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model.");

        // "Draw" requires the transformation of the object (In global-space).

        XMMATRIX GlobalTransform = Object.GetGlobalTransformation(m_world);

        // The global wireframe flag (A "Game" class member) trumps the local flag
        // (The "DisplayObject" class member).

        bool DrawWireframe = (Wireframe) ? true : Object.m_wireframe;

        Object.m_model->Draw(DeviceContext, *m_states, GlobalTransform, m_view, m_projection
            , DrawWireframe);
        
		m_deviceResources->PIXEndEvent();
	}

    if (FrameworkHead)
    {
        m_deviceResources->PIXBeginEvent(L"Draw model outline.");

        // Of the objects that are currently "Selected,"
        // render a visual indicator (An outline).

        for (auto SelectedObject : FrameworkHead->SelectedObjects)
        {
            RenderObjectHighlight(SelectedObject, HighlightScalar, HighlightColour);
        }

        m_deviceResources->PIXEndEvent();
    }

    m_deviceResources->PIXEndEvent();

    
    /** Render the "Brush" tool pointer/indicator. */

    if (FrameworkHead)
    {
        // Request access to the Brush tool, on the condition that
        // the tool is in use.

        if (Brush* Brush = FrameworkHead->GetBrushIfActive())
        {
            // An invalid Brush indicator is not to be drawn.

            if (Brush->GetIsAwake())
            {
                m_deviceResources->PIXBeginEvent(L"Draw Brush indicator.");

                // Call upon the Brush tool to draw the indicator geometries.

                Brush->DrawIndicatorGeometry(m_world);

                m_deviceResources->PIXEndEvent();
            }
        }
    }


	/** Render the terrain. */

    DeviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    DeviceContext->OMSetDepthStencilState(m_states->DepthDefault(),0);
    DeviceContext->RSSetState(m_states->CullNone());

    if (Wireframe)
    {
        // Adapt the rasteriser stage to the wireframe rendering mode.
        
        DeviceContext->RSSetState(m_states->Wireframe());
    }

	// Terrain chunk batch rendering is encapsulated in "DisplayChunk."

	m_displayChunk.RenderBatch(m_deviceResources);

    // "Present the contents of the swap chain to the screen."

    m_deviceResources->Present();
}


void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.

    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}


void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis
    , FXMVECTOR origin
    , size_t xdivs
    , size_t ydivs
    , GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
#pragma endregion


#pragma region Message Handlers

void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}


void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

    //is the vector empty
	if (!m_displayList.empty())
	{
        //if not, empty it
		m_displayList.clear();
	}

	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		//create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;

        newDisplayObject.m_ID = SceneGraph->at(i).ID;
		
		//load model
        //convect string to Wchar
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);
        //get DXSDK to load model "False" for LH coordinate system (maya)
		newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str()
            , *m_fxFactory, true);

		//Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr
            , &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		//if texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr
                , &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}

		//apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect)
		{	
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);			
			}
		});

		//set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;
		
		//setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		//set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		//set wireframe / render flags
		newDisplayObject.m_render		= SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe	= SceneGraph->at(i).editor_wireframe;

		newDisplayObject.m_light_type		= SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r	= SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g	= SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b	= SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant	= SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear		= SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic	= SceneGraph->at(i).light_quadratic;
		
		m_displayList.push_back(newDisplayObject);
		
	}
}


void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

#pragma endregion


#pragma region Direct3D Resources

void Game::CreateDeviceDependentResources()
{
    auto DeviceContext = m_deviceResources->GetD3DDeviceContext();
    auto Device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(Device);

    m_fxFactory = std::make_unique<EffectFactory>(Device);

    //fx Factory will look in the database directory
	m_fxFactory->SetDirectory(L"database/data/");

    //we must set this to false otherwise it will share effects based on the initial tex loaded
    // (When the model loads) rather than what we will change them to.
	m_fxFactory->SetSharing(false);

    m_sprites = std::make_unique<SpriteBatch>(DeviceContext);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(DeviceContext);

    m_batchEffect = std::make_unique<BasicEffect>(Device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            Device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(Device, L"SegoeUI_18.spritefont");

    //m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(Device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(Device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(Device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );


    if (FrameworkHead)
    {
        std::vector<Brush*> Brushes = FrameworkHead->GetBrushes();

        for (auto Tool : Brushes)
        {
            Tool->CreateDeviceDependentGeometry(Device, DeviceContext);
        }
    }
}


void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();

    if (FrameworkHead)
    {
        std::vector<Brush*> Brushes = FrameworkHead->GetBrushes();

        for (auto Tool : Brushes)
        {
            Tool->ResetGeometry();
        }
    }

    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}


void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion


std::wstring StringToWCHART(std::string s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}


