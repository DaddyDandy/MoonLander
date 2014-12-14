// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "pch.h"
#include "Game.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <algorithm>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace VSD3DStarter;

const float ROTATION_POWER = 0.01f;
const float MOOVE_POWER = 0.05f;
const float ANIMATION_DURATION = 0.2f;
const float MOON_GA = 1.6f;

const float START_CAM_POS_X = 0.0f;
const float START_CAM_POS_Y = 1.0f;
const float START_CAM_POS_Z = -2.5f;

Game::Game()
{
	m_rotationSpeedX = 0.0f;
	m_rotationSpeedY = 0.0f;
	m_rotationSpeedZ = 0.0f;
	m_translationSpeed = 0.0f;

	m_isGameStarted = false;
	m_isPause = false;
	m_isMultiplayer = false;
}

Game::~Game()
{
	for (Mesh* m : m_meshModels)
	{
		delete m;
	}
	m_meshModels.clear();
}

void Game::CreateWindowSizeDependentResources()
{
	GameBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;

	// setup camera for our scene
	m_graphics.GetCamera().SetViewport((UINT)m_windowBounds.Width, (UINT)m_windowBounds.Height);
	m_graphics.GetCamera().SetPosition(XMFLOAT3(START_CAM_POS_X, START_CAM_POS_Y, START_CAM_POS_Z));
	m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, 0.0f, 0.0f));

	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		/// portrait or snap view
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(1.0f, 0.0f, 0.0f));
		fovAngleY = 120.0f * XM_PI / 180.0f;

	}
	else
	{
		/// landscape view
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
	}

	m_graphics.GetCamera().SetProjection(fovAngleY, aspectRatio, 1.0f, 1000.0f);

	// setup lighting for our scene
	XMFLOAT3 pos = XMFLOAT3(5.0f, 5.0f, -2.5f);
	XMVECTOR vPos = XMLoadFloat3(&pos);

	XMFLOAT3 dir;
	XMStoreFloat3(&dir, XMVector3Normalize(vPos));

	m_lightConstants.ActiveLights = 1;
	m_lightConstants.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_lightConstants.IsPointLight[0] = false;
	m_lightConstants.LightColor[0] = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_lightConstants.LightDirection[0].x = dir.x;
	m_lightConstants.LightDirection[0].y = dir.y;
	m_lightConstants.LightDirection[0].z = dir.z;
	m_lightConstants.LightDirection[0].w = 0;
	m_lightConstants.LightSpecularIntensity[0].x = 2;

	m_graphics.UpdateLightConstants(m_lightConstants);
}

void Game::Initialize()
{
	Mesh::LoadFromFile(m_graphics, L"StarShip.cmo", L"", L"", m_starShipModel);
	Mesh::LoadFromFile(m_graphics, L"TheMoon.cmo", L"", L"", m_moonModel);
}

void Game::Clear()
{
	// clear
	m_d3dContext->OMSetRenderTargets(
		1,
		m_d3dRenderTargetView.GetAddressOf(),
		m_d3dDepthStencilView.Get()
		);

	m_d3dContext->ClearRenderTargetView(
		m_d3dRenderTargetView.Get(),
		DirectX::Colors::DarkSlateGray
		);

	m_d3dContext->ClearDepthStencilView(
		m_d3dDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);
}

String^ Game::OnHitObject(int x, int y)
{
	String^ result = nullptr;

	XMFLOAT3 point;
	XMFLOAT3 dir;
	m_graphics.GetCamera().GetWorldLine(x, y, &point, &dir);

	XMFLOAT4X4 world;
	XMMATRIX worldMat = XMMatrixIdentity();
	XMStoreFloat4x4(&world, worldMat);

	float closestT = FLT_MAX;
	for (Mesh* m : m_meshModels)
	{
		XMFLOAT4X4 meshTransform = world;

		auto name = ref new String(m->Name());

		float t = 0;
		bool hit = this->LineHitTest(m, &point, &dir, &meshTransform, &t);
		if (hit && t < closestT)
		{
			result = name;
		}
	}

	return result;
}

void Game::Update(float timeTotal, float timeDelta)
{
	if (!Pause()) // !pause
	{
		m_animationTime += timeDelta;
		float rotateAnimationProgress = std::min<float>(m_animationTime / ANIMATION_DURATION, 0.4f);

		XMVECTOR initial = XMLoadFloat3(&m_initialRotation);
		XMVECTOR target = XMLoadFloat3(&m_targetRotation);
		XMVECTOR current = initial + rotateAnimationProgress * (target - initial);
		XMStoreFloat3(&m_currentRotation, current);

		float initialT = m_initialTranslation;
		float targetT = m_targetTranslation;
		float currentT = initialT + rotateAnimationProgress * (targetT - initialT);
		m_currentTranslation = currentT;

		float fallAnimationProgress = std::min<float>(m_animationTime / ANIMATION_DURATION, 1.0f);
		//m_gravitationTranslation = -1.0f * (timeTotal * MOON_GA);
	}
}

void Game::Render()
{
	GameBase::Render();
	Clear();

	// reset camera
	m_graphics.GetCamera().SetPosition(XMFLOAT3(START_CAM_POS_X, START_CAM_POS_Y + m_gravitationTranslation, START_CAM_POS_Z + m_currentTranslation));
	m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, m_gravitationTranslation, m_currentTranslation));

	// render ship
	XMMATRIX transform = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_currentRotation));
	transform *= XMMatrixTranslation(0.0f, m_gravitationTranslation, m_currentTranslation);		
	for (UINT i = 0; i < m_starShipModel.size(); i++)
	{
		m_starShipModel[i]->Render(m_graphics, transform);
	}

	// render Moon
	transform = XMMatrixTranslation(0.0f, -30.0f, 0.0f);
	for (UINT i = 0; i < m_moonModel.size(); i++)
	{
		m_moonModel[i]->Render(m_graphics, transform);
	}

	// only enable MSAA if the device has enough power
	if (m_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
	{
		// resolve multi-sample textures into single-sample textures
		UINT resourceIndex = D3D11CalcSubresource(0, 0, 1);
		m_d3dContext->ResolveSubresource(m_backBuffer.Get(), resourceIndex, m_backBufferMsaa.Get(), resourceIndex, DXGI_FORMAT_B8G8R8A8_UNORM);
	}

	if (!Pause())
	{
		UpdateObjectTarget();
	}	
}

void Game::RotateObject(int rotationType)
{
	if (!Pause())
	{
		switch (rotationType)
		{
		case ROTATE_UP:
			m_rotationSpeedX -= ROTATION_POWER;
			break;
		case ROTATE_DOWN:
			m_rotationSpeedX += ROTATION_POWER;
			break;
		case ROTATE_RIGHT:
			m_rotationSpeedY -= ROTATION_POWER;
			break;
		case ROTATE_LEFT:
			m_rotationSpeedY += ROTATION_POWER;
			break;
		}

		UpdateObjectTarget();
	}
}



void Game::MooveObject(int mooveType)
{
	if (!Pause())
	{
		switch (mooveType)
		{
		case MOOVE_FORWARD:
			m_translationSpeed += MOOVE_POWER;
			break;
		case MOOVE_BACKWARD:
			m_translationSpeed -= MOOVE_POWER;
			break;
		}

		UpdateObjectTarget();
	}
}

void Game::UpdateObjectTarget()
{
	m_targetRotation = XMFLOAT3(m_targetRotation.x + m_rotationSpeedX, m_targetRotation.y + m_rotationSpeedY, m_targetRotation.z + m_rotationSpeedZ);
	m_initialRotation = m_currentRotation;
	
	m_targetTranslation += m_translationSpeed;
	m_initialTranslation = m_currentTranslation;

	m_animationTime = 0.0f;
	m_isAnimationRunning = true;
}