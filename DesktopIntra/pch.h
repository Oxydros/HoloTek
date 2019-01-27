//
// pch.h
// Header for platform projection include files
//

#pragma once

#define DBOUT( s )            \
{                             \
   std::wstringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

#include <pplawait.h>
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>
#include <winrt\Windows.ApplicationModel.Activation.h>
#include <winrt\Windows.UI.Xaml.h>
#include <winrt\Windows.UI.Xaml.Controls.h>
#include <winrt\Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt\Windows.UI.Xaml.Data.h>
#include <winrt\Windows.UI.Xaml.Interop.h>
#include <winrt\Windows.UI.Xaml.Markup.h>
#include <winrt\Windows.UI.Xaml.Navigation.h>
#include <winrt\Windows.Web.h>
#include <winrt\Windows.Web.Http.h>
#include <winrt\Windows.Data.Json.h>
