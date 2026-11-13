import 'package:flutter_test/flutter_test.dart';
import 'package:duanju_app/mpv_decoder.dart';
import 'package:duanju_app/playback_preferences.dart';

void main() {
  String? call(
    TelevisionDecoder d, {
    bool a = false,
    bool i = false,
    bool m = false,
    bool w = false,
    WindowsHwdecMode mode = WindowsHwdecMode.direct,
  }) =>
      resolveHwdec(
        d,
        isAndroid: a,
        isIOS: i,
        isMacOS: m,
        isWindows: w,
        windowsMode: mode,
      );

  group('resolveHwdec 与原 switch 等价', () {
    test('software 全平台软解', () {
      expect(call(TelevisionDecoder.software, w: true), 'no');
      expect(call(TelevisionDecoder.software, a: true), 'no');
    });

    test('optimized 默认档行为不变', () {
      expect(call(TelevisionDecoder.optimized, w: true), 'd3d11va-copy');
      expect(call(TelevisionDecoder.optimized, a: true), 'auto-safe');
      expect(call(TelevisionDecoder.optimized, m: true), 'auto-safe');
      expect(call(TelevisionDecoder.optimized, i: true), isNull);
      expect(call(TelevisionDecoder.optimized), isNull);
    });

    test('hardware 移动端不变', () {
      expect(call(TelevisionDecoder.hardware, a: true), 'mediacodec-copy');
      expect(call(TelevisionDecoder.hardware, i: true), 'videotoolbox-copy');
      expect(call(TelevisionDecoder.hardware, m: true), 'videotoolbox-copy');
      expect(call(TelevisionDecoder.hardware), isNull);
    });
  });

  group('Windows 直通与回退', () {
    test('hardware + direct', () {
      expect(call(TelevisionDecoder.hardware, w: true), 'd3d11va');
    });

    test('探测失败后 copyBack，且不影响 optimized', () {
      expect(
        call(TelevisionDecoder.hardware, w: true,
            mode: WindowsHwdecMode.copyBack),
        'd3d11va-copy',
      );
      expect(
        call(TelevisionDecoder.optimized, w: true,
            mode: WindowsHwdecMode.copyBack),
        'd3d11va-copy',
      );
    });
  });
}
