#pragma once

#include <deque>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
	std::string name; // < название остановки
	geo::Coordinates coords; // < координаты остановки
};

struct BusInfo {
	size_t num_of_stops; // < количество остановок на маршруте
	size_t num_of_unique_stops; // < количество уникальных остановок на маршруте;
	double route_length; // < реальная длина маршрута
	double curvature; // < кривизна маршрута (отношение реальной длины к географической)
};

struct Bus {
	std::string name; // < название автобусного маршрута
	std::vector<const Stop*> stops; // < набор остановок на маршруте
};

// Класс-хэшер pair<const Stop*, const Stop*>
class StopsPairHasher {
public:
	size_t operator() (const std::pair<const Stop*, const Stop*>& pair_of_stops) const {
		return (p_hasher_(pair_of_stops.first) + p_hasher_(pair_of_stops.second) * 37);
	}

private:
	std::hash<const void*> p_hasher_;
};

class TransportCatalogue {
public:
	// Возвращает указатель на остановку по ее имени
	const Stop* GetStop(std::string_view stop_name) const;

	// Возвращает указатель на автобусный маршрут по его имени
	const Bus* GetBus(std::string_view bus_name) const;

	// Возвращает информацию об автобусном маршруте по его имени
	const BusInfo GetBusInfo(std::string_view bus_name) const;

	/* Возвращает константную ссылку на множество автобусных маршрутов, 
	   проходящих через остановку, по имени остановки */
	const std::unordered_set<const Bus*>& GetStopInfo(std::string_view stop_name) const;

	// Добавляет новую остановку в транспортный справочник
	void AddStop(const std::string& name, geo::Coordinates coords);

	// Добавляет расстояние между двумя остановками в справочник
	void SetStopDistances(std::string_view from, std::string_view to, int distance);

	// Добавляет новый автобусный маршрут в транспортный справочник
	void AddBus(const std::string& name, const std::vector<std::string_view>& stops_names);

private:
	std::deque<Stop> stops_; // < набор остановок
	std::deque<Bus> buses_; // < набор автобусных маршрутов

	std::unordered_map<std::string_view, const Stop*> stop_by_name_; // < набор указателей на остановки по их имени
	std::unordered_map<std::string_view, const Bus*> bus_by_name_; // < набор указателей на автобусные маршруты по их имени
	std::unordered_map<std::string_view, std::unordered_set<const Bus*>> buses_on_stop_; // < набор автобусных маршрутов, проходящих через определенную остановку, по имени остановки
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsPairHasher> stop_to_stop_; // < набор расстояний от одной остановки к другой
};

}