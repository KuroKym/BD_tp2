#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <algorithm>

// Definições de constantes
#define NUM_BUCKETS 100
#define BLOCK_SIZE 4096 // 4KB por bloco
#define RECORDS_PER_BLOCK (BLOCK_SIZE / sizeof(Article))
#define BLOCKS_PER_BUCKET 10
#include "HashUpload.h"



// Função de hash simples
int hashFunction(int id) {
    return id % NUM_BUCKETS;
}

// Função para remover aspas de uma string
std::string removeQuotes(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

// Função para verificar se uma string é numérica
bool is_number(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), [](char c) {
        return std::isdigit(c);
    });
}

// Inicializar o arquivo binário com M buckets, cada um com B blocos vazios
void initializeBuckets(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Erro ao criar o arquivo de buckets!" << std::endl;
        return;
    }

    // Preenche o arquivo com blocos vazios
    BlockHeader emptyHeader = {0};
    char buffer[BLOCK_SIZE];
    std::memset(buffer, 0, BLOCK_SIZE);
    std::memcpy(buffer, &emptyHeader, sizeof(BlockHeader));

    for (int bucket = 0; bucket < NUM_BUCKETS; ++bucket) {
        for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
            file.write(buffer, BLOCK_SIZE);
        }
    }

    file.close();
}

// Função para processar o arquivo CSV e retornar um vetor de artigos
std::vector<Article> processarCSV(const std::string& file_path) {
    std::ifstream input_file(file_path);
    if (!input_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo CSV!" << std::endl;
        return {};
    }

    std::string line;
    std::vector<Article> articles;

    while (std::getline(input_file, line)) {
        std::stringstream ss(line);
        Article article;

        // Lê o ID
        std::string id_str;
        while(std::getline(ss, id_str, ';')) {
            id_str.erase(0, id_str.find_first_not_of(" \t"));
            id_str.erase(id_str.find_last_not_of(" \t") + 1);
            id_str = removeQuotes(id_str);
            
            if (is_number(id_str)) {
                article.id = std::stoi(id_str);
                break;  // Para assim que encontrar um número válido
            } else {
                continue;
            }
        }

        // Lê o título
        std::string title;
        std::getline(ss, title, ';');
        strncpy(article.title, title.c_str(), sizeof(article.title) - 1);
        article.title[sizeof(article.title) - 1] = '\0';

        // Lê o ano
        std::string year_str;
        while(std::getline(ss, year_str, ';')) {
            year_str.erase(0, year_str.find_first_not_of(" \t"));
            year_str.erase(year_str.find_last_not_of(" \t") + 1);
            year_str = removeQuotes(year_str);
            
            if (is_number(year_str)) {
                article.year = std::stoi(year_str);
                break;  // Para assim que encontrar um número válido
            } else {
                continue;
            }
        }

        // Lê os autores
        std::string authors;
        std::getline(ss, authors, ';');
        strncpy(article.authors, authors.c_str(), sizeof(article.authors) - 1);
        article.authors[sizeof(article.authors) - 1] = '\0';

        // Lê as citações
        std::string citations_str;
        while (std::getline(ss, citations_str, ';')) {
            citations_str.erase(0, citations_str.find_first_not_of(" \t"));
            citations_str.erase(citations_str.find_last_not_of(" \t") + 1);
            citations_str = removeQuotes(citations_str);
            
            if (is_number(citations_str)) {
                article.citations = std::stoi(citations_str);
                break;  // Para assim que encontrar um número válido
            } else {
                continue;
            }
        }

        // Lê a data de atualização
        std::string updated_at_str;
        std::getline(ss, updated_at_str, ';');
        updated_at_str = removeQuotes(updated_at_str);
        strncpy(article.updated_at, updated_at_str.c_str(), sizeof(article.updated_at) - 1);
        article.updated_at[sizeof(article.updated_at) - 1] = '\0';

        // Lê o snippet
        std::string snippet;
        std::getline(ss, snippet, ';');
        strncpy(article.snippet, snippet.c_str(), sizeof(article.snippet) - 1);
        article.snippet[sizeof(article.snippet) - 1] = '\0';

        articles.push_back(article);
    }

    input_file.close();
    return articles;
}

// Função para inserir um registro em um bucket
void insertRecord(const Article& article, const std::string& bucket_filename, const std::string& overflow_filename) {
    std::fstream file(bucket_filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para inserção!" << std::endl;
        return;
    }

    int bucket = hashFunction(article.id);
    //std::cout << "Inserindo registro no bucket " << bucket << std::endl;
    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

    bool inserted = false;

    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + block * BLOCK_SIZE;
        file.seekg(block_pos);
        
        // Ler o cabeçalho do bloco
        BlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        if (header.recordCount < RECORDS_PER_BLOCK) {
            // Inserir o registro neste bloco
            std::streampos write_pos = block_pos + sizeof(BlockHeader) + header.recordCount * sizeof(Article);
            file.seekp(write_pos);
            file.write(reinterpret_cast<const char*>(&article), sizeof(Article));

            // Atualizar o cabeçalho
            header.recordCount += 1;
            file.seekp(block_pos);
            file.write(reinterpret_cast<const char*>(&header), sizeof(BlockHeader));

            inserted = true;
            break;
        }
    }

    file.close();

    if (!inserted) {
        // Inserção no arquivo de overflow
        std::ofstream overflow_file(overflow_filename, std::ios::binary | std::ios::app);
        if (!overflow_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo de overflow!" << std::endl;
            return;
        }

        overflow_file.write(reinterpret_cast<const char*>(&article), sizeof(Article));
        overflow_file.close();
        //std::cerr << "Bucket cheio. Registro inserido no overflow." << std::endl;
    }
}

// Função para gravar todos os artigos utilizando hash
void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename) {
    for (const auto& article : articles) {
        insertRecord(article, bucket_filename, overflow_filename);
    }
}

// Função para ler registros de um bucket específico
std::vector<Article> readBucket(int bucket, const std::string& bucket_filename, const std::string& overflow_filename) {
    std::vector<Article> articles;
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para leitura!" << std::endl;
        return articles;
    }

    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + block * BLOCK_SIZE;
        file.seekg(block_pos);

        // Ler o cabeçalho do bloco
        BlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        for (int i = 0; i < header.recordCount; ++i) {
            Article article;
            file.read(reinterpret_cast<char*>(&article), sizeof(Article));
            articles.push_back(article);
        }
    }

    file.close();

    // Ler registros do arquivo de overflow
    std::ifstream overflow_file(overflow_filename, std::ios::binary);
    if (overflow_file.is_open()) {
        Article article;
        while (overflow_file.read(reinterpret_cast<char*>(&article), sizeof(Article))) {
            if (hashFunction(article.id) == bucket) {
                articles.push_back(article);
            }
        }
        overflow_file.close();
    }

    return articles;
}

// Função para encontrar um registro pelo ID



int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <csv_file>" << std::endl;
        return 1;
    }
    std::string file_path = argv[1];

    // Inicializa o arquivo de buckets (executar apenas uma vez)
    initializeBuckets("articles.bin");
    std::cout << "Buckets inicializados." << std::endl;

    // Processa o CSV e retorna os artigos
    std::vector<Article> articles = processarCSV(file_path);
    std::cout << "CSV processado. Total de artigos: " << articles.size() << std::endl;

    // Grava os artigos no arquivo binário utilizando hash
    gravarArtigosComHash(articles, "articles.bin", "overflow.bin");
    std::cout << "Artigos gravados nos buckets binários." << std::endl;

    // Exemplo: Ler e imprimir os artigos de um bucket específico
    int bucket_to_read;
    while(bucket_to_read != -1) {
        std::cout << "Digite o número do bucket para ler (0 a " << NUM_BUCKETS - 1 << "): ";
        std::cin >> bucket_to_read;

        if (bucket_to_read < 0 || bucket_to_read >= NUM_BUCKETS) {
            std::cerr << "Número de bucket inválido!" << std::endl;
            return 1;
        }

        std::vector<Article> bucket_articles = readBucket(bucket_to_read, "articles.bin", "overflow.bin");
        std::cout << "Artigos no bucket " << bucket_to_read << ":" << std::endl;
        for (const auto& article : bucket_articles) {
            article.print();
        }
    }

    std::cout << "Processamento completo!" << std::endl;
    return 0;
}
