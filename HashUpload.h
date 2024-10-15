#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>

struct BlockHeader {
    int recordCount;
};

// Estrutura que representa um artigo
struct Article {
    int id;
    char title[300];
    int year;
    char authors[150];
    int citations;
    char updated_at[20]; // Armazenar a data como string fixa
    char snippet[1024];

    // Função para imprimir os dados do artigo
    void print() const {
        std::cout << "ID: " << id << ", Title: " << title 
                  << ", Year: " << year << ", Authors: " << authors 
                  << ", Citations: " << citations 
                  << ", Updated At: " << updated_at 
                  << ", Snippet: " << snippet << std::endl;
    }
};

constexpr size_t NUM_BUCKETS = 100;
constexpr size_t BLOCK_SIZE = 4096; // 4KB por bloco
constexpr size_t RECORDS_PER_BLOCK = BLOCK_SIZE / sizeof(Article);
constexpr size_t BLOCKS_PER_BUCKET = 10;

/* Definições de constantes

constexpr usado devido à ambiguidade no cálculo do C++
Struct Article tem que ser declarada antes do "RECORDS_PER_BLOCK", pois utiliza o Article para calcular o tamanho

#define NUM_BUCKETS 100
#define BLOCK_SIZE 4096 // 4KB por bloco
#define BLOCKS_PER_BUCKET 10
#define RECORDS_PER_BLOCK (BLOCK_SIZE / sizeof(Article))*/

// Funções de manipulação
int hashFunction(int id);
std::string removeQuotes(const std::string& str);
bool is_number(const std::string& str);

void initializeBuckets(const std::string& filename);
std::vector<Article> processarCSV(const std::string& file_path);
void insertRecord(const Article& article, const std::string& bucket_filename, const std::string& overflow_filename);
void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename);
std::vector<Article> readBucket(int bucket, const std::string& bucket_filename, const std::string& overflow_filename);
