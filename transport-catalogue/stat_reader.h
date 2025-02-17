<<<<<<< HEAD
// место для вашего кода
=======
#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {

// Структура запроса информации из транспортного справочник
struct RequestDescription {
    std::string name; // < имя коммандЫ
    std::string id; // < id запроса
};

// Выполняет запрос над транспортным справочником и выводит информацию в поток
void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);

}
>>>>>>> master
