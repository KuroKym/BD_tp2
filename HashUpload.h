#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Definições de constantes
#define NUM_BUCKETS 100
#define BLOCK_SIZE 4096 // 4KB por bloco
#define BLOCKS_PER_BUCKET 10

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
#define RECORDS_PER_BLOCK (BLOCK_SIZE / sizeof(Article))

// Estrutura que representa o cabeçalho de um bloco
struct BlockHeader {
    int recordCount;
};

// Funções de manipulação
int hashFunction(int id);
std::string removeQuotes(const std::string& str);
bool is_number(const std::string& str);

void initializeBuckets(const std::string& filename);
std::vector<Article> processarCSV(const std::string& file_path);
void insertRecord(const Article& article, const std::string& bucket_filename, const std::string& overflow_filename);
void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename);
std::vector<Article> readBucket(int bucket, const std::string& bucket_filename, const std::string& overflow_filename);

