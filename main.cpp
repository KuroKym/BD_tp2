#include "HashUpload.h"
#include <locale>

int main(int argc, char* argv[]) {

    std::locale::global(std::locale(""));

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
    do {
        std::cout << "Digite o número do bucket para ler (0 a " << NUM_BUCKETS - 1 << "): ";
        std::cin >> bucket_to_read;

        if(bucket_to_read > NUM_BUCKETS-1){
            std:: cout << "Numero invalido" << std::endl;
        }else{
            std::vector<Article> bucket_articles = readBucket(bucket_to_read, "articles.bin", "overflow.bin");
            std::cout << "Artigos no bucket " << bucket_to_read << ":" << std::endl;
            for (const auto& article : bucket_articles) {
                article.print();
            }
        }
    }while(bucket_to_read != -1);

    std::cout << "Processamento completo!" << std::endl;
    return 0;
}