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

// Класс, представляющий JSON-узел
class Node {
public:
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    // Конструктор по умолчанию, создает узел с нулевым указателем
    Node() = default;

    // Конструктор узла из переданного значения
    template <typename T>
    Node(T&& v) noexcept : node_(std::forward<T>(v)) {}

    // Конструктор узла из списка инициализации массива
    Node(std::initializer_list<Node> init);

    // Конструктор узла из списка инициализации словаря
    Node(std::initializer_list<std::pair<const std::string, Node>> init);

    // Возвращает константную ссылку на хранимое значение
    const Value& GetValue() const;

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

    // Операторы сравнения
    bool operator== (const Node& other) const;
    bool operator!= (const Node& other) const;

private:
    Value node_ = nullptr;
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