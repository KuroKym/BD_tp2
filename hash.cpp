#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <algorithm>
#include "hash.hpp"
#include "primaria.hpp"
#include "secundaria.hpp"

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
        //std::cerr << "Bucket cheio. Registro inserido no overflow." << std::endl;
    }
}


// Função para inserir um registro em um bucket e retornar a posição onde foi inserido
std::streampos insertRecordAndGetPosition(const Article& article, const std::string& bucket_filename, const std::string& overflow_filename) {
    std::fstream file(bucket_filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para inserção!" << std::endl;
        return -1;  // Indica erro ao abrir o arquivo
    }

    int bucket = hashFunction(article.id);
    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

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

            file.close();

            return block_pos;  // Retorna a posição onde o artigo foi inserido
        }
    }

    file.close();

    // Se o bucket estiver cheio, insere no arquivo de overflow
    std::ofstream overflow_file(overflow_filename, std::ios::binary | std::ios::app);
    if (!overflow_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de overflow!" << std::endl;
        return -1;  // Indica erro ao abrir o arquivo de overflow
    }

    std::streampos overflow_pos = overflow_file.tellp();  // Posição antes de escrever o artigo
    overflow_file.write(reinterpret_cast<const char*>(&article), sizeof(Article));
    overflow_file.close();

    return overflow_pos;  // Retorna a posição do artigo no overflow
}


void gravarArtigosComHash(const std::vector<Article>& articles, const std::string& bucket_filename, const std::string& overflow_filename) {
    // Inicializa a B+ Tree
    BplusTree bptree(3);  // O grau é (3)
    BplusTreeSec bptreeSec(3);

    // Itera sobre todos os artigos e insere cada um no bucket correspondente
    int count = 0;
    for (const auto& article : articles) {
        // Inserir o artigo no bucket correto e obter a posição onde ele foi inserido
        std::streampos pos = insertRecordAndGetPosition(article, bucket_filename, overflow_filename);

        // Inserir o ID e a posição na B+ Tree
        if (pos != -1) {  // Verifique se a posição não é -1
            bptree.insert(article.id, pos);  // Insere o ID e a posição na B+ Tree
            bptreeSec.insert(article.title, pos);
            std::cout << "Artigo " << article.id << " inserido na B+ Tree primaria na posição " << pos << std::endl;
            std::cout << "Artigo "  << article.title<< " inserido na B+ Tree secundaria na posição " << pos << std::endl;
        }

        count++;
        if (count % 10000 == 0) {
            std::cout << "Artigos processados: " << count << std::endl;
        }
    }

    // Salva a B+ Tree no arquivo de índice (reescreve a árvore no arquivo)
    bptree.saveToFile("index.bin");
    bptreeSec.saveToFile("indexSec.bin");
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


// Função auxiliar para realizar busca binária dentro de um bloco de registros
int binarySearchInBlock(const Block& block, int id) {
    int low = 0;
    int high = block.header.recordCount - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        if (block.records[mid].id == id) {
            return mid;  // Registro encontrado
        }
        if (block.records[mid].id < id) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;  // Registro não encontrado
}

int buscarPorTitulo(const Block& block, const std::string& title) {
    int low = 0;
    int high = block.header.recordCount - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        
        // Remove as aspas do título no registro atual
        std::string currentTitle = removeQuotes(block.records[mid].title);
        
        // Comparação
        if (currentTitle == title) {
            return mid;  // Retorna o índice do registro encontrado
        }
        if (currentTitle < title) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;  // Retorna -1 para indicar que o registro não foi encontrado
}

