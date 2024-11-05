#include "secundaria.hpp"
#include "HashUpload.hpp"
#include <iostream>
#include <fstream>
using namespace std;

Article findRecordByPosition(BplusTreeSec bptree, const std::string titulo, const std::string& bucket_filename) {
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para leitura!" << std::endl;
        throw std::runtime_error("Erro ao abrir o arquivo.");
    }
    std::streampos pos = bptree.searchKey(titulo);  // Posição do registro a ser encontrado
    // Mover o ponteiro do arquivo para a posição especificada
    file.seekg(pos);
    
    // Carregar o bloco de dados a partir da posição
    Block current_block;
    file.read(reinterpret_cast<char*>(&current_block), sizeof(Block));

    // Realizar uma busca binária dentro do bloco para encontrar o registro com o `titulo` correto
    int recordIndex = buscarPorTitulo(current_block, titulo);
    if (recordIndex != -1) {
        // Retorna o artigo encontrado
        return current_block.records[recordIndex];
    } else {
        std::cerr << "Registro com Titulo " << titulo << " não encontrado no bloco." << std::endl;
        throw std::runtime_error("Registro não encontrado.");
    }
}

int main(){
    std::string bucket_filename = "articles.bin";
    BplusTreeSec bptree(3); // Posição do registro a ser encontrado
    string titulo;  // ID do registro a ser encontrado
    cout << "Digite o ID do registro a ser encontrado: ";
    cin >> titulo;
    bptree.loadFromFile("index.bin");

    try {
        // Encontrar o registro na posição especificada
        Article found_article = findRecordByPosition(bptree, titulo, bucket_filename);

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
