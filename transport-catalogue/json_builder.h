#pragma once

#include <optional>

#include "json.h"

namespace json {

class Builder;
class KeyContext;
class StartDictContext;
class StartArrayContext;

class Context {
public:
    Context(Builder& builder) : builder_(builder) {}

    KeyContext Key(std::string str);
    Builder& Value(Val val);  
    StartDictContext StartDict();
    Builder& EndDict();
    StartArrayContext StartArray();
    Builder& EndArray();
    Node Build();
    Builder& GetBuilder();

private:
    Builder& builder_;
};

class ValueAfterKeyContext final : public Context {
public:
    ValueAfterKeyContext(Builder& builder) : Context(builder) {}

    Builder& Value(Val val) = delete;
    StartDictContext& StartDict() = delete;
    StartArrayContext& StartArray() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
};

class ValueInArrayContext final : public Context {
public:
    ValueInArrayContext(Builder& builder) : Context(builder) {}

    ValueInArrayContext Value(Val val); 
    KeyContext& Key(std::string str) = delete;
    Builder& EndDict() = delete;
    Node Build() = delete;
};

class KeyContext final : public Context {
public:
    KeyContext(Builder& builder) : Context(builder) {}

    ValueAfterKeyContext Value(Val val);  
    KeyContext& Key(std::string str) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
};

class StartDictContext final : public Context {
public:
    StartDictContext(Builder& builder) : Context(builder) {}

    Builder& Value(Val val) = delete;
    StartDictContext& StartDict() = delete;
    StartArrayContext& StartArray() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
};

class StartArrayContext final : public Context {
public:
    StartArrayContext(Builder& builder) : Context(builder) {}

    ValueInArrayContext Value(Val val);  
    KeyContext& Key(std::string str) = delete;
    Builder& EndDict() = delete;
    Node Build() = delete;
};

class Builder {
public:
    KeyContext Key(std::string str);
    Builder& Value(Val val);
    
    StartDictContext StartDict();
    Builder& EndDict();
    StartArrayContext StartArray();
    Builder& EndArray();

    Node Build();

private:
    std::optional<std::string> key_ = std::nullopt;
    std::optional<Node> root_ = std::nullopt;
    std::vector<Node*> nodes_stack_;
};

} // namespace json