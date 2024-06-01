#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

//------------PrintColor-------------

void PrintColor::operator()(std::monostate) const{
    PrintColor::out << "none"sv;
}

void PrintColor::operator()(std::string color) const{
    PrintColor::out << color;
}

void PrintColor::operator()(svg::Rgb rgb) const{
    PrintColor::out << "rgb("sv << rgb.red << ","sv << rgb.green << ","sv << rgb.blue << ")"sv;
}

void PrintColor::operator()(svg::Rgba rgba) const{
    PrintColor::out << "rgba("sv << rgba.red << ","sv << rgba.green << ","sv << rgba.blue << ","sv << rgba.opacity << ")"sv;
}

//------------std::ostream-------------

std::ostream& operator<<(std::ostream& out, Color color){
    std::visit(PrintColor{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap){
    switch (static_cast<int>(line_cap)){
    case 0:
        out << "butt"sv;
        break;
    case 1:
        out << "round"sv;
        break;
    case 2:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join){
    switch (static_cast<int>(line_join)){
        case 0:
            out << "arcs"sv;
            break;
        case 1:
            out << "bevel"sv;
            break;
        case 2:
            out << "miter"sv;
            break;
        case 3:
            out << "miter-clip"sv;
            break;
        case 4:
            out << "round"sv;
            break;
    }
    return out;
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
    RenderAttrs(out);
    out << "/>"sv;
}

//-------------Polyline--------------------

Polyline& Polyline::AddPoint(Point point){
    points.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    bool first = false;
    out << "<polyline points=\""sv;
    for(auto point : points){
        if(first){
            out << " "sv;
        }
        first = true;
        out << point.x << ","sv << point.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//------------Text------------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data = ""){
    data = data.substr(data.find_first_not_of(" "));
    data = data.substr(0, data.find_last_not_of(" ") + 1);

    for(char c : data){
        if(c == ' ' && data.empty()){
            continue;
        }
        else if(c == '\"'){
            data_ += "&quot;";
        }
        else if(c == '\''){
            data_ += "&apos;";
        }
        else if(c == '<'){
            data_ += "&lt;";
        }
        else if(c == '>'){
            data_ += "&gt;";
        }
        else if(c == '&'){
            data_ += "&amp;";
        }
        else {
            data_ += c;
        }
    }
    return *this;
}

void Text::RenderObject(const RenderContext& context) const{
    std::ostream& out = context.out;
    out << "<text x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" "
        << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" "
        << "font-size=\"" << size_ << "\"";
    if(!font_family_.empty()){
        out << " font-family=\"" << font_family_ << "\"";
    }
    if(!font_weight_.empty()){
        out << " font-weight=\"" << font_weight_ << "\"";
    }
    RenderAttrs(out);
    out << ">" << data_ << "</text>";
}

//--------------Document----------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_ptr_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for(size_t i = 0; i < static_cast<size_t>(objects_ptr_.size()); i++){
        objects_ptr_.at(static_cast<size_t>(i))->Render(out);
    }
    out << "</svg>";
}

}  // namespace svg