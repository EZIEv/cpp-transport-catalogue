#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Исключение, выбрасывающиеся при ошибке парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

// Посетитель для вывода содержимого JSON-узла в поток
struct NodePrinter {
    std::ostream& out;

    template <typename Numeric>
    void operator() (const Numeric value) {
        out << value;
    }

    void operator() (const std::nullptr_t value);
    void operator() (const std::string& value);
    void operator() (const bool value);
    void operator() (const Array& value);
    void operator() (const Dict& value);
};

using Val = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

// Класс, представляющий JSON-узел
class Node : public Val {
public:
    Node() : Val(nullptr) {}
    Node(const Val& val) : Val(val) {}
    Node(int value) : Val(value) {}
    Node(double value) : Val(value) {}
    Node(const std::string& value) : Val(value) {}
    Node(std::string&& value) : Val(std::move(value)) {}
    Node(bool value) : Val(value) {}
    Node(const char* value) : Val(std::string(value)) {}
    Node(const Array& value) : Val(value) {}
    Node(Array&& value) : Val(std::move(value)) {}
    Node(const Dict& value) : Val(value) {}
    Node(Dict&& value) : Val(std::move(value)) {}

    // Возвращает ссылку на хранимое значение
    const Val& GetValue() const;
    Val& GetValue();

    // Проверка типа узла
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    // Преобразование узла к конкретному типу
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    std::string& AsString();
    Array& AsArray();
    Dict& AsMap();

    // Операторы сравнения
    bool operator== (const Node& other) const;
    bool operator!= (const Node& other) const;
};

// Класс, представляющий JSON-документ
class Document {
public:
    // Конструктор документа с корневым узлом
    explicit Document(Node root);

    // Возвращает константную ссылку на корневой узел
    const Node& GetRoot() const;

    // Оператор сравнения
    bool operator==(const Document& other) const;

private:
    Node root_;
};

// Загружает JSON-документ из входного потока
Document Load(std::istream& input);

// Вывод JSON- документа в поток вывода
void Print(const Document& doc, std::ostream& output);

}  // namespace json