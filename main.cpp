#include <iostream>
#include <map>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "vector"

#define MIN_LINKS_TO_FILE 2//мінімальне число жорстких посилань

using namespace std;

struct Node {//структура, що зв'язує серійний номер файлу (st_ino) та шляхи до файлів
    int linkID;//серійний номер файлу
    vector<string> *paths;//шляхи до файлів
    int numberOfLinks;// кількість посилань

    Node(const int &linkID, const int &numberOfLinks, const string &path) {
        this->linkID = linkID;
        paths = new vector<string>();
        this->numberOfLinks = numberOfLinks;
        push(path);
    }

    void push(const string &path) {
        paths->push_back(path);
    }

    void print() {
        for (string &path: *paths) {
            cout << "Link:\t| " << linkID << " |\tNumber of links:\t| " << numberOfLinks
                 << " |\tpath:\t\" " << path << "\"" << endl;
        }
    }

    ~Node() {
        delete paths;
    }
};

map<int, Node *> nodes;//int - link

void scanFile(const struct stat &buf, string &path) {
    int key = buf.st_ino;//дістаємо серійний номер, що і буде ключем
    if (nodes.contains(key)) {
        auto node = nodes.find(key);
        node->second->push(path);//якщо даний серійний номер вже зустрічався, то просто додаємо шлях у наявну Node
        return;
    }
    //інакше створюємо нову Node
    nodes.insert(pair(key, new Node(key, buf.st_nlink, path)));
}

void scanAllFilesInDirectory(const string &path) {
    //функція сканує всі файли за даним шляхом
    DIR *dirp = NULL;//DIR - Тип, що представляє потік каталогу
    dirp = opendir(path.c_str());/*Функція opendir() відкриває потік каталогу,
 * що відповідає каталогу, названому аргументом path.
 *
 * Після успішного завершення opendir() повертає вказівник на об’єкт типу DIR.
 * В іншому випадку повертається NULL, а errno встановлюється для вказівки на помилку.*/
    if (dirp == NULL) {
        cout << "Error in " << path << endl;
        return;
    }
    struct dirent entry;
    struct dirent *result = NULL;
    readdir_r(dirp, &entry, &result);
    /*Функція readdir_r() ініціалізує структуру dirent, на яку посилається entry, для представлення
     * запису каталогу в поточній позиції в потоці каталогу, на який посилається dirp, зберігає вказівник
     * на цю структуру в місці, на яке посилається result, і розміщує потік каталогу в наступний запис.*/
    char filePath[PATH_MAX];// шлях до файлу
    while (result != NULL) {
        struct stat buf;//інформація про файл
        strncpy(filePath, (path + "/" + entry.d_name).c_str(), PATH_MAX);
        if (lstat(filePath, &buf) == 0) {/*Функція lstat() отримує інформацію про названий файл
 * і записує її в область, на яку вказує аргумент buf
 * Після успішного завершення lstat() повертає 0.
 * В іншому випадку він повертає -1 і встановлює errno, щоб вказати помилку.*/
            string filePathString(filePath);
            if (S_ISDIR(buf.st_mode)) { //S_ISDIR - Цей макрос повертає int відмінний від нуля, якщо файл є каталогом.
                if (strcmp(entry.d_name, ".") &&
                    strcmp(entry.d_name, "..")) { // Пропускаємо папки "." та ".."
                    scanAllFilesInDirectory(filePathString); //рекурсивно заходимо в папку
                }
            } else if (S_ISREG(buf.st_mode) != 0 && buf.st_nlink >= MIN_LINKS_TO_FILE) {
                //S_ISREG - Цей макрос повертає відмінний від нуля, якщо файл є звичайним.
                // buf.st_nlink - кількість жорстких посилань на файл
                scanFile(buf, filePathString);//опрацьовуємо файл
            }
        } else {//lstat не зміг отримати інформацію про файл і повернув не 0
            cout << "Error in lstat() through" << filePath << endl;
        }
        readdir_r(dirp, &entry, &result);//ітеруємося на наступний файл
    }
    closedir(dirp);//закриваємо потік
}

void printAll() {
    for (auto &it: nodes) {
        it.second->print();
        delete it.second;
        cout << endl;
    }
}

int main() {
    string path = "/home/serhii";
    scanAllFilesInDirectory(path);
    printAll();
    return 0;
}