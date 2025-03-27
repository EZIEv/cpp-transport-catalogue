#include "map_renderer.h"

#include <deque>

using namespace std;

namespace {

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

} // namespace

namespace map_renderer {

// Задание настроек для рендера
void MapRenderer::SetRenderSettings(const RenderSettings& settings) {
    settings_ = settings;
}

// Отрисовывает автобусные маршруты
void MapRenderer::DrawBusLines(svg::Document& doc, const set<const domain::Bus*, const domain::BusPointerComparator>& buses) const {
    size_t color_index = 0;
    for (const domain::Bus* bus : buses) {
        if (bus->stops.size() == 0) {
            continue;
        }

        svg::Polyline poly_line;

        poly_line.SetFillColor({});
        poly_line.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()]);
        poly_line.SetStrokeWidth(settings_.line_width);
        poly_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        poly_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const domain::Stop* stop : bus->stops) {
            poly_line.AddPoint(screen_coords_by_stop_name_.at(stop->name));
        }

        doc.Add(poly_line);

        ++color_index;
    }
}

// Отрисовывает подписи к автобусным маршрутам
void MapRenderer::DrawBusLabels(svg::Document& doc, const set<const domain::Bus*, const domain::BusPointerComparator>& buses) const {
    size_t color_index = 0;
    for (const domain::Bus* bus : buses) {
        svg::Text underlayer;
        svg::Text text;

        underlayer.SetPosition(screen_coords_by_stop_name_.at(bus->stops[0]->name));
        text.SetPosition(screen_coords_by_stop_name_.at(bus->stops[0]->name));
        underlayer.SetOffset(settings_.bus_label_offset);
        text.SetOffset(settings_.bus_label_offset);
        underlayer.SetFontSize(settings_.bus_label_font_size);
        text.SetFontSize(settings_.bus_label_font_size);
        underlayer.SetFontFamily("Verdana");
        text.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        text.SetFontWeight("bold");
        underlayer.SetData(bus->name);
        text.SetData(bus->name);

        underlayer.SetFillColor(settings_.underlayer_color);
        underlayer.SetStrokeColor(settings_.underlayer_color);
        underlayer.SetStrokeWidth(settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()]);

        doc.Add(underlayer);
        doc.Add(text);

        // Если маршрут кольцевой или начальная и конечная остановки совпадают, то название маршрута выводим только у начальной остановки
        if (!(bus->is_roundtrip) && bus->stops.size() != 1 && bus->stops[0]->name != bus->stops[bus->stops.size() / 2]->name) {
            underlayer.SetPosition(screen_coords_by_stop_name_.at(bus->stops[bus->stops.size() / 2]->name));
            text.SetPosition(screen_coords_by_stop_name_.at(bus->stops[bus->stops.size() / 2]->name));
            underlayer.SetOffset(settings_.bus_label_offset);
            text.SetOffset(settings_.bus_label_offset);
            underlayer.SetFontSize(settings_.bus_label_font_size);
            text.SetFontSize(settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            text.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            text.SetFontWeight("bold");
            underlayer.SetData(bus->name);
            text.SetData(bus->name);

            underlayer.SetFillColor(settings_.underlayer_color);
            underlayer.SetStrokeColor(settings_.underlayer_color);
            underlayer.SetStrokeWidth(settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text.SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()]);

            doc.Add(underlayer);
            doc.Add(text);
        }

        ++color_index;
    }    
}

// Отрисовывает остановки, через которые проходят автобусные маршруты
void MapRenderer::DrawStopCircles(svg::Document& doc, const set<const domain::Stop*, domain::StopPointerComparator>& stops) const {
    for (const domain::Stop* stop : stops) {
        svg::Circle circle;

        circle.SetCenter(screen_coords_by_stop_name_.at(stop->name));
        circle.SetRadius(settings_.stop_radius);
        circle.SetFillColor("white");

        doc.Add(circle);
    }
}

// Отрисовывает названия остановок
void MapRenderer::DrawStopLabels(svg::Document& doc, const set<const domain::Stop*, domain::StopPointerComparator>& stops) const {
    for (const domain::Stop* stop : stops) {
        svg::Text underlayer;
        svg::Text text;

        underlayer.SetPosition(screen_coords_by_stop_name_.at(stop->name));
        text.SetPosition(screen_coords_by_stop_name_.at(stop->name));
        underlayer.SetOffset(settings_.stop_label_offset);
        text.SetOffset(settings_.stop_label_offset);
        underlayer.SetFontSize(settings_.stop_label_font_size);
        text.SetFontSize(settings_.stop_label_font_size);
        underlayer.SetFontFamily("Verdana");
        text.SetFontFamily("Verdana");
        underlayer.SetData(stop->name);
        text.SetData(stop->name);

        underlayer.SetFillColor(settings_.underlayer_color);
        underlayer.SetStrokeColor(settings_.underlayer_color);
        underlayer.SetStrokeWidth(settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetFillColor({"black"});

        doc.Add(underlayer);
        doc.Add(text);
    }
}

// Рендерит карту с выводом в поток
void MapRenderer::Render(std::ostream& out, const set<const domain::Bus*, const domain::BusPointerComparator>& buses) {
    svg::Document doc;

    // Упорядочиваем все остановки в лексикографическом порядке
    // Создаем поле со всеми координатами остановок
    set<const domain::Stop*, domain::StopPointerComparator> stops;
    deque<geo::Coordinates> geo_coords;
    for (const domain::Bus* bus : buses) {
        if (bus->stops.size() == 0) {
            continue;
        }
        
        for (const domain::Stop* stop : bus->stops) {
            geo_coords.push_back(stop->coords);
            stops.insert(stop);
        }
    }

    // Создадим на основе поля координат объект для проецирования географических координат на плоскость
    const SphereProjector proj{geo_coords.begin(), geo_coords.end(), settings_.width, settings_.height, settings_.padding};

    // Создадим словарь, который по имени остановки возвращает спроецированные координаты
    for (const domain::Stop* stop : stops) {
        screen_coords_by_stop_name_[stop->name] = proj(stop->coords);
    }

    // Отрисуем все необходимые элементы
    DrawBusLines(doc, buses);
    DrawBusLabels(doc, buses);
    DrawStopCircles(doc, stops);
    DrawStopLabels(doc, stops);

    doc.Render(out);
}

} // namespace map_renderer