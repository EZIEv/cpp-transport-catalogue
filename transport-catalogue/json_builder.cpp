#include "json_builder.h"

using namespace std;

namespace json {

KeyContext Context::Key(string str) {
    return builder_.Key(move(str));
}

Builder& Context::Value(Val val) {
    return builder_.Value(move(val));
}

StartDictContext Context::StartDict() {
    return builder_.StartDict();
}

Builder& Context::EndDict() {
    return builder_.EndDict();
}

StartArrayContext Context::StartArray() {
    return builder_.StartArray();
}

Builder& Context::EndArray() {
    return builder_.EndArray();
}

Node Context::Build() {
    return builder_.Build();
}

Builder& Context::GetBuilder() {
    return builder_;
}

ValueAfterKeyContext KeyContext::Value(Val val) {
    return ValueAfterKeyContext(GetBuilder().Value(move(val)));
}

ValueInArrayContext StartArrayContext::Value(Val val) {
    return ValueInArrayContext(GetBuilder().Value(move(val)));
}

ValueInArrayContext ValueInArrayContext::Value(Val val) {
    return ValueInArrayContext(GetBuilder().Value(move(val)));
}

Node Builder::Build() {
    if (root_ != nullopt && nodes_stack_.empty()) {
        return *root_;
    }
    throw logic_error("Builder is not ready");
}

KeyContext Builder::Key(std::string str) {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
        if (key_ == nullopt) {
            key_ = move(str);
            return KeyContext(*this);
        }
    }
    throw logic_error("Incorrect usage of method \"Key\"");
}

Builder& Builder::Value(Val val) {
    if (root_ == nullopt && nodes_stack_.empty()) {
        root_ = Node(val); 
        return *this;
    }
    if (!nodes_stack_.empty()) {
        if (key_ != nullopt && nodes_stack_.back()->IsMap()) {
            nodes_stack_.back()->AsMap()[*key_] = Node(val);
            key_ = nullopt;
            return *this;
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(val);
            return *this;
        }
    }
    throw logic_error("Incorrect usage of method \"Value\"");
}

StartDictContext Builder::StartDict() {
    if (root_ == nullopt && nodes_stack_.empty()) {
        root_ = Node(Dict{});
        nodes_stack_.push_back(&(*root_));
        return StartDictContext(*this);
    }
    if (!nodes_stack_.empty()) {
        if (key_ != nullopt && nodes_stack_.back()->IsMap()) {
            nodes_stack_.back()->AsMap()[*key_] = Node(Dict{});
            Node* last_node = &(nodes_stack_.back()->AsMap()[*key_]);
            nodes_stack_.push_back(last_node);
            key_ = nullopt;
            return StartDictContext(*this);
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(Dict{});
            Node* last_node = &(nodes_stack_.back()->AsArray().back());
            nodes_stack_.push_back(last_node);
            return StartDictContext(*this);
        }
    }
    throw logic_error("Incorrect usage of method \"StartDict\"");
}

Builder& Builder::EndDict() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
        nodes_stack_.pop_back();
        return *this;
    }
    throw logic_error("Incorrect usage of method \"EndDict\"");
}

StartArrayContext Builder::StartArray() {
    if (root_ == nullopt && nodes_stack_.empty()) {
        root_ = Node(Array{});
        nodes_stack_.push_back(&(*root_));
        return StartArrayContext(*this);
    }
    if (!nodes_stack_.empty()) {
        if (key_ != nullopt && nodes_stack_.back()->IsMap()) {
            nodes_stack_.back()->AsMap()[*key_] = Node(Array{});
            Node* last_node = &(nodes_stack_.back()->AsMap()[*key_]);
            nodes_stack_.push_back(last_node);
            key_ = nullopt;
            return StartArrayContext(*this);
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(Node(Array{}));
            Node* last_node = &(nodes_stack_.back()->AsArray().back());
            nodes_stack_.push_back(last_node);
            return StartArrayContext(*this);
        }
    }
    throw logic_error("Incorrect usage of method \"StartArray\"");
}

Builder& Builder::EndArray() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        nodes_stack_.pop_back();
        return *this;
    }
    throw logic_error("Incorrect usage of method \"EndArray\"");
}

} // namespace json