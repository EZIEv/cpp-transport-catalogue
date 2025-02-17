<<<<<<< HEAD
// место для вашего кода
=======
#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

// Возвращает указатель на остановку по ее имени
const Stop* TransportCatalogue::GetStop(string_view stop_name) const {
    return stop_by_name_.at(stop_name);
}

// Возвращает указатель на автобусный маршрут по его имени
const Bus* TransportCatalogue::GetBus(string_view bus_name) const {
    return bus_by_name_.at(bus_name);
}

// Возвращает ссылку на информацию об автобусном маршруте по его имени
const BusInfo& TransportCatalogue::GetBusInfo(string_view bus_name) const {
    return bus_by_name_.at(bus_name)->info;
}

/* Возвращает ссылку на упорядоченное по алфавиту множество
   наименований автобусных маршрутов, проходящих через остановку,
   по имени остановки */
const set<string_view>& TransportCatalogue::GetStopInfo(string_view stop_name) const {
    return buses_on_stop_.at(stop_name);
}

// Добавляет новую остановку в транспортный справочник
void TransportCatalogue::AddStop(string name, geo::Coordinates coords) {
    stops_.push_back({name, coords}); // Создает новую остановку
    const Stop* stop_ptr = &stops_.back(); // Создает указатель на остановку
    stop_by_name_[stop_ptr->name] = stop_ptr;
    buses_on_stop_[stop_ptr->name];
}

// Добавляет новый автобусный маршрут в транспортный справочник
void TransportCatalogue::AddBus(string name, vector<string_view> stops_names) {
    vector<const Stop*> stops; // Создает набор указателей на остановки, через которые проходит маршрут
    stops.reserve(stops_names.size());
    unordered_set<const Stop*> unique_stops; // Создает набор уникальных указателей на остановки, через которые проходит маршрут
    double length = 0;
    for (string_view stop_name : stops_names) {
        const Stop* stop = GetStop(stop_name);
        if (!stops.empty()) {
            if (!distances_.contains({stops.back(), stop})) {
                distances_[{stops.back(), stop}] = geo::ComputeDistance(stops.back()->coords, stop->coords);
            }
            length += distances_[{stops.back(), stop}];
        }
        stops.push_back(stop);
        unique_stops.insert(stop);
    }

    buses_.push_back({name, move(stops), {stops_names.size(), unique_stops.size(), length}}); // Создает новый маршрут
    const Bus* bus_ptr = &buses_.back(); // Создает указатель на этот маршрут
    bus_by_name_[bus_ptr->name] = bus_ptr;
    for (const Stop* stop : bus_ptr->stops) {
        buses_on_stop_[stop->name].insert(bus_ptr->name);
    }
}

}
>>>>>>> master
