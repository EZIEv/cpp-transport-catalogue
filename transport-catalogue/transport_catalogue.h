#pragma once

#include <deque>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>

#include "domain.h"

namespace transport_catalogue {

// Класс-хэшер pair<const Stop*, const Stop*>
class StopsPairHasher {
public:
	size_t operator() (const std::pair<const domain::Stop*, const domain::Stop*>& pair_of_stops) const {
		return (p_hasher_(pair_of_stops.first) + p_hasher_(pair_of_stops.second) * 37);
	}

private:
	std::hash<const void*> p_hasher_;
};

class TransportCatalogue {
public:
	// Возвращает множество всех автобусных маршрутов, отсортированных в лексикографическом порядке
	const std::set<const domain::Bus*, domain::BusPointerComparator>& GetAllBuses() const;

	// Возвращает указатель на остановку по ее имени
	const domain::Stop* GetStop(std::string_view stop_name) const;

	// Возвращает указатель на автобусный маршрут по его имени
	const domain::Bus* GetBus(std::string_view bus_name) const;

	// Возвращает информацию об автобусном маршруте по его имени
	const domain::BusInfo GetBusInfo(std::string_view bus_name) const;

	/* Возвращает константную ссылку на множество автобусных маршрутов, 
	   проходящих через остановку, по имени остановки */
	const domain::StopInfo& GetStopInfo(std::string_view stop_name) const;

	// Добавляет новую остановку в транспортный справочник
	void AddStop(const std::string& name, geo::Coordinates coords);

	// Добавляет расстояние между двумя остановками в справочник
	void SetStopDistances(std::string_view from, std::string_view to, int distance);

	// Добавляет новый автобусный маршрут в транспортный справочник
	void AddBus(const std::string& name, const std::vector<std::string>& stops_names, bool is_roundtrip);

private:
	std::deque<domain::Stop> stops_; // < набор остановок
	std::deque<domain::Bus> buses_; // < набор автобусных маршрутов

	std::set<const domain::Bus*, domain::BusPointerComparator> sorted_buses_; // < набор всех автобусных маршрутов, сортированных по имени в лексикографическом порядке

	std::unordered_map<std::string_view, const domain::Stop*> stop_by_name_; // < набор указателей на остановки по их имени
	std::unordered_map<std::string_view, const domain::Bus*> bus_by_name_; // < набор указателей на автобусные маршруты по их имени

	std::unordered_map<std::string_view, domain::StopInfo> buses_on_stop_; // < набор автобусных маршрутов, проходящих через определенную остановку, по имени остановки

	std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, StopsPairHasher> stop_to_stop_; // < набор расстояний от одной остановки к другой
};

} // namespace transport_catalogue