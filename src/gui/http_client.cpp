#include "http_client.h"

HttpClient::HttpClient(const std::string& host, int port)
    : m_Client(host, port)
{
}

HttpClient::~HttpClient()
{
}

bool HttpClient::sendCommand(const std::string& endpoint, const nlohmann::json& data)
{
    auto res = m_Client.Post(endpoint, data.dump(), "application/json");
    if (!res || res->status != 200) {
        return false;
    }
    return true;
}

bool HttpClient::startSimulation()
{
    return sendCommand("/simulation/start");
}

bool HttpClient::stopSimulation()
{
    return sendCommand("/simulation/stop");
}

bool HttpClient::stepSimulation()
{
    return sendCommand("/simulation/step");
}

bool HttpClient::resetSimulation()
{
    return sendCommand("/simulation/reset");
}

bool HttpClient::getNetworkState(std::string& state)
{
    auto res = m_Client.Get("/network/state");
    if (!res || res->status != 200) {
        return false;
    }
    state = res->body;
    return true;
}

bool HttpClient::loadNetwork(const std::string& filename)
{
    nlohmann::json data;
    data["filename"] = filename;
    return sendCommand("/network/load", data);
}

bool HttpClient::saveNetwork(const std::string& filename)
{
    nlohmann::json data;
    data["filename"] = filename;
    return sendCommand("/network/save", data);
}

void HttpClient::setStateUpdateCallback(std::function<void(const std::string&)> callback)
{
    m_StateUpdateCallback = callback;
} 