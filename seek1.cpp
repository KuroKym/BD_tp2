#include "primaria.hpp"
#include "hash.hpp"
#include <iostream>
#include <fstream>
 
Article findRecordByPosition(BplusTree bptree, int id, const std::string& bucket_filename) {
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para leitura!" << std::endl;
        throw std::runtime_error("Erro ao abrir o arquivo.");
    }
    std::streampos pos = bptree.searchKey(id);  // Posição do registro a ser encontrado
    // Mover o ponteiro do arquivo para a posição especificada
    file.seekg(pos);
    
    // Carregar o bloco de dados a partir da posição
    Block current_block;
    file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));

    // Realizar uma busca binária dentro do bloco para encontrar o registro com o `id` correto
    int recordIndex = binarySearchInBlock(current_block, id);
    if (recordIndex != -1) {
        // Retorna o artigo encontrado
        return current_block.records[recordIndex];
    } else {
        std::cerr << "Registro com ID " << id << " não encontrado no bloco." << std::endl;
        throw std::runtime_error("Registro não encontrado.");
    }
}

int main(){
    std::string bucket_filename = "articles.bin";
    BplusTree bptree(3); // Posição do registro a ser encontrado
    int id;  // ID do registro a ser encontrado
    cout << "Digite o ID do registro a ser encontrado: ";
    cin >> id;
    bptree.loadFromFile("index.bin");

    try {
        // Encontrar o registro na posição especificada
        Article found_article = findRecordByPosition(bptree, id, bucket_filename);

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
