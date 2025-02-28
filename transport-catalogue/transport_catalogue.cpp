#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

// Возвращает указатель на остановку по ее имени
const Stop* TransportCatalogue::GetStop(string_view stop_name) const {
    auto it = stop_by_name_.find(stop_name);
    return it != stop_by_name_.end() ? it->second : nullptr;
}

// Возвращает указатель на автобусный маршрут по его имени
const Bus* TransportCatalogue::GetBus(string_view bus_name) const {
    auto it = bus_by_name_.find(bus_name);
    return it != bus_by_name_.end() ? it->second : nullptr;
}

// Возвращает информацию об автобусном маршруте по его имени
const BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
    const Bus* bus = GetBus(bus_name);
    if (!bus) {
        return {0, 0, 0, 0};
    }
    
    unordered_set<const Stop*> unique_stops; // Создает набор уникальных указателей на остановки, через которые проходит маршрут
    double route_length = 0;
    double geo_length = 0;
    for (size_t i = 0; i < bus->stops.size(); ++i) {
        if (i != 0) {
            geo_length += geo::ComputeDistance((*bus).stops[i - 1]->coords, (*bus).stops[i]->coords);
            auto it = stop_to_stop_.find({(*bus).stops[i - 1], (*bus).stops[i]});
            route_length += it != stop_to_stop_.end() ? it->second : stop_to_stop_.at({(*bus).stops[i], (*bus).stops[i - 1]});
        }
        unique_stops.insert((*bus).stops[i]);
    }

    return {(*bus).stops.size(), unique_stops.size(), route_length, route_length / geo_length};
}

/* Возвращает константную ссылку на множество наименований автобусных маршрутов, 
   проходящих через остановку, по имени остановки */
const unordered_set<const Bus*>& TransportCatalogue::GetStopInfo(string_view stop_name) const {
    static const unordered_set<const Bus*> empty_set;
    auto it = buses_on_stop_.find(stop_name);
    return it != buses_on_stop_.end() ? it->second : empty_set;
}

// Добавляет новую остановку в транспортный справочник
void TransportCatalogue::AddStop(const string& name, geo::Coordinates coords) {
    stops_.push_back({name, coords}); // Создает новую остановку
    const Stop* stop_ptr = &stops_.back(); // Создает указатель на остановку
    stop_by_name_[stop_ptr->name] = stop_ptr;
    buses_on_stop_[stop_ptr->name];    
}

// Добавляет расстояние между двумя остановками в справочник
void TransportCatalogue::AddStopDistances(string_view from, const std::unordered_map<std::string_view, int>& distances) {
    const Stop* stop_ptr = GetStop(from);
    for (const auto& [to_stop, distance] : distances) {
        stop_to_stop_[{stop_ptr, GetStop(to_stop)}] = distance;
    }
}

// Добавляет новый автобусный маршрут в транспортный справочник
void TransportCatalogue::AddBus(const string& name, const vector<string_view>& stops_names) {
    vector<const Stop*> stops; // Создает набор указателей на остановки, через которые проходит маршрут
    stops.reserve(stops_names.size());

    for (string_view stop_name : stops_names) {
        stops.push_back(GetStop(stop_name));
    }

    buses_.push_back({name, move(stops)}); // Создает новый маршрут
    const Bus* bus_ptr = &buses_.back(); // Создает указатель на этот маршрут
    
    bus_by_name_[bus_ptr->name] = bus_ptr;
    for (const Stop* stop : bus_ptr->stops) {
        buses_on_stop_[stop->name].insert(bus_ptr);
    }
}

}