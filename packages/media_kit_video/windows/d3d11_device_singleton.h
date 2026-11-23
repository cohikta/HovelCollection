#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <vector>

class D3D11DeviceSingleton {
 public:
  static D3D11DeviceSingleton& Instance() {
    static D3D11DeviceSingleton instance;
    return instance;
  }

  ID3D11Device* device() const { return device_.Get(); }
  ID3D11DeviceContext* context() const { return context_.Get(); }

  std::mutex& mutex() { return immediate_context_mutex_; }

  LUID adapter_luid() const { return adapter_luid_; }

 private:
  D3D11DeviceSingleton() {
    const std::vector<D3D_FEATURE_LEVEL> levels = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,
    };

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    char buf[32] = {};
    const DWORD n = ::GetEnvironmentVariableA(
        "MEDIA_KIT_D3D11_ADAPTER_INDEX", buf, static_cast<DWORD>(sizeof(buf)));
    if (n > 0 && n < sizeof(buf)) {
      const int index = std::atoi(buf);
      if (index > 0) {
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;
        if (SUCCEEDED(::CreateDXGIFactory(__uuidof(IDXGIFactory),
                                          (void**)&factory))) {
          Microsoft::WRL::ComPtr<IDXGIAdapter> enumerated;
          if (SUCCEEDED(factory->EnumAdapters(static_cast<UINT>(index),
                                              &enumerated))) {
            adapter = enumerated;
            std::cout << "media_kit: D3D11DeviceSingleton: using adapter index "
                      << index << " (MEDIA_KIT_D3D11_ADAPTER_INDEX)"
                      << std::endl;
          }
        }
      }
    }

    const HRESULT hr = ::D3D11CreateDevice(
        adapter.Get(), D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels.data(),
        static_cast<UINT>(levels.size()), D3D11_SDK_VERSION, &device_,
        nullptr, &context_);
    if (FAILED(hr)) {
      std::cout << "media_kit: D3D11DeviceSingleton: D3D11CreateDevice failed "
                   "hr=0x" << std::hex << hr << std::dec << std::endl;
      return;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
    if (SUCCEEDED(device_->QueryInterface(__uuidof(IDXGIDevice),
                                          (void**)&dxgi_device))) {
      Microsoft::WRL::ComPtr<IDXGIAdapter> used;
      if (SUCCEEDED(dxgi_device->GetAdapter(&used))) {
        DXGI_ADAPTER_DESC desc{};
        if (SUCCEEDED(used->GetDesc(&desc))) {
          adapter_luid_ = desc.AdapterLuid;
          std::cout << "media_kit: D3D11DeviceSingleton: AdapterLuid="
                    << desc.AdapterLuid.HighPart << ":"
                    << desc.AdapterLuid.LowPart << " FeatureLevel=0x"
                    << std::hex << (unsigned)device_->GetFeatureLevel()
                    << std::dec << std::endl;
        }
      }
    }
  }

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  std::mutex immediate_context_mutex_;
  LUID adapter_luid_{};
};
