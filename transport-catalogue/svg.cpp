#include <utility>

#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<< (std::ostream& out, Rgb color) {
    out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
    return out;
}

std::ostream& operator<< (std::ostream& out, Rgba color) {
    out << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity << ")"sv;
    return out;
}

std::ostream& operator<< (std::ostream& out, Color color) {
    std::visit(ColorPrinter(out), color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
    static const std::unordered_map<StrokeLineCap, std::string_view> line_cap_map = {
        {StrokeLineCap::BUTT, "butt"},
        {StrokeLineCap::ROUND, "round"},
        {StrokeLineCap::SQUARE, "square"},
    };

    out << line_cap_map.at(line_cap);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
    static const std::unordered_map<StrokeLineJoin, std::string_view> line_join_map = {
        {StrokeLineJoin::ARCS, "arcs"},
        {StrokeLineJoin::BEVEL, "bevel"},
        {StrokeLineJoin::MITER, "miter"},
        {StrokeLineJoin::MITER_CLIP, "miter-clip"},
        {StrokeLineJoin::ROUND, "round"}
    };

    out << line_join_map.at(line_join);
    return out;
}

void ColorPrinter::operator() (std::monostate) const {
    out << "none"sv;
}

void ColorPrinter::operator() (std::string color) const {
    out << color;
}

void ColorPrinter::operator() (Rgb color) const {
    out << color;
}

void ColorPrinter::operator() (Rgba color) const {
    out << color;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// -------- PolyLine ---------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first_point = true;
    for (Point point : points_) {
        if (!is_first_point) {
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        is_first_point = false;
    }
    out << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// -------- Text -----------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    static const std::unordered_map<char, std::string_view> chars_map = {
        {'\"', "&quot;"},
        {'\'', "&apos;"},
        {'<', "&lt;"},
        {'>', "&gt;"},
        {'&', "&amp;"}
    };

    auto& out = context.out;
    out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }
    RenderAttrs(context.out);
    out << ">"sv;
    for (char c : data_) {
        auto it = chars_map.find(c);
        if (it != chars_map.end()) {
            out << it->second;
        } else {
            out << c;
        }
    }
    out << "</text>"sv;
}

// ------------- Document -------------

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (const auto& object : objects_) {
        object->Render({out, 1, 2});
    }
    out << "</svg>";
}

}  // namespace svg