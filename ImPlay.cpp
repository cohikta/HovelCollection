#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <mpv/client.h>          // 来自 mpv-dev 包的 include/
#include <string>
#include <vector>
#include <utility>

static std::string utf8(const wchar_t* w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
    s.pop_back();
    return s;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::vector<std::pair<std::string, std::string>> options;
    std::vector<std::string> paths;
    bool optEnd = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = utf8(argv[i]);
        if (!optEnd && arg.size() > 1 && arg[0] == '-') {
            if (arg == "--") { optEnd = true; continue; }
            std::string a = arg.substr(arg[1] == '-' ? 2 : 1);
            if (a.rfind("no-", 0) == 0 && a.size() > 3)
                options.emplace_back(a.substr(3), "no");          // --no-config -> config=no
            else if (auto p = a.find('='); p != std::string::npos)
                options.emplace_back(a.substr(0, p), a.substr(p + 1));
            else
                options.emplace_back(a, "yes");
        } else {
            paths.push_back(arg);                                  // thumbfast 在 "--" 后传媒体路径
        }
    }

    mpv_handle* ctx = mpv_create();
    if (!ctx) return 1;
    for (auto& [k, v] : options)
        if (mpv_set_option_string(ctx, k.c_str(), v.c_str()) < 0) {
            mpv_terminate_destroy(ctx); return 1;
        }
    if (mpv_initialize(ctx) < 0) { mpv_terminate_destroy(ctx); return 1; }

    for (auto& p : paths) {
        const char* cmd[] = {"loadfile", p.c_str(), "append-play", nullptr};
        mpv_command(ctx, cmd);
    }
    for (;;) {                                                     // 必须常驻，等 IPC 的 seek/quit
        mpv_event* ev = mpv_wait_event(ctx, -1);
        if (!ev || ev->event_id == MPV_EVENT_SHUTDOWN) break;
    }
    mpv_terminate_destroy(ctx);
    return 0;
}
