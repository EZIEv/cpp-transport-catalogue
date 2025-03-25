#pragma once

#include <set>
#include <string>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
	std::string name; // < название остановки
	geo::Coordinates coords; // < координаты остановки
};

struct Bus {
	std::string name; // < название автобусного маршрута
	std::vector<const Stop*> stops; // < набор остановок на маршруте
	bool is_roundtrip; // < флаг типа маршрута (true - кольцевой, false - некольцевой)
};

struct BusInfo {
	int num_of_stops; // < количество остановок на маршруте
	int num_of_unique_stops; // < количество уникальных остановок на маршруте;
	double route_length; // < реальная длина маршрута
	double curvature; // < кривизна маршрута (отношение реальной длины к географической)
};

// Компаратор для сортировки указателей на Bus по имени маршрута
struct BusPointerComparator {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}
};

// Компаратор для сортировки указателей на Stop по имени остановки
struct StopPointerComparator {
	bool operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name < rhs->name;
	}
};

using StopInfo = std::set<const Bus*, BusPointerComparator>;

} // namespace domain