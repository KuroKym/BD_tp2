#ifndef HASH_H
#define HASH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>


#define TITULO 300
#define AUTOR 150
#define UPDATED 20
#define SNIPPET 1024
struct BlockHeader {
    int recordCount;
};

// Estrutura que representa um artigo
struct Article {
    int id;
    char title[TITULO];
    int year;
    char authors[AUTOR];
    int citations;
    char updated_at[UPDATED]; // Armazenar a data como string fixa
    char snippet[SNIPPET];

    // Função para imprimir os dados do artigo
    void print() const {
        std::cout << "ID: " << id << ", \nTitle: " << title 
                  << ", \nYear: " << year << ", \nAuthors: " << authors 
                  << ", \nitations: " << citations 
                  << ", \nUpdated At: " << updated_at 
                  << ", \nSnippet: " << snippet << std::endl;
    }
};

constexpr size_t NUM_BUCKETS = 100000;
constexpr size_t BLOCK_SIZE = 4096; // 4KB por bloco
constexpr size_t RECORDS_PER_BLOCK = BLOCK_SIZE / sizeof(Article);
constexpr size_t BLOCKS_PER_BUCKET = 10;

#define RECORDS = RECORDS_PER_BLOCK
struct Block {
    BlockHeader header;  // Cabeçalho do bloco, que inclui a contagem de registros
    Article records[RECORDS_PER_BLOCK];  // Array de registros do bloco (máximo RECORDS_PER_BLOCK artigos)

    // Função para verificar se o bloco está cheio
    bool isFull() const {
        return header.recordCount >= RECORDS_PER_BLOCK;
    }

    // Função para adicionar um artigo ao bloco
    bool addArticle(const Article& article) {
        if (isFull()) {
            return false;  // Bloco cheio, não pode adicionar mais artigos
        }
        records[header.recordCount++] = article;
        return true;
    }

    // Função para imprimir os artigos do bloco
    void printArticles() const {
        for (int i = 0; i < header.recordCount; ++i) {
            records[i].print();
        }
    }
};

// Funções de manipulação
int hashFunction(int id);
std::string removeQuotes(const std::string& str);
bool is_number(const std::string& str);

void initializeBuckets(const std::string& filename);
std::vector<Article> processarCSV(const std::string& file_path);
void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename);
std::vector<Article> readBucket(int bucket, const std::string& bucket_filename, const std::string& overflow_filename);
int binarySearchInBlock(const Block& block, int id);
int buscarPorTitulo(const Block& block, const std::string& title);
#endif