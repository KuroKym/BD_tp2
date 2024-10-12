#include "HashUpload.h" // Include the header file where hashFunction is defined

int hashFunction(int id) {
    return id % NUM_BUCKETS;
}

Article* findRecordById(int id, const std::string& bucket_filename, const std::string& overflow_filename) {
    int bucket = hashFunction(id);
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para leitura!" << std::endl;
        return nullptr;
    }

    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

    // Procura nos blocos do bucket
    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + block * BLOCK_SIZE;
        file.seekg(block_pos);

        // Ler o cabeçalho do bloco
        BlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        for (int i = 0; i < header.recordCount; ++i) {
            Article article;
            file.read(reinterpret_cast<char*>(&article), sizeof(Article));
            if (article.id == id) {
                file.close();
                return new Article(article); // Retorna uma cópia do artigo encontrado
            }
        }
    }

    file.close();

    // Busca no arquivo de overflow
    std::ifstream overflow_file(overflow_filename, std::ios::binary);
    if (overflow_file.is_open()) {
        Article article;
        while (overflow_file.read(reinterpret_cast<char*>(&article), sizeof(Article))) {
            if (article.id == id) {
                overflow_file.close();
                return new Article(article); // Retorna uma cópia do artigo encontrado
            }
        }
        overflow_file.close();
    }

    return nullptr; // Registro não encontrado
}

int main(int argc, char* argv[]) {
    if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return 1;
    }
    int id_to_find;
    do {
        std::cout << "Digite o ID do artigo para buscar: ";
        std::cin >> id_to_find;
        Article* found_article = findRecordById(id_to_find, "articles.bin", "overflow.bin");
        if(id_to_find != -1){
            if (found_article) {
                std::cout << "Registro encontrado:" << std::endl;
                found_article->print();
                delete found_article; // Lembre-se de liberar a memória alocada
            } else {
                std::cout << "Registro com ID " << id_to_find << " não encontrado." << std::endl;
            }
        }else{
            std::cout << "Saindo..." << std::endl;
            break;
        }
    }while(id_to_find != -1);
    
    
    return 0;   
}