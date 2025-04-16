#pragma once

#include <string>
#include <functional>
#include <httplib.h>
#include <nlohmann/json.hpp>

class HttpClient {
public:
    HttpClient(const std::string& host = "localhost", int port = 8080);
    ~HttpClient();

    // 网络控制命令
    bool startSimulation();
    bool stopSimulation();
    bool stepSimulation();
    bool resetSimulation();

    // 网络状态
    bool getNetworkState(std::string& state);
    bool loadNetwork(const std::string& filename);
    bool saveNetwork(const std::string& filename);

    // 设置状态更新回调
    void setStateUpdateCallback(std::function<void(const std::string&)> callback);

private:
    httplib::Client m_Client;
    std::function<void(const std::string&)> m_StateUpdateCallback;

    bool sendCommand(const std::string& endpoint, const nlohmann::json& data = nlohmann::json());
}; 