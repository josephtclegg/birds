#include "pch.h"
#include "stb_image.h"
#if __has_include("stb_image.g.cpp")
#include "stb_image.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::birds::implementation
{
    int32_t stb_image::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void stb_image::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void stb_image::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
