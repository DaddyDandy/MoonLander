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

Game::Game()
{
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

    //
    // setup camera for our scene
    //
    m_graphics.GetCamera().SetViewport((UINT)m_windowBounds.Width, (UINT)m_windowBounds.Height);
    m_graphics.GetCamera().SetPosition(XMFLOAT3(0.0f, 6.0f, -18.0f));
    m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, 0.0f, 0.0f));

    float fovAngleY = 70.0f * XM_PI / 180.0f;

    if (aspectRatio < 1.0f)
    {
        ///
        /// portrait or snap view
        ///
        m_graphics.GetCamera().SetUpVector(XMFLOAT3(1.0f, 0.0f, 0.0f));
        fovAngleY = 120.0f * XM_PI / 180.0f;

    }
    else
    {
        ///
        /// landscape view
        ///
        m_graphics.GetCamera().SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
    }

    m_graphics.GetCamera().SetProjection(fovAngleY, aspectRatio, 1.0f, 1000.0f);

    //
    // setup lighting for our scene
    //
    XMFLOAT3 pos = XMFLOAT3(5.0f, 5.0f, -2.5f);
    XMVECTOR vPos = XMLoadFloat3(&pos);

    XMFLOAT3 dir;
    XMStoreFloat3(&dir, XMVector3Normalize(vPos));

    m_lightConstants.ActiveLights = 1;
    m_lightConstants.Ambient =  XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
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
}

void Game::Update(float timeTotal, float timeDelta)
{
}

void Game::Render()
{
    GameBase::Render();

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

    //
    // only enable MSAA if the device has enough power
    //
    if (m_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
    {
        //
        // resolve multi-sample textures into single-sample textures
        //
        UINT resourceIndex = D3D11CalcSubresource(0, 0, 1);
        m_d3dContext->ResolveSubresource(m_backBuffer.Get(), resourceIndex, m_backBufferMsaa.Get(), resourceIndex, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
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