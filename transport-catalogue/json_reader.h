#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json_reader {

// Парсит входной json с запросами
void ParseRequest(std::istream& input, request_handler::RequestHandler& rh, map_renderer::MapRenderer& mr);

// Выводит в поток полученную статистику в формате json
void PrintStat(std::ostream& output, const std::deque<request_handler::StatResponse>& stats);

} // namespace json_reader