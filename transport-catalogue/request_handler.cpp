#include "request_handler.h"

#include <sstream>

using namespace std;

namespace request_handler {

// Добавляет в очередь запрос на добавление остановки
void RequestHandler::AddStopRequest(const StopRequest& stop_request) {
    stop_requests_.push_back(stop_request);
}

// Добавляет в очередь запрос на добавление маршрута
void RequestHandler::AddBusRequest(const BusRequest& bus_request) {
    bus_requests_.push_back(bus_request);
}

// Добавляет в очередь запрос на получение статистики
void RequestHandler::AddStatRequest(const StatRequest& stat_request) {
    stat_requests_.push_back(stat_request);
}

// Выполняет запросы на добавление остановок и маршрутов в каталог
void RequestHandler::ApplyBaseRequests() {
    for (const StopRequest& stop_request : stop_requests_) {
        catalogue_.AddStop(stop_request.name, stop_request.coords);
    }

    for (const StopRequest& stop_request : stop_requests_) {
        for (const auto& [stop, distance] : stop_request.distances) {
            catalogue_.SetStopDistances(stop_request.name, stop, distance);
        }
    }
    stop_requests_.clear();

    for (const BusRequest& bus_request : bus_requests_) {
        catalogue_.AddBus(bus_request.name, bus_request.stops, bus_request.is_roundtrip);
    }
    bus_requests_.clear();
}

// Выполняет запросы на получение статистики из каталога
deque<StatResponse> RequestHandler::ApplyStatRequests(map_renderer::MapRenderer& mr) {
    deque<StatResponse> stat_responses;

    for (const StatRequest& stat_request : stat_requests_) {
        if (stat_request.type == "Bus") {
            domain::BusInfo bus_info = catalogue_.GetBusInfo(stat_request.name);
            if (bus_info.num_of_stops) {
                stat_responses.emplace_back(stat_request.id, bus_info);
            } else {
                stat_responses.emplace_back(stat_request.id, nullptr);
            }
        } else if (stat_request.type == "Stop") {
            const domain::StopInfo& stop_info = catalogue_.GetStopInfo(stat_request.name);
            if (&stop_info != &catalogue_.GetStopInfo("")) {
                stat_responses.emplace_back(stat_request.id, stop_info);
            } else {
                stat_responses.emplace_back(stat_request.id, nullptr);
            }
        } else if (stat_request.type == "Map") {
            stringstream map;
            mr.Render(map, catalogue_.GetAllBuses()); 
            stat_responses.emplace_back(stat_request.id, map.str());
        }
    }
    stat_requests_.clear();

    return stat_responses;
}

// Возвращает константную ссылку на каталог
const transport_catalogue::TransportCatalogue& RequestHandler::GetCatalogue() const {
    return catalogue_;
}

} // request_handler