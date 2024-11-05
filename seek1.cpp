#include "primaria.hpp"
#include "hash.hpp"
#include <iostream>
#include <fstream>

Article findRecordByPosition(BplusTree bptree, int id, const std::string& bucket_filename, const std::string& overflow_filename) {
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo principal para leitura!" << std::endl;
        throw std::runtime_error("Erro ao abrir o arquivo principal.");
    }

    // Tentar localizar o registro no arquivo principal
    std::streampos pos = bptree.searchKey(id);  // Posição do registro a ser encontrado
    file.seekg(pos);

    Block current_block;
    file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));

    // Realizar a busca no bloco do arquivo principal
    int recordIndex = binarySearchInBlock(current_block, id);
    if (recordIndex != -1) {
        // Registro encontrado no arquivo principal
        return current_block.records[recordIndex];
    }

    // Caso não encontrado, abrir o arquivo de overflow
    std::ifstream overflow_file(overflow_filename, std::ios::binary);
    if (!overflow_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de overflow para leitura!" << std::endl;
        throw std::runtime_error("Erro ao abrir o arquivo de overflow.");
    }

    // Procurar no arquivo de overflow
    while (true) {
        overflow_file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));
        if (overflow_file.eof()) break;

        // Buscar no bloco do arquivo de overflow
        recordIndex = binarySearchInBlock(current_block, id);
        if (recordIndex != -1) {
            // Registro encontrado no arquivo de overflow
            return current_block.records[recordIndex];
        }
    }

    // Se o laço termina, significa que não encontrou o registro
    std::cerr << "Registro com ID " << id << " não encontrado." << std::endl;
    throw std::runtime_error("Registro não encontrado.");
}

int main() {
    std::string bucket_filename = "articles.bin";
    std::string overflow_filename = "overflow.bin";
    BplusTree bptree(3); // Ordem da B+ Tree
    int id;  // ID do registro a ser encontrado
    std::cout << "Digite o ID do registro a ser encontrado: ";
    std::cin >> id;
    bptree.loadFromFile("index.bin");

    try {
        // Encontrar o registro na posição especificada
        Article found_article = findRecordByPosition(bptree, id, bucket_filename, overflow_filename);

        // Imprimir os dados do registro encontrado
        std::cout << "Registro encontrado:" << std::endl;
        std::cout << "ID: " << found_article.id << std::endl;
        std::cout << "Título: " << found_article.title << std::endl;
        std::cout << "Ano: " << found_article.year << std::endl;
        std::cout << "Autores: " << found_article.authors << std::endl;
        std::cout << "Citações: " << found_article.citations << std::endl;
        std::cout << "Data de Atualização: " << found_article.updated_at << std::endl;
        std::cout << "Snippet: " << found_article.snippet << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao encontrar o registro: " << e.what() << std::endl;
    }

    return 0;
}
