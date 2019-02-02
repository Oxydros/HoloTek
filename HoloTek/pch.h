#pragma once

#define TRACE( s )            \
{                             \
   std::wstringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

#include <iostream>
#include <iomanip>
#include <ctime>
#include <array>
#include <future>
#include <functional>
#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <sstream>
#include <experimental/resumable>

#include <pplawait.h>
#include <wincodec.h>
#include <WindowsNumerics.h>
#include <hstring.h>
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <d2d1_2.h>
#include <d2d1effects_1.h>
#include <d3d11_4.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <dwrite_2.h>
#include <dxgi1_5.h>

#include <Windows.Graphics.Directx.Direct3D11.Interop.h>
#include <wrl/client.h>

#include <winrt\Windows.ApplicationModel.Activation.h>
#include <winrt\Windows.ApplicationModel.Core.h>

#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>
#include <winrt\Windows.Foundation.Numerics.h>
#include <winrt\Windows.Foundation.Metadata.h>

#include <winrt\Windows.Gaming.Input.h>

#include <winrt\Windows.Graphics.Display.h>
#include <winrt\Windows.Graphics.Holographic.h>
#include <winrt\Windows.Graphics.Imaging.h>
#include <winrt\Windows.Graphics.DirectX.h>
#include <winrt\Windows.Graphics.DirectX.Direct3D11.h>

#include <winrt\Windows.Perception.People.h>
#include <winrt\Windows.Perception.Spatial.h>

#include <winrt\Windows.Storage.h>
#include <winrt\Windows.Storage.Streams.h>

#include <winrt\Windows.UI.Core.h>
#include <winrt\Windows.UI.Input.Spatial.h>

#include <winrt\Windows.Media.h>
#include <winrt\Windows.Media.Capture.h>
#include <winrt\Windows.Media.Capture.Frames.h>
#include <winrt\Windows.Media.Devices.h>
#include <winrt\Windows.Media.Devices.Core.h>
#include <winrt\Windows.Media.FaceAnalysis.h>

#include <winrt\Windows.Web.h>
#include <winrt\Windows.Web.Http.h>
#include <winrt\Windows.Data.Json.h>
