#pragma once

#include <string>
#include <unordered_map>

#include "location.hpp"

class Token {
  public:
    enum Type {
        // keywords
        K_IMPORT,
        K_EXPR,
        K_TYPE,
        K_FN,
        K_CAST,
        K_IF,
        K_ELSE,
        K_NEW,
        K_CALL,
        K_EQ,
        K_GRE,
        K_LES,
        K_AND,
        K_OR,
        K_EXTERN,
        K_AS,

        // literals
        WORD,
        NUMBER,
        FLOATING_POINT,
        STRING,
        CHAR,

        // symbol
        SYM_ARROW,
        SYM_DOUBLE_COLON,

        // single char / delimiters
        L_PAREN,
        R_PAREN,
        L_BRACE,
        R_BRACE,
        COLON,
        PERIOD,
        AT_SIGN,
        POINTER,
        EQUALS,
        BACK_SLASH
    };

    Token(std::string value, Type type, Location loc)
        : loc(loc), value(value), type(type) {}

    Location    loc;
    std::string value;
    Type        type;

    static inline const std::unordered_map<char, Type> single_chars = {
        {'(', L_PAREN}, {')', R_PAREN},    {'[', L_BRACE}, {']', R_BRACE},
        {':', COLON},   {'.', PERIOD},     {'@', AT_SIGN}, {'^', POINTER},
        {'=', EQUALS},  {'\\', BACK_SLASH}};

    static inline const std::unordered_map<std::string, Type> symbols = {
        {"::", SYM_DOUBLE_COLON}, {"->", SYM_ARROW}};

    static inline const std::unordered_map<std::string, Type> keywords = {
        {"import", K_IMPORT}, {"expr", K_EXPR}, {"type", K_TYPE},
        {"fn", K_FN},         {"cast", K_CAST}, {"if", K_IF},
        {"else", K_ELSE},     {"new", K_NEW},   {"call", K_CALL},
        {"eq", K_EQ},         {"gre", K_GRE},   {"les", K_LES},
        {"and", K_AND},       {"or", K_OR},     {"extern", K_EXTERN},
        {"as", K_AS}};
};
