#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

enum TokenType
{
    INPUT,    // 'i'
    OUTPUT,   // 'btw'
    SUB_CELL, // 'not'
    ADD_CELL, // 'use'
    SUB_PTR,  // 'notarch'
    ADD_PTR,  // 'arch'
    BEG_LOOP, // '['
    END_LOOP, // ']'
};

struct Token
{
    TokenType type;
};

struct AdvanceResult
{
    unsigned int position = 0;
    char currentChar;
};

bool endsWith(const std::string& str, const std::string& suffix);

std::vector<Token> lex(const std::string& src);
void advance(AdvanceResult* result, const std::string& src);
Token makeToken(AdvanceResult* result, const std::string& src);

void parse(std::vector<Token> tokens);

int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cerr << "ERR: Usage: iusearchbtw <file.archbtw>" << std::endl;
        return 1;
    }

    std::ifstream file;
    file.open(argv[1]);

    if(!file.is_open()) {
        std::cerr << "ERR: Failed to open source file." << std::endl;
        return 1;
    }

    if(!endsWith(argv[1], ".archbtw")) {
        std::cerr << "ERR: Invalid file type." << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << file.rdbuf();

    std::vector<Token> lexResult = lex(ss.str());

    // std::cout << "[ ";
    // for(unsigned int i = 0; i < lexResult.size(); ++i) {
    //     std::cout << lexResult.at(i).type << " ";
    // }
    // std::cout << "]" << std::endl;;


    parse(lexResult);

    std::cout << std::endl;

    return 0;
}

bool endsWith(const std::string& str, const std::string& suffix)
{
    if(suffix.size() > str.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::vector<Token> lex(const std::string& src)
{
    AdvanceResult result;
    result.position = -1;
    result.currentChar = 0;

    advance(&result, src);

    std::vector<Token> tokens;
    while(result.currentChar != 0) {
        if(result.currentChar == ' ' || result.currentChar == '\t' || result.currentChar == '\n' || result.currentChar == '\r') {
            advance(&result, src);
        }
        else if(isalpha(result.currentChar)) {
            tokens.push_back(makeToken(&result, src));
        }
        else if(result.currentChar == '[') {
            Token token;
            token.type = TokenType::BEG_LOOP;
            tokens.push_back(token);
            advance(&result, src);
        }
        else if(result.currentChar == ']') {
            Token token;
            token.type = TokenType::END_LOOP;
            tokens.push_back(token);
            advance(&result, src);
        } 
    }

    return tokens;
}

void advance(AdvanceResult* result, const std::string& src)
{
    result->position++;
    if(result->position < src.length()) {
        result->currentChar = src[result->position];
    } else {
        result->currentChar = 0;
    }
}

Token makeToken(AdvanceResult* result, const std::string& src)
{
    std::string tokenStr;
    while(result->currentChar != 0 && (isalpha(result->currentChar))) {
        tokenStr += result->currentChar;
        advance(result, src);
    }

    TokenType type;
    if(tokenStr == "i") {
        type = TokenType::INPUT;
    }
    else if(tokenStr == "btw") {
        type = TokenType::OUTPUT;
    }
    else if(tokenStr == "not") {
        type = TokenType::SUB_CELL;
    }
    else if(tokenStr == "use") {
        type = TokenType::ADD_CELL;
    }
    else if(tokenStr == "notarch") {
        type = TokenType::SUB_PTR;
    }
    else if(tokenStr == "arch") {
        type = TokenType::ADD_PTR;
    }

    Token token;
    token.type = type;
    return token;
}

void parse(std::vector<Token> tokens)
{
    unsigned int CELLS_BEGIN = 0;
    unsigned int CELLS_END = 30000;
    unsigned int MIN_CELL_SIZE = 0;
    unsigned int MAX_CELL_SIZE = 256;

    std::vector<int> memory(CELLS_END, 0);
    unsigned int ptr = MIN_CELL_SIZE;

    unsigned int i = 0;
    while(i < tokens.size()) {
        Token token = tokens.at(i);
        if(token.type == TokenType::INPUT) {
            std::cin >> memory[ptr];
            if(memory[ptr] < MIN_CELL_SIZE) {
                memory[ptr] = MAX_CELL_SIZE;
            } 
            else if(memory[ptr] > MAX_CELL_SIZE) {
                memory[ptr] = MIN_CELL_SIZE;
            }
        }
        else if(token.type == TokenType::OUTPUT) {
            std::cout << (char)memory[ptr];
        }
        else if(token.type == TokenType::SUB_CELL) {
            memory[ptr]--;
            if(memory[ptr] < MIN_CELL_SIZE) {
                memory[ptr] = MAX_CELL_SIZE;
            } 
        }
        else if(token.type == TokenType::ADD_CELL) {
            memory[ptr]++;
            if(memory[ptr] > MAX_CELL_SIZE) {
                memory[ptr] = MIN_CELL_SIZE;
            }
        }
        else if(token.type == TokenType::SUB_PTR) {
            ptr--;
            if(ptr < CELLS_BEGIN) {
                ptr = CELLS_END;
            }
        }
        else if(token.type == TokenType::ADD_PTR) {
            ptr++;
            if(ptr > CELLS_END) {
                ptr = CELLS_BEGIN;
            }
        }
        else if(token.type == TokenType::BEG_LOOP) {
            if(!memory[ptr]) {
                int open = 1;
                while(open) {
                    if(++i == -1) {
                        std::cerr << "ERR: Loop went out of scope." << std::endl;
                        exit(1);
                    }
                    if(tokens.at(i).type == TokenType::BEG_LOOP) {
                        open++;
                    }
                    if(tokens.at(i).type == TokenType::END_LOOP) {
                        open--;
                    }
                }
            }
        }
        else if(token.type == TokenType::END_LOOP) {
            if(memory[ptr]) {
                int close = 1;
                while(close) {
                    if(--i == -1) {
                        std::cerr << "ERR: Loop went out of scope." << std::endl;
                        exit(1);
                    }
                    if(tokens.at(i).type == TokenType::END_LOOP) {
                        close++;
                    }
                    if(tokens.at(i).type == TokenType::BEG_LOOP) {
                        close--;
                    }
                }
            }
        } else {
            std::cerr << "ERR: Invalid Token `" << token.type << "`" << std::endl;
            exit(1);
        }
        i++;
    }
}