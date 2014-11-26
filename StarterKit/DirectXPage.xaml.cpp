// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage.xaml class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace StarterKit;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI;
using namespace Windows::UI::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization::NumberFormatting;

DirectXPage::DirectXPage()
{
    InitializeComponent();
    
    m_renderer = ref new Game();

    m_renderer->Initialize(
        Window::Current->CoreWindow,
        SwapChainPanel,
        DisplayProperties::LogicalDpi
        );

    Window::Current->CoreWindow->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);

    DisplayProperties::LogicalDpiChanged +=
        ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnLogicalDpiChanged);

    DisplayProperties::OrientationChanged +=
        ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnOrientationChanged);

    DisplayProperties::DisplayContentsInvalidated +=
        ref new DisplayPropertiesEventHandler(this, &DirectXPage::OnDisplayContentsInvalidated);

    m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

    m_timer = ref new BasicTimer();
}

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnLogicalDpiChanged(Object^ sender)
{
    m_renderer->SetDpi(DisplayProperties::LogicalDpi);
}

void DirectXPage::OnOrientationChanged(Object^ sender)
{
    m_renderer->UpdateForWindowSizeChange();
}

void DirectXPage::OnDisplayContentsInvalidated(Object^ sender)
{
    m_renderer->ValidateDevice();
}

void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
    m_timer->Update();
    m_renderer->Update(m_timer->Total, m_timer->Delta);
    m_renderer->Render();
    m_renderer->Present();
}

void DirectXPage::SaveInternalState(IPropertySet^ state)
{
}

void DirectXPage::LoadInternalState(IPropertySet^ state)
{
}

void DirectXPage::OnTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	// TODO : some test	
	if (!m_renderer->AnimationRunning())
	{
		m_renderer->rotateObject(0);
	}		
}

void DirectXPage::OnKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	if (!m_renderer->AnimationRunning())
	{
		switch (e->Key)
		{
		case Windows::System::VirtualKey::W:
			m_renderer->rotateObject(ROTATE_UP);
			break;
		case Windows::System::VirtualKey::S:
			m_renderer->rotateObject(ROTATE_DOWN);
			break;
		case Windows::System::VirtualKey::A:
			m_renderer->rotateObject(ROTATE_LEFT);
			break;
		case Windows::System::VirtualKey::D:
			m_renderer->rotateObject(ROTATE_RIGHT);
			break;
		default:
			break;
		}
	}
}
