#pragma once

#include <deque>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {

// Структура остановки
struct Stop {
	std::string name; // < название остановки
	geo::Coordinates coords; // < координаты остановки
};

// Структура информации об автобусном маршруте
struct BusInfo {
	size_t num_of_stops; // < количество остановок на маршруте
	size_t num_of_unique_stops; // < количество уникальных остановок на маршруте;
	double length; // < длина маршрута
};

// Структура автобусного маршрута
struct Bus {
	std::string name; // < название автобусного маршрута
	std::vector<const Stop*> stops; // < набор остановок на маршруте
	BusInfo info; // < информация об автобусном маршруте
};

// Класс-хэшер string_view
class StringViewHasher {
public:
	size_t operator() (const std::string_view str) const {
		return sv_hasher_(str);
	}

private:
	std::hash<std::string_view> sv_hasher_;
};

// Класс-хэшер pair<const Stop*, const Stop*>
class StopsPairHasher {
public:
	size_t operator() (const std::pair<const Stop*, const Stop*>& pair_of_stops) const {
		return (d_hasher_(pair_of_stops.first->coords.lat) + d_hasher_(pair_of_stops.second->coords.lat)) + 
				(d_hasher_(pair_of_stops.first->coords.lng) + d_hasher_(pair_of_stops.second->coords.lng)) * 37;
	}

private:
	std::hash<double> d_hasher_;
};

// Класс транспортного справочника
class TransportCatalogue {
public:
	// Возвращает указатель на остановку по ее имени
	const Stop* GetStop(std::string_view stop_name) const;

	// Возвращает указатель на автобусный маршрут по его имени
	const Bus* GetBus(std::string_view bus_name) const;

	// Возвращает ссылку на информацию об автобусном маршруте по его имени
	const BusInfo& GetBusInfo(std::string_view bus_name) const;

	/* Возвращает ссылку на упорядоченное по алфавиту множество
       наименований автобусных маршрутов, проходящих через остановку,
	   по имени остановки */
	const std::set<std::string_view>& GetStopInfo(std::string_view stop_name) const;

	// Добавляет новую остановку в транспортный справочник
	void AddStop(std::string name, geo::Coordinates coords);

	// Добавляет новый автобусный маршрут в транспортный справочник
	void AddBus(std::string name, std::vector<std::string_view> stops_names);

private:
	std::deque<Stop> stops_; // < набор остановок
	std::deque<Bus> buses_; // < набор автобусных маршрутов

	std::unordered_map<std::string_view, const Stop*, StringViewHasher> stop_by_name_; // < набор указателей на остановки по их имени
	std::unordered_map<std::string_view, const Bus*, StringViewHasher> bus_by_name_; // < набор указателей на автобусные маршруты по их имени
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stop_; // < набор автобусных маршрутов, проходящих через определенную остановку, по имени остановки

	std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopsPairHasher> distances_; // < набор расстояний между остановками по паре двух остановок
};

}