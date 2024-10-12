#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <string.h>
#include <algorithm>

#define MAX_AUTHORS 150

std::string removeQuotes(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

bool is_number(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), [](char c) {
        return std::isdigit(c);
    });
}

struct Article {
    int id;
    char title[300];
    int year;
    char authors[MAX_AUTHORS];
    int citations;
    std::string updated_at; // Pode ser usado para armazenar a data como string
    char snippet[1024];

    // Função para imprimir os dados do artigo
    void print() const {
        std::cout << "ID: " << id << ", Title: " << title 
                  << ", Year: " << year << ", Authors: " << authors 
                  << ", Citations: " << citations 
                  << ", Updated At: " << updated_at 
                  << ", Snippet: " << snippet << std::endl;
    }

    // Função para formatar o artigo como string
    std::string toString() const {
        std::stringstream ss;
        ss << "Id: " << id << " | " 
           << "Titulo: " <<title << " | " 
           << "Ano: " << year << " | " 
           << "Autor: " << authors << " | "  
           << "Citacao: " <<citations << " | " 
           << "Atualizacao: "  << updated_at << " | " 
           << "Snippet: " << snippet << " | ";
        return ss.str();
    }
};

void upload(const std::string& file_path, const std::string& output_path) {
    std::ifstream input_file(file_path);
    std::ofstream output_file(output_path);
    std::cout << "Entrou no upload " << std::endl;
    if (!input_file.is_open() || !output_file.is_open()) {
        std::cerr << "Error opening files!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        std::cout << "Entrou no while " << std::endl;
        std::stringstream ss(line);
        Article article;

        // Lê o ID
        std::string id_str;
        std::getline(ss, id_str, ';');
        id_str = removeQuotes(id_str);
        article.id = std::stoi(id_str);
        std::cout << "ID: " << article.id << std::endl;

        // Lê o título
        std::string title;
        std::getline(ss, title, ';');
        strncpy(article.title, title.c_str(), sizeof(article.title) - 1);
        article.title[sizeof(article.title) - 1] = '\0'; // Garantir que a string seja terminada

        // Lê o ano
        std::string year_str;
        std::getline(ss, year_str, ';');
        year_str = removeQuotes(year_str);
        article.year = std::stoi(year_str);
        std::cout << "Year: " << article.year << std::endl;

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
                std::cout << "Citations: " << article.citations << std::endl;
                break;  // Para assim que encontrar um número válido
            } else {
                std::cerr << "Citações inválidas, ignorando: '" << citations_str << "'" << std::endl;
                // Lê novamente até encontrar o próximo delimitador
                continue;
            }
        }

        // Lê a data de atualização
        std::getline(ss, article.updated_at, ';');

        // Lê o snippet
        std::string snippet;
        std::getline(ss, snippet, ';');
        strncpy(article.snippet, snippet.c_str(), sizeof(article.snippet) - 1);
        article.snippet[sizeof(article.snippet) - 1] = '\0';

        // Escreve o artigo no arquivo de texto
        output_file << article.toString() << std::endl;
    }

    input_file.close();
    output_file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }
    std::string file_path = argv[1];

    upload(file_path, "articles.txt"); // Saída em um arquivo de texto
    std::cout << "Upload completed successfully!" << std::endl;

}
