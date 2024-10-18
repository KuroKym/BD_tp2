#include "HashUpload.h" // Include the header file where hashFunction is defined


// Função para calcular o número total de blocos no arquivo
int calculateTotalBlocks(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para cálculo de blocos!" << std::endl;
        return -1;
    }

    // Calcula o tamanho total do arquivo
    std::streampos file_size = file.tellg();
    file.close();

    // Retorna o número total de blocos no arquivo
    return static_cast<int>(file_size / BLOCK_SIZE);
}

// Função para encontrar um registro pelo ID e retornar as informações solicitadas
void findRecordById(int id, const std::string& bucket_filename, const std::string& overflow_filename) {
    int bucket = hashFunction(id);
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para leitura!" << std::endl;
        return;
    }

    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;
    int blocks_read = 0;
    bool found = false;
    Article found_article;

    // Procura nos blocos do bucket
    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + static_cast<std::streamoff>(block * BLOCK_SIZE);
        file.seekg(block_pos);

        // Ler o cabeçalho do bloco
        BlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        blocks_read++;  // Incrementa o contador de blocos lidos

        // Procura pelo registro no bloco
        for (int i = 0; i < header.recordCount; ++i) {
            Article article;
            file.read(reinterpret_cast<char*>(&article), sizeof(Article));
            if (article.id == id) {
                found = true;
                found_article = article;
                break;
            }
        }

        if (found) break; // Sai do laço se o registro foi encontrado
    }

    file.close();

    // Se o registro não foi encontrado no arquivo principal, procura no overflow
    if (!found) {
        std::ifstream overflow_file(overflow_filename, std::ios::binary);
        if (!overflow_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo de overflow para leitura!" << std::endl;
            return;
        }

        Article article;
        while (overflow_file.read(reinterpret_cast<char*>(&article), sizeof(Article))) {
            blocks_read++;  // Incrementa o contador de blocos lidos
            if (article.id == id) {
                found = true;
                found_article = article;
                break;
            }
        }
        overflow_file.close();
    }

    // Calcula o número total de blocos no arquivo de dados principal
    int total_blocks = calculateTotalBlocks(bucket_filename);

    // Exibe os resultados
    if (found) {
        
        std::cout << "Registro encontrado:" << std::endl;
        std::cout << "ID: " << found_article.id << std::endl;
        std::cout << "Título: " << found_article.title << std::endl;
        std::cout << "Ano: " << found_article.year << std::endl;
        std::cout << "Autores: " << found_article.authors << std::endl;
        std::cout << "Citações: " << found_article.citations << std::endl;
        std::cout << "Data de Atualização: " << found_article.updated_at << std::endl;
        std::cout << "Snippet: " << found_article.snippet << std::endl;

        std::cout << "Blocos lidos: " << blocks_read << std::endl;
        std::cout << "Total de blocos no arquivo: " << total_blocks << std::endl;
    } else if(id > 0){
        std::cout << "Registro com ID " << id << " não encontrado." << std::endl;
    }else{return;}
}



/*void printRecordsInBlock(int block_number, const std::string& bucket_filename) {
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para leitura!" << std::endl;
        return;
    }

    std::streampos block_pos = static_cast<std::streamoff>(block_number * BLOCK_SIZE);
    file.seekg(block_pos);

    // Ler o cabeçalho do bloco
    BlockHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

    std::cout << "Bloco " << block_number << ":" << std::endl;
    std::cout << "Quantidade de registros no bloco: " << header.recordCount << std::endl;

    // Ler e imprimir todos os registros contidos no bloco
    for (int i = 0; i < header.recordCount; ++i) {
        Article article;
        file.read(reinterpret_cast<char*>(&article), sizeof(Article));

        std::cout << "Registro " << i + 1 << ":" << std::endl;
        std::cout << "ID: " << article.id << std::endl;
        std::cout << "Título: " << article.title << std::endl;
        std::cout << "Ano: " << article.year << std::endl;
        std::cout << "Autores: " << article.authors << std::endl;
        std::cout << "Citações: " << article.citations << std::endl;
        std::cout << "Data de Atualização: " << article.updated_at << std::endl;
        std::cout << "Snippet: " << article.snippet << std::endl;
        std::cout << "------------------------------" << std::endl;

    file.close();
} */

// Função para imprimir informações sobre todos os blocos em um bucket
/*void printBlocksInBucket(int bucket, const std::string& bucket_filename) {
    std::ifstream file(bucket_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de buckets para leitura!" << std::endl;
        return;
    }

    std::streampos bucket_start = bucket * BLOCKS_PER_BUCKET * BLOCK_SIZE;

    std::cout << "Bucket " << bucket << ":" << std::endl;

    for (int block = 0; block < BLOCKS_PER_BUCKET; ++block) {
        std::streampos block_pos = bucket_start + static_cast<std::streamoff>(block * BLOCK_SIZE);
        file.seekg(block_pos);

        // Ler o cabeçalho do bloco
        BlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        std::cout << "Bloco " << block << ": " << std::endl;
        std::cout << "Quantidade de registros no bloco: " << header.recordCount << std::endl;

        if (header.recordCount > 0) {
            std::cout << "Registros presentes neste bloco:" << std::endl;
            for (int i = 0; i < header.recordCount; ++i) {
                Article article;
                file.read(reinterpret_cast<char*>(&article), sizeof(Article));

                std::cout << "  Registro " << i + 1 << ": ID " << article.id << ", Título: " << article.title << std::endl;
            }
        } else {
            std::cout << "  Bloco vazio." << std::endl;
        }

        std::cout << "------------------------------" << std::endl;
    }

    file.close();
}*/

int main(int argc, char* argv[]) {
    if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return 1;
    }
    int id_to_find;
    do {
        std::cout << "Digite o ID do artigo para buscar: ";
        std::cin >> id_to_find;
        findRecordById(id_to_find, "articles.bin", "overflow.bin");
        
    }while(id_to_find != -1);
    std:: cout << "Tamanho do registro por bloco: " << sizeof(RECORDS_PER_BLOCK) << std:: endl;
    //printBlocksInBucket(22, "articles.bin");
    //printRecordsInBlock(22, "articles.bin");
    
    
    return 0;   
}