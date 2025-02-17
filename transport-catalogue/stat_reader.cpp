<<<<<<< HEAD
// место для вашего кода
=======
#include <iomanip>
#include <iosfwd>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

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

// Выполняет запрос над транспортным справочником и выводит информацию в поток
void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, string_view request,
                       ostream& output) {
    RequestDescription command = ParseRequest(request);

    if (command.name == "Bus"sv) {
        try {
            const transport_catalogue::BusInfo& bus_info = tansport_catalogue.GetBusInfo(command.id);

            output << setprecision(6);
            output << "Bus "s << command.id << ": "s << bus_info.num_of_stops << " stops on route, "s <<
                    bus_info.num_of_unique_stops << " unique stops, "s << bus_info.length << " route length "s << endl;
        } catch (const out_of_range& e) {
            output << "Bus "s << command.id << ": not found"s << endl;
        }
    } else if (command.name == "Stop"sv) {
        try {
            const set<string_view> stop_info = tansport_catalogue.GetStopInfo(command.id);
            if (stop_info.empty()) {
                output << "Stop "s << command.id << ": no buses"s << endl;
            } else {
                output << "Stop "s << command.id << ": buses "s;
                for (const string_view bus : stop_info) {
                    output << bus << " "s;
                }
                output << endl;
            }
        } catch (const out_of_range& e) {
            output << "Stop "s << command.id << ": not found"s << endl;
        }
    }
}

}
>>>>>>> master
