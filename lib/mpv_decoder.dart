import 'playback_preferences.dart';

/// Windows 零拷贝硬解：D3D11VA 解码表面经 ANGLE 的 EGL interop
/// 直接交给 GL renderer，不经系统内存。
const kWindowsDirectHwdec = 'd3d11va';
const kWindowsCopyBackHwdec = 'd3d11va-copy';

enum WindowsHwdecMode { direct, copyBack }

/// 与原 switch 等价，仅新增：hardware + Windows 走 direct。
/// 平台以参数注入，便于在 CI / 单测中覆盖全平台。
String? resolveHwdec(
  TelevisionDecoder decoder, {
  required bool isAndroid,
  required bool isIOS,
  required bool isMacOS,
  required bool isWindows,
  WindowsHwdecMode windowsMode = WindowsHwdecMode.copyBack,
}) {
  if (decoder == TelevisionDecoder.software) return 'no';

  if (decoder == TelevisionDecoder.hardware) {
    if (isAndroid) return 'mediacodec-copy';
    if (isIOS || isMacOS) return 'videotoolbox-copy';
    if (isWindows) {
      return windowsMode == WindowsHwdecMode.direct
          ? kWindowsDirectHwdec
          : kWindowsCopyBackHwdec;
    }
    return null;
  }

  // optimized（默认档）：完全保留原行为，不进入实验路径。
  if (isWindows) return kWindowsCopyBackHwdec;
  if (isAndroid || isMacOS) return 'auto-safe';
  return null;
}
