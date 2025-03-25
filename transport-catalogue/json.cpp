#include "json.h"

#include <unordered_map>

using namespace std;

namespace json {

namespace {

// Загружает узел JSON-дерева из входного потока.
Node LoadNode(istream& input);

// Загружает массив JSON из входного потока
Node LoadArray(std::istream& input) {
    Array result;
    char c;
    if (!(input >> c)) throw ParsingError("Error in parsing array");
    
    while (c != ']') {
        if (c != ',' && !std::isspace(c)) {
            input.putback(c);
            result.push_back(LoadNode(input));
        }
        if (!(input >> c)) throw ParsingError("Error in parsing array");
    }
    return Node(std::move(result));
}

// Загружает число (int или double) из входного потока
Node LoadNumber(std::istream& input) {
    using namespace std::literals;
    std::string parsed_num;
    
    auto read_char = [&]() {
        parsed_num += static_cast<char>(input.get());
        if (!input) throw ParsingError("Failed to read number from stream");
    };
    
    auto read_digits = [&]() {
        if (!std::isdigit(input.peek())) throw ParsingError("A digit is expected");
        while (std::isdigit(input.peek())) read_char();
    };
    
    if (input.peek() == '-') read_char();
    (input.peek() == '0') ? read_char() : read_digits();
    
    bool is_int = true;
    if (input.peek() == '.') { read_char(); read_digits(); is_int = false; }
    
    if (std::tolower(input.peek()) == 'e') {
        read_char();
        if (input.peek() == '+' || input.peek() == '-') read_char();
        read_digits();
        is_int = false;
    }
    
    try {
        return is_int ? Node(std::stoi(parsed_num)) : Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number");
    }
}

// Загружает строку из входного потока
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    std::string s;
    for (char ch; input.get(ch);) {
        if (ch == '"') return s;
        if (ch == '\\') {
            if (!input.get(ch)) throw ParsingError("String parsing error");
            switch (ch) {
                case 'n': s += '\n'; break;
                case 't': s += '\t'; break;
                case 'r': s += '\r'; break;
                case '"': case '\\': s += ch; break;
                default: throw ParsingError("Unrecognized escape sequence \\"s + ch);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line in string");
        } else {
            s += ch;
        }
    }
    throw ParsingError("String parsing error");
}

// Загружает словарь из входного потока
Node LoadDict(std::istream& input) {
    Dict result;
    char c;
    if (!(input >> c)) throw ParsingError("Error in parsing object");
    
    while (c != '}') {
        if (c != ',' && !std::isspace(c)) {
            if (c != '"') throw ParsingError("Expected string key in object");
            std::string key = LoadString(input);
            if (!(input >> c) || c != ':') throw ParsingError("Expected ':' after key");
            result.emplace(std::move(key), LoadNode(input));
        }
        if (!(input >> c)) throw ParsingError("Error in parsing object");
    }
    return Node(std::move(result));
}

// Загружает литералы null, false, true
Node LoadLiteral(std::istream& input, const std::string& expected, Node value) {
    for (size_t i = 1; i < expected.size(); ++i) {
        if (input.get() != expected[i]) throw ParsingError("Error in parsing '" + expected + "'");
    }
    if (std::isalnum(input.peek())) throw ParsingError("Unexpected characters after '" + expected + "'");
    return value;
}

// Определяет тип загружаемого узла и вызывают соответствующий обработчик
Node LoadNode(std::istream& input) {
    char c;
    input >> c;
    
    switch (c) {
        case '[': return LoadArray(input);
        case '{': return LoadDict(input);
        case '"': return Node(LoadString(input));
        case 'n': return LoadLiteral(input, "null", Node());
        case 't': return LoadLiteral(input, "true", Node(true));
        case 'f': return LoadLiteral(input, "false", Node(false));
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}

}  // namespace

void NodePrinter::operator() ([[maybe_unused]] const nullptr_t value) {
    out << "null"sv;
}

// Реализация посетителя для вывода содержимого JSON-узла в поток

void NodePrinter::operator() (const string& value) {
    static const std::unordered_map<char, std::string_view> chars_map = {
        {'\n', "\\n"},
        {'\r', "\\r"},
        {'\t', "\\t"},
        {'\\', "\\\\"},
        {'\"', "\\\""}
    };

    out << "\""sv;
    for (char c : value) {
        auto it = chars_map.find(c);
        if (it != chars_map.end()) {
            out << it->second;
        } else {
            out << c;
        }
    }
    out << "\""sv;
}

void NodePrinter::operator() (const bool value) {
    out << boolalpha << value;
}

void NodePrinter::operator() (const Array& value) {
    out << "["sv;
    bool is_first = true;
    for (const Node& node : value) {
        if (!is_first) {
            out << ", "sv;
        }
        visit(NodePrinter{out}, node.GetValue());
        is_first = false;
    }
    out << "]"sv;
}

void NodePrinter::operator() (const Dict& value) {
    out << "{"sv;
    bool is_first = true;
    for (const auto& [key, node] : value) {
        if (!is_first) {
            out << ", "sv;
        }
        visit(NodePrinter{out}, Node(key).GetValue());
        out << ": "sv;
        visit(NodePrinter{out}, node.GetValue());
        is_first = false;
    }
    out << "}"sv;
}

// Конец реализации посетителя

// Реализация методов класса, представляющего JSON-узел

Node::Node(std::initializer_list<Node> init) 
    : node_(init.size() == 1 ? init.begin()->node_ : Array(init)) {}

Node::Node(std::initializer_list<std::pair<const string, Node>> init) : node_(Dict(init)) {}

const Node::Value& Node::GetValue() const {
    return node_;
}

bool Node::IsInt() const {
    return holds_alternative<int>(node_);
}

bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(node_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(node_);
}

bool Node::IsString() const {
    return holds_alternative<string>(node_);
}

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(node_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(node_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(node_);
}

int Node::AsInt() const {
    if (IsInt()) {
        return get<int>(node_);
    } else {
        throw logic_error("The type is not suitable");
    }
}

bool Node::AsBool() const {
    if (IsBool()) {
        return get<bool>(node_);
    } else {
        throw logic_error("The type is not suitable");
    }
}

double Node::AsDouble() const {
    if (IsDouble()) {
        if (IsPureDouble()) {
            return get<double>(node_);
        } else {
            return get<int>(node_);
        }
    } else {
        throw logic_error("The type is not suitable");
    }
}

const string& Node::AsString() const {
    if (IsString()) {
        return get<string>(node_);
    } else {
        throw logic_error("The type is not suitable");
    }
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return get<Array>(node_);
    } else {
        throw logic_error("The type is not suitable");
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return get<Dict>(node_);
    } else {
        throw logic_error("The type is not suitable");
    }
}

bool Node::operator== (const Node& other) const {
    return node_ == other.node_;
}

bool Node::operator!= (const Node& other) const {
    return !(*this == other);
}

// Конец реализации класса, представляющего JSON-узел

// Реализация класса, представляющего JSON-документ

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return this->GetRoot() == other.GetRoot();
}

// Конец реализации класса, представляющего JSON-документ

// Загружает JSON-документ из входного потока
Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// Вывод JSON- документа в поток вывода
void Print(const Document& doc, std::ostream& output) {
    visit(NodePrinter{output}, doc.GetRoot().GetValue());
}

}  // namespace json