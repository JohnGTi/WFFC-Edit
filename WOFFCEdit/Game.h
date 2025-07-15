#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include "SceneObject.h"
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ChunkObject.h"

#include <vector>
#include <map>


/** Forward declaration. */
class ToolMain;
class CameraController;


/**
	* "A basic game implementation that creates a Direct3D 11 device
	* and provides a game loop."
*/
class Game : public DX::IDeviceNotify
{
	/** Constructor/Destructor. */
public:
	/** "Game" is to likely to be instantiated by the Tool framework-head. */
	Game(ToolMain* ToolFrameworkHead);
	~Game();

	/**
		* Where the terrain is composed of multiple chunks, this function
		* may be adapted to return a chunk by index.
	*/
	DisplayChunk* GetDisplayChunk()
	{
		return &m_displayChunk;
	}

	/** @return	The minumum and maximum screen viewport depth, respectively. */
	std::pair<float, float> GetViewportDepth();

	/** "Initialize the Direct3D resources required to run." */
	void Initialize(HWND window, int width, int height);

	/**  */
	void SetGridState(bool state);

	/** "Tick" executes the basic game loop.. */
	void Tick();

	/**
		* At a scale some amount larger (e.g.) than the original object,
		* render an outline of some colour.
	*/
	void RenderObjectHighlight(DisplayObject* Object
		, DirectX::SimpleMath::Vector3 Scalar, DirectX::XMVECTORF32 Colour);

	/** Draw the scene. */
	void Render();

	/** "Clear the back buffers." */
	void Clear();

	/**
		* "Game," which owns the Direct3D 11 device (resources), implements a messaging interface
		* that responds to a lost/returned connection of the device.
	*/
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	/**
		* Messages.
	*/
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);

	/** (Tool-specific). */
	void BuildDisplayList(std::vector<SceneObject> * SceneGraph); //note vector passed by reference 
	void BuildDisplayChunk(ChunkObject *SceneChunk);
	void SaveDisplayChunk(ChunkObject *SceneChunk);	//saves geometry et al
	void ClearDisplayList() {}

	/**  */
	ID3D11DeviceContext* GetDeviceContext();

	/**
		* "ToolMain" subscribes a camera class member to a delegate.
	*/
	std::shared_ptr<CameraController> GetCamera();

	/** @return */
	double GetDeltaTime();

	/** @return */
	double GetGameTime();

	/**
		* The "Pick" operation would be encapsulated, further,
		* had object manipulation become the project focus.
	*/
	DisplayObject* Pick();


	/**
		* Getters for the world, view and projection matrices.
	*/
	DirectX::SimpleMath::Matrix GetWorld() const
	{
		return m_world;
	}

	DirectX::SimpleMath::Matrix GetView() const
	{
		return m_view;
	}

	DirectX::SimpleMath::Matrix GetProjection() const
	{
		return m_projection;
	}

	/**  */
	void ToggleWireframe()
	{
		Wireframe = !Wireframe;
	}


#ifdef DXTK_AUDIO
	void NewAudioDevice();
#endif
	std::wstring var;


private:
	/**  */
	void Update(DX::StepTimer const& timer);

	/**  */
	void CreateDeviceDependentResources();

	/**
		* "Allocate all memory resources that change on a window SizeChanged event."
	*/
	void CreateWindowSizeDependentResources();

	/**  */
	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis
		, DirectX::FXMVECTOR origin
		, size_t xdivs
		, size_t ydivs
		, DirectX::GXMVECTOR color);



	/** Attributes. */

	/** This class' - a component of the tool framework - owner. */
	ToolMain* FrameworkHead = nullptr;

	/**
		* Tool-specific display objects.
	*/
	std::vector<DisplayObject>				m_displayList;
	DisplayChunk							m_displayChunk;

	/** Toggle grid rendering. */
	bool m_grid = false;

	/** Toggle wireframe rendering. */
	bool Wireframe = false;

	/** Device resources. */
	std::shared_ptr<DX::DeviceResources>    m_deviceResources;

	/** Camera controller. */
	std::shared_ptr<CameraController> Camera;

	/** Handle to the game window. */
	HWND WindowHandle = nullptr;

    /** Rendering loop timer. */
    DX::StepTimer                           m_timer;

    /** Input peripherals. */
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

	/**
		* The colour and scale (Relative to the original object)
		* of the outline of a selected object.
	*/
	DirectX::SimpleMath::Vector3 HighlightScalar;

	DirectX::XMVECTORF32 HighlightColour = DirectX::Colors::White;


    /**
		* DirectX TK objects.
	*/
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
    std::unique_ptr<DirectX::Model>                                         m_model;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

	DirectX::SimpleMath::Matrix                                             m_world;
	DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;


};

std::wstring StringToWCHART(std::string s);