#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "geo.h"
#include "transport_catalogue.h"

namespace input_reader {

// Структура описания запроса на добавление в транспортный справочние
struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    operator bool() const {
        return !name.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string name;      // < название команды
    std::string id;           // < id маршрута или остановки
    std::string description;  // < параметры команды
};

// Класс чтения входных данных
class InputReader {
public:
    // Парсит строку в структуру CommandDescription и сохраняет результат в commands_
    void ParseLine(std::string_view line);

    // Наполняет данными транспортный справочник, используя команды из commands_
    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

}