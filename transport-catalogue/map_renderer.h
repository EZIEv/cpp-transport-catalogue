#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

} // namespace

namespace map_renderer {

struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    int bus_label_font_size;
    svg::Point bus_label_offset;

    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    // Рендерит карту с выводом в поток
    void Render(std::ostream& out, const std::set<const domain::Bus*, domain::BusPointerComparator>& buses);

    // Задание настроек для рендера
    void SetRenderSettings(const RenderSettings& settings);

private:
    // Отрисовывает автобусные маршруты
    void DrawBusLines(svg::Document& doc, const std::set<const domain::Bus*, domain::BusPointerComparator>& buses) const;

    // Отрисовывает подписи к автобусным маршрутам
    void DrawBusLabels(svg::Document& doc, const std::set<const domain::Bus*, domain::BusPointerComparator>& buses) const;

    // Отрисовывает остановки, через которые проходят автобусные маршруты
    void DrawStopCircles(svg::Document& doc, const std::set<const domain::Stop*, domain::StopPointerComparator>& stops) const;

    // Отрисовывает названия остановок
    void DrawStopLabels(svg::Document& doc, const std::set<const domain::Stop*, domain::StopPointerComparator>& stops) const;

    RenderSettings settings_; // < настройки рендера
    std::unordered_map<std::string_view, svg::Point> screen_coords_by_stop_name_; // < спроецированные координаты остановок на плоскость по их имени
};

} // namespace map_renderer