#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace stat_reader {

// Парсит запрос на чтение из транспортного справочника в структуру RequestDescription 
RequestDescription ParseRequest(string_view request) {
    auto space_pos = request.find(' ');
    if (space_pos == request.npos) {
        return {};
    }

    auto not_space = request.find_first_not_of(' ', space_pos);
    if (not_space == request.npos) {
        return {};
    }

    return {string(request.substr(0, space_pos)),
            string(request.substr(not_space))};
}

// Выводит в поток информацию об автобусном маршруте
void PrintBusInfo(string_view bus_id, const transport_catalogue::TransportCatalogue& catalogue, string_view id, ostream& output) {
    transport_catalogue::BusInfo bus_info = catalogue.GetBusInfo(id);

    if (bus_info.num_of_stops) {
        output << setprecision(6);
        output << "Bus "s << bus_id << ": "s << bus_info.num_of_stops << " stops on route, "s <<
                bus_info.num_of_unique_stops << " unique stops, "s << bus_info.route_length << " route length, "s << 
                bus_info.curvature << " curvature" << endl;
    } else {
        output << "Bus "s << bus_id << ": not found"s << endl;
    }
}

// Выводит в поток информацию об остановке
void PrintStopInfo(string_view stop_id, const transport_catalogue::TransportCatalogue& catalogue, string_view id, ostream& output) {
    const unordered_set<const transport_catalogue::Bus*>& stop_info = catalogue.GetStopInfo(id);

    if (&stop_info != &(catalogue.GetStopInfo(""))) {
        if (stop_info.empty()) {
            output << "Stop "s << stop_id << ": no buses"s << endl;
        } else {
            output << "Stop "s << stop_id << ": buses "s;
            vector<const transport_catalogue::Bus*> buses(stop_info.begin(), stop_info.end());
            sort(buses.begin(), buses.end(), [](const transport_catalogue::Bus* lhs, const transport_catalogue::Bus* rhs) {
                return lhs->name < rhs->name;
            });
            for (const transport_catalogue::Bus* bus : buses) {
                output << bus->name << " "s;
            }
            output << endl;
        }
    } else {
        output << "Stop "s << stop_id << ": not found"s << endl;
    }
}

// Выполняет запрос над транспортным справочником и выводит информацию в поток
void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue, string_view request,
                       ostream& output) {
    RequestDescription command = ParseRequest(request);

    if (command.name == "Bus"sv) {
        PrintBusInfo(command.id, catalogue, command.id, output);
    } else if (command.name == "Stop"sv) {
        PrintStopInfo(command.id, catalogue, command.id, output);
    }
}

}