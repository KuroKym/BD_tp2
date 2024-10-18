#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <algorithm>
#include "HashUpload.h"
#include "btree.cpp"

/* #defines foram definidos no .hb
#define NUM_BUCKETS 100
#define BLOCK_SIZE 4096 // 4KB por bloco
#define RECORDS_PER_BLOCK (BLOCK_SIZE / sizeof(Article))
#define BLOCKS_PER_BUCKET 10*/

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
    // Abre o arquivo em modo de truncamento, o que apaga seu conteúdo, se existir
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
    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

    bool inserted = false;

    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + static_cast<std::streamoff>(block * BLOCK_SIZE);
        file.seekg(block_pos);

        // Ler o bloco completo
        Block current_block;
        file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));

        if (!current_block.isFull()) {
            // Inserir o artigo neste bloco
            current_block.addArticle(article);

            // Voltar e gravar o bloco atualizado
            file.seekp(block_pos);
            file.write(reinterpret_cast<const char*>(&current_block), sizeof(Block));

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
        std::cerr << "Bucket cheio. Registro inserido no overflow." << std::endl;
    }
}


// Função para gravar todos os artigos utilizando hash
void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename, const std::string& index_filename) {
    // Apaga o arquivo de overflow, se já existir, para recriá-lo
    std::ofstream overflow_file(overflow_filename, std::ios::binary | std::ios::trunc);
    if (!overflow_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de overflow para sobrescrever!" << std::endl;
        return;
    }
    overflow_file.close();  // Fechar, pois apenas criamos o arquivo vazio

    for (const auto& article : articles) {
        insertRecord(article, bucket_filename, overflow_filename);
    }

    // Gerar os índices primários após inserir os artigos
    gerarIndicesPrimarios(articles, index_filename);
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
        std::streampos block_pos = bucket_start + static_cast<std::streamoff>(block * BLOCK_SIZE);
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


// Função para gerar o arquivo de índices primários usando a B+ Tree
// Função para gerar o arquivo de índices primários usando a B+ Tree
void gerarIndicesPrimarios(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& index_filename) {
    BPlusTree btree;

    // Inserir os IDs dos artigos na B+ Tree com o endereço do bloco
    for (const auto& article : articles) {
        int bucket = hashFunction(article.id);
        std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

        // Calcula o endereço do bloco onde o artigo está armazenado
        for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
            std::streampos block_pos = bucket_start + static_cast<std::streamoff>(block * BLOCK_SIZE);
            Block current_block;

            // Ler o bloco para verificar se o artigo está presente
            std::ifstream file(bucket_filename, std::ios::binary);
            file.seekg(block_pos);
            file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));

            // Verifica se o artigo está no bloco
            for (int i = 0; i < current_block.header.recordCount; ++i) {
                if (current_block.records[i].id == article.id) {
                    // Inserir na B+ Tree: chave (ID) e endereço (posição do bloco)
                    btree.insert(article.id, block_pos);
                    break;
                }
            }
            file.close();
        }
    }

    // Salvar a árvore B+ em um arquivo
    std::ofstream index_file(index_filename, std::ios::binary);
    if (!index_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de índices primários para gravação!" << std::endl;
        return;
    }

    // Aqui você pode implementar a lógica para gravar a B+ Tree no arquivo
    btree.save(index_file); // Supondo que você tenha uma função para salvar a B+ Tree

    index_file.close();
}
