// packages/media_kit_video/windows/d3d11_surface_manager.h
// Based on media_kit_video 2.0.1 (MIT, Hitesh Kumar Saini).
// Modified for guoapp: Windows uses mpv 0.42 MPV_RENDER_API_TYPE_D3D11.
#ifndef D3D11_SURFACE_MANAGER_H_
#define D3D11_SURFACE_MANAGER_H_

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>

#include <cstdint>
#include <functional>

#include "d3d11_device_singleton.h"

// |D3D11SurfaceManager| owns the texture mpv renders into and the shared
// texture handed to Flutter. The ID3D11Device itself is process-wide
// and owned by |D3D11DeviceSingleton|; instances only hold a reference.
// * |Draw|: runs mpv_render_context_render into |internal| texture, then
//   copies |internal| -> |external| and submits everything with one Flush().
// * |Read|: kept for API compatibility; intentionally does no GPU work now.
class D3D11SurfaceManager {
 public:
  const int32_t width() const { return width_; }
  const int32_t height() const { return height_; }
  const HANDLE handle() const { return handle_; }

  ID3D11Device* device() const { return d3d_11_device_.Get(); }
  ID3D11Texture2D* target() const { return d3d_11_texture_2D_.Get(); }

  D3D11SurfaceManager(int32_t width, int32_t height);
  ~D3D11SurfaceManager();

  void SetSize(int32_t width, int32_t height);
  // If |publish| is false (destruction path), skip the copy into the shared
  // texture but still flush mpv's pending work.
  void Draw(std::function<void()> callback, bool publish = true);
  void Read();

 private:
  void Create();
  void CleanUp(bool release_device);
  bool CreateD3DTexture();

  int32_t width_ = 1;
  int32_t height_ = 1;
  HANDLE handle_ = nullptr;

  Microsoft::WRL::ComPtr<ID3D11Device> d3d_11_device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d_11_device_context_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> internal_d3d_11_texture_2D_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d_11_texture_2D_;
};

#endif  // D3D11_SURFACE_MANAGER_H_