#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace request_handler {

struct StopRequest {
    std::string name; // < имя остановки
    geo::Coordinates coords; // < географические координаты остановки
    std::unordered_map<std::string, int> distances; // < расстояния до прилегающих остановок
};

struct BusRequest {
    std::string name; // < имя марщрута
    std::vector<std::string> stops; // < имена остановок, входящих в маршрута
    bool is_roundtrip; // < флаг типа маршрута (true - кольцевой, false - некольцевой)
};

struct StatRequest {
    int id; // < id запроса статистики
    std::string type; // < типа запроса статистики (Bus, Stop, Map)
    std::string name; // < имя маршрута или остановки (для Map значение "")
};

struct StatResponse {
    int id; // < id запроса статистики
    std::variant<std::nullptr_t, domain::BusInfo, domain::StopInfo, std::string> info; // < описание ответа на запрос
};

class RequestHandler {
public:
    // Выполняет запросы на добавление остановок и маршрутов в каталог
    void ApplyBaseRequests();

    // Выполняет запросы на получение статистики из каталога
    std::deque<StatResponse> ApplyStatRequests(map_renderer::MapRenderer& mr);

    // Возвращает константную ссылку на каталог
    const transport_catalogue::TransportCatalogue& GetCatalogue() const;

    // Добавляет в очередь запрос на добавление остановки
    void AddStopRequest(const StopRequest& stop_request);

    // Добавляет в очередь запрос на добавление маршрута
    void AddBusRequest(const BusRequest& bus_request);

    // Добавляет в очередь запрос на получение статистики
    void AddStatRequest(const StatRequest& stat_request);

private:
    transport_catalogue::TransportCatalogue catalogue_; // < транспортный справочник (каталог)

    std::deque<StopRequest> stop_requests_; // < очередь запросов на добалвение остановки
    std::deque<BusRequest> bus_requests_; // < очередь запросов на добавление маршрутов
    std::deque<StatRequest> stat_requests_; // < очередь запросов на получение статистики
};

} // namespace request_handler