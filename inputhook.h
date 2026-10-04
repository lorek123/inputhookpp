#pragma once

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include <string_view>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <sys/types.h>

extern "C" {
#include "ezinject_module.h"
#include <gum/gum.h>
}

struct keybind_info_t {
    int unk1[4];
    int uinput_code;
    int unk2[9];
};

struct uinput_info_t {
    int fd;
    keybind_info_t* keybinds;
};

struct input_event_t {
    uint64_t time;
    uint16_t type;
    uint16_t code;
    int32_t value;
};

enum class Action {
    REPLACE,
    PASS,
    IGNORE,
};

class InputHook {
    using lginput_uinput_send_button_t = int(uinput_info_t*, int, int);
    using MICOM_FuncWriteKeyEvent_t = int(int, uint16_t, uint16_t, int32_t);
    using write_t = ssize_t(int, input_event_t*, size_t);

    static constexpr std::string_view CONFIG_LOCATION = "/home/root/.config/lginputhook/keybinds.json";

public:
    explicit InputHook();

private:
    void resolveFunctions();

    void applyHooks();

    bool loadKeybinds();

    void launch(const std::string& cmd);

    [[noreturn]] void launchWorker();

    [[noreturn]] void watchConfigFile();

    std::tuple<Action, int> handleKey(int keycode, int state);

    static ssize_t trampoline_write(int fd, input_event_t* events, size_t count);

    static int trampoline_lginput(uinput_info_t* info, int keyid, int state);

    static int trampoline_MICOM_FuncWriteKeyEvent(int fd, uint16_t type, uint16_t code, int32_t value);

    int hook_lginput(uinput_info_t* info, int keyid, int state);

    int hook_MICOM_FuncWriteKeyEvent(int fd, uint16_t type, uint16_t code, int32_t value);

    ssize_t hook_write(int fd, input_event_t* events, size_t count);

    bool isUinput(int fd) const;

    GumInterceptor* m_interceptor{nullptr};
    nlohmann::json m_keybinds{};
    std::mutex m_mutex{};

    std::mutex m_launchMutex{};
    std::condition_variable m_launchCv{};
    std::deque<std::string> m_launchQueue{};

    dev_t m_uinputRdev{0};

    lginput_uinput_send_button_t* orig_lginput_uinput_send_button{nullptr};
    MICOM_FuncWriteKeyEvent_t* orig_MICOM_FuncWriteKeyEvent{nullptr};
    write_t* orig_write{nullptr};
};
