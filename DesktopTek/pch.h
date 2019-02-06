//
// pch.h
// Header for platform projection include files
//

#pragma once

#define TRACE( s )            \
{                             \
   std::wstringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

#include <array>
#include <regex>
#include <future>
#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <map>

#include <pplawait.h>
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>

#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>

#include <winrt\Windows.ApplicationModel.Activation.h>

#include <winrt\Windows.UI.Core.h>
#include <winrt\Windows.UI.Xaml.h>
#include <winrt\Windows.UI.Xaml.Controls.h>
#include <winrt\Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt\Windows.UI.Xaml.Data.h>
#include <winrt\Windows.UI.Xaml.Interop.h>
#include <winrt\Windows.UI.Xaml.Markup.h>
#include <winrt\Windows.UI.Xaml.Navigation.h>
#include <winrt\Windows.UI.Xaml.Shapes.h>
#include <winrt\Windows.UI.Xaml.Media.h>
#include <winrt\Windows.UI.Xaml.Media.Imaging.h>

#include <winrt\Windows.Media.h>
#include <winrt\Windows.Media.Core.h>
#include <winrt\Windows.Media.MediaProperties.h>
#include <winrt\Windows.Media.Capture.h>
#include <winrt\Windows.Media.Capture.Frames.h>
#include <winrt\Windows.Media.Devices.h>
#include <winrt\Windows.Media.Devices.Core.h>
#include <winrt\Windows.Media.FaceAnalysis.h>

#include <winrt\Windows.System.Display.h>
#include <winrt\Windows.System.Threading.h>

#include <winrt\Windows.Graphics.Imaging.h>

#include <winrt\Windows.Storage.h>
#include <winrt\Windows.Storage.Streams.h>

#include <winrt\Windows.Web.h>
#include <winrt\Windows.Web.Http.h>
#include <winrt\Windows.Web.Http.Headers.h>

#include <winrt\Windows.Data.Json.h>

#include <dlib\dnn.h>
#include <dlib\clustering.h>
#include <dlib\string.h>
#include <dlib\image_io.h>
#include <dlib\image_processing\frontal_face_detector.h>
#include <dlib\gui_widgets.h>
