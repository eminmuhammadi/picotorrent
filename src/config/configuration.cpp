#include <picotorrent/config/configuration.hpp>

#include <picotorrent/picojson.hpp>
#include <picotorrent/common/environment.hpp>
#include <picotorrent/common/string_operations.hpp>
#include <picotorrent/filesystem/directory.hpp>
#include <picotorrent/filesystem/file.hpp>
#include <picotorrent/filesystem/path.hpp>

namespace fs = picotorrent::filesystem;
namespace pj = picojson;
using namespace picotorrent::common;
using picotorrent::config::configuration;

configuration::configuration()
{
    load();
}

configuration::~configuration()
{
    save();
}

configuration& configuration::instance()
{
    static configuration instance;
    return instance;
}

int configuration::alert_queue_size()
{
    return get_or_default("alert_queue_size", 500);
}

configuration::close_action_t configuration::close_action()
{
    return (configuration::close_action_t)get_or_default("close_action", (int64_t)configuration::close_action_t::prompt);
}

void configuration::set_close_action(configuration::close_action_t action)
{
    set("close_action", (int)action);
}

bool configuration::check_for_updates()
{
    return get_or_default("check_for_updates", true);
}

std::wstring configuration::default_save_path()
{
    fs::path defaultPath = environment::get_special_folder(special_folder::user_downloads);
    return get_or_default<std::wstring>("default_save_path", defaultPath.to_string());
}

void configuration::set_default_save_path(const std::wstring &path)
{
    set("default_save_path", path);
}

std::wstring configuration::ignored_update()
{
    return get_or_default<std::wstring>("ignored_update", L"");
}

void configuration::set_ignored_update(const std::wstring &version)
{
    set("ignored_update", version);
}

std::wstring configuration::listen_address()
{
    return get_or_default<std::wstring>("listen_address", L"0.0.0.0");
}

void configuration::set_listen_address(const std::wstring &address)
{
    set("listen_address", address);
}

int configuration::listen_port()
{
    return get_or_default("listen_port", 6881);
}

void configuration::set_listen_port(int port)
{
    set("listen_port", port);
}

bool configuration::prompt_for_remove_data()
{
    return get_or_default("prompt_for_remove_data", true);
}

void configuration::set_prompt_for_remove_data(bool value)
{
    set("prompt_for_remove_data", value);
}

bool configuration::prompt_for_save_path()
{
    return get_or_default("prompt_for_save_path", true);
}

void configuration::set_prompt_for_save_path(bool value)
{
    set("prompt_for_save_path", value);
}

configuration::proxy_type_t configuration::proxy_type()
{
    return (configuration::proxy_type_t)get_or_default("proxy_type", (int64_t)configuration::proxy_type_t::none);
}

void configuration::set_proxy_type(configuration::proxy_type_t type)
{
    set("proxy_type", (int)type);
}

int configuration::stop_tracker_timeout()
{
    return get_or_default("stop_tracker_timeout", 1);
}

std::wstring configuration::update_url()
{
    return get_or_default<std::wstring>("update_url", L"https://api.github.com/repos/picotorrent/picotorrent/releases/latest");
}

template<typename T>
T configuration::get_or_default(const char *name, T defaultValue)
{
    auto &item = value_->find(name);

    if (item == value_->end())
    {
        return defaultValue;
    }

    return item->second.get<T>();
}

template<typename T>
void configuration::set(const char *name, T value)
{
    (*value_)[name] = pj::value(value);
}

template<>
int configuration::get_or_default<int>(const char *name, int defaultValue)
{
    auto &item = value_->find(name);

    if (item == value_->end())
    {
        return defaultValue;
    }

    return (int)item->second.get<int64_t>();
}

template<>
std::wstring configuration::get_or_default<std::wstring>(const char *name, std::wstring defaultValue)
{
    auto &item = value_->find(name);

    if (item == value_->end())
    {
        return defaultValue;
    }

    return to_wstring(item->second.get<std::string>());
}

template<>
void configuration::set<int>(const char *name, int value)
{
    (*value_)[name] = pj::value((int64_t)value);
}

template<>
void configuration::set<std::wstring>(const char *name, std::wstring value)
{
    std::string s = to_string(value);
    if (s[s.size() - 1] == '\0') { s = s.substr(0, s.size() - 1); }

    (*value_)[name] = pj::value(s);
}

void configuration::load()
{
    fs::path data = environment::get_data_path();
    fs::file cfg = data.combine(L"PicoTorrent.json");

    if (!cfg.path().exists())
    {
        value_ = std::make_unique<pj::object>();
        return;
    }

    std::vector<char> buf;
    cfg.read_all(buf);

    pj::value v;
    pj::parse(v, buf.begin(), buf.end(), nullptr);

    value_ = std::make_unique<pj::object>(v.get<pj::object>());
}

void configuration::save()
{
    pj::value v(*value_);
    std::string json = v.serialize(true);
    std::vector<char> buf(json.begin(), json.end());

    fs::directory data = environment::get_data_path();

    if (!data.path().exists())
    {
        data.create();
    }

    fs::file cfg = data.path().combine(L"PicoTorrent.json");
    cfg.write_all(buf);
}
