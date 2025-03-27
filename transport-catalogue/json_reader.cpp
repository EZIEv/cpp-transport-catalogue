#include "json_reader.h"

using namespace std;

namespace json_reader {

// Пасрит json запрос на добавление остановки
request_handler::StopRequest ParseStopRequest(const json::Dict& request) {
    request_handler::StopRequest stop_request;

    stop_request.name = request.at("name"s).AsString();
    stop_request.coords = {request.at("latitude"s).AsDouble(), request.at("longitude").AsDouble()};
    for (const auto& [stop, distance] : request.at("road_distances"s).AsMap()) {
        stop_request.distances[stop] = distance.AsInt();
    }

    return stop_request;
}

// Парсит json запрос на добаление маршрута
request_handler::BusRequest ParseBusRequest(const json::Dict& request) {
    request_handler::BusRequest bus_request;

    bus_request.name = request.at("name"s).AsString();
    bus_request.is_roundtrip = request.at("is_roundtrip"s).AsBool();
    for (const json::Node& stop : request.at("stops"s).AsArray()) {
        bus_request.stops.push_back(stop.AsString());
    }

    return bus_request;
}

// Парсит базовые запросы на добаление маршрутов и оствновок в каталог
void ParseBaseRequests(const json::Array& base_requests, request_handler::RequestHandler& rh) {
    for (const json::Node& base_request : base_requests) {
        const json::Dict& request = base_request.AsMap();
        const string& request_type = request.at("type"s).AsString();
        if (request_type == "Stop"sv) {
            rh.AddStopRequest(ParseStopRequest(request));
        } else if (request_type == "Bus"sv) {
            rh.AddBusRequest(ParseBusRequest(request));
        }
    }
}

// Парсит запрос на получение статистики
request_handler::StatRequest ParseStatRequest(const json::Dict& request) {
    request_handler::StatRequest stat_request;

    stat_request.id = request.at("id"s).AsInt();
    stat_request.type = request.at("type"s).AsString();
    stat_request.name = stat_request.type == "Map"s ? ""s : request.at("name"s).AsString();

    return stat_request;
}

// Парсит запросы на получение статистики из каталога
void ParseStatRequests(const json::Array& stat_requests, request_handler::RequestHandler& rh) {
    for (const json::Node& stat_request : stat_requests) {
        const json::Dict& request = stat_request.AsMap();
        rh.AddStatRequest(ParseStatRequest(request));
    }
}

// Парсит ноду с цветом
svg::Color ParseColor(const json::Node& json_color) {
    if (json_color.IsString()) {
        return {json_color.AsString()};
    }

    const json::Array& array_color = json_color.AsArray();

    int red = array_color[0].AsInt();
    int green = array_color[1].AsInt();
    int blue = array_color[2].AsInt();

    if (array_color.size() == 3) {
        return {svg::Rgb(red, green, blue)};
    }

    return {svg::Rgba(red, green, blue, array_color[3].AsDouble())};
}

// Парсит настройки рендера карты
void ParsRenderSettings(const json::Dict& render_settings, map_renderer::MapRenderer& mr) {
    map_renderer::RenderSettings settings;

    settings.width = render_settings.at("width"s).AsDouble();
    settings.height = render_settings.at("height"s).AsDouble();

    settings.padding = render_settings.at("padding"s).AsDouble();

    settings.line_width = render_settings.at("line_width"s).AsDouble();
    settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();

    settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    const json::Array& bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
    settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
    const json::Array& stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
    settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    settings.underlayer_color = ParseColor(render_settings.at("underlayer_color"s));
    settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

    for (const json::Node& color : render_settings.at("color_palette"s).AsArray()) {
        settings.color_palette.push_back(ParseColor(color));
    }

    mr.SetRenderSettings(settings);
}

// Парсит все запросы
void ParseRequest(istream& input, request_handler::RequestHandler& rh, map_renderer::MapRenderer& mr) {
    json::Document doc = json::Load(input);

    const json::Dict& requests = doc.GetRoot().AsMap();
    const json::Array& base_requests = requests.at("base_requests"s).AsArray();
    const json::Dict& render_settings = requests.at("render_settings"s).AsMap();
    const json::Array& stat_requests = requests.at("stat_requests"s).AsArray();

    ParseBaseRequests(base_requests, rh);    
    ParsRenderSettings(render_settings, mr);
    ParseStatRequests(stat_requests, rh);
}

// Выводит в поток собранную статистику в формате json
void PrintStat(std::ostream& output, const std::deque<request_handler::StatResponse>& stats) {
    json::Array json_stats;

    for (const request_handler::StatResponse& stat : stats) {
        json::Dict json_stat;

        if (std::holds_alternative<domain::BusInfo>(stat.info)) {
            domain::BusInfo bus_info =  get<domain::BusInfo>(stat.info);
            json_stat["request_id"] = json::Node(stat.id);
            json_stat["curvature"] = json::Node(bus_info.curvature);
            json_stat["route_length"] = json::Node(bus_info.route_length);
            json_stat["stop_count"] = json::Node(bus_info.num_of_stops);
            json_stat["unique_stop_count"] = json::Node(bus_info.num_of_unique_stops);
        } else if (std::holds_alternative<domain::StopInfo>(stat.info)) {
            const domain::StopInfo& stop_info = get<domain::StopInfo>(stat.info);
            json_stat["request_id"] = json::Node(stat.id);
            json::Array buses_on_stop;
            for (const domain::Bus* bus : stop_info) {
                buses_on_stop.push_back(json::Node(bus->name));
            }
            json_stat["buses"] = json::Node(buses_on_stop);
        } else if (std::holds_alternative<string>(stat.info)) {
            json_stat["request_id"] = json::Node(stat.id);
            json_stat["map"] = json::Node(get<string>(stat.info));
        } else {
            json_stat["request_id"] = json::Node(stat.id);
            json_stat["error_message"] = json::Node("not found");
        }

        json_stats.push_back(json::Node(json_stat));
    }

    json::Print(json::Document{json::Node(json_stats)}, output);
}

} // namespace json_reader