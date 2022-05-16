#include <iostream>
#include <map>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "math.h"

#define CONSOLE_CAPACITY 80//максимальне число символів для найбільшого результату гістограми
using namespace std;

const int interval = 1024 / 2;//інтервал
map<string, int> statistic;//string - інтервал, int - лічильник файлів, що входять у цей інтервал
long maxCounter = 0;//максимальна кількість файлів, що потрапляла у проміжок

void countSizeToRange(int size);/*знаходимо проміжок якому належить розмір файлу
 * та інкрементуємо лічильник проміжку*/

void countFile(struct stat &file) {//дізнаємося розмір файлу
    int size = file.st_size;
    countSizeToRange(size);
}

void scanAllFilesInDirectory(const string &path) {
    //функція сканує всі файли за даним шляхом
    DIR *dirp = NULL;//DIR - Тип, що представляє потік каталогу
    dirp = opendir(path.c_str());/*Функція opendir() відкриває потік каталогу,
 * що відповідає каталогу, названому аргументом path.
 *
 * Після успішного завершення opendir() повертає вказівник на об’єкт типу DIR.
 * В іншому випадку повертається NULL, а errno встановлюється для вказівки на помилку.*/
    if (dirp == NULL) { cout << "Error in " << path << endl; }
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
            if (S_ISDIR(buf.st_mode)) { //S_ISDIR - Цей макрос повертає int відмінний від нуля, якщо файл є каталогом.
                if (strcmp(entry.d_name, ".") &&
                    strcmp(entry.d_name, "..")) { // Пропускаємо папки "." та ".."
                    string nextFolder(filePath);
                    scanAllFilesInDirectory(nextFolder); //рекурсивно заходимо в неї
                }
            } else if (S_ISREG(buf.st_mode)) {
                //S_ISREG - Цей макрос повертає відмінний від нуля, якщо файл є звичайним.
                countFile(buf);
            }
        } else {//lstat не зміг отримати інформацію про файл і повернув не 0
            cout << "Error in lstat() through" << filePath << endl;
        }
        readdir_r(dirp, &entry, &result);//ітеруємося на наступний файл
    }
    closedir(dirp);//закриваємо потік
}

void printResult();

int main() {
    string path = "/home/serhii/Documents";
    scanAllFilesInDirectory(path);
    printResult();
    return 0;
}

string rangeToString(int start, int finish) {
    return to_string(start) + "-" + to_string(finish);
}

string getRange(int value) {//знаходимо проміжок якому належить даний розмір
    if (value <= interval) {
        return rangeToString(1, interval);
    }
    int start, finish;
    if (value % interval == 0) {
        start = value - interval + 1;
        return rangeToString(start, value);
    }
    int diff = value - value % interval;
    start = diff + 1;
    finish = diff + interval;
    return rangeToString(start, finish);
}

void countSizeToRange(int size) {//функція для підрахунку кількості файлів, що належать кожному проміжку
    string key = getRange(size); //обраховуємо проміжок якому він належить - це і є ключ.
    // А далі додаємо у мапу або інкрементуємо значення лічильника, якщо у цьому проміжку вже хтось був
    int counter;
    if (!statistic.contains(key)) {
        counter = 1;
        statistic.insert(pair(key, counter));
    } else {
        auto node = statistic.find(key);
        counter = node->second + 1;
        node->second = counter;
    }
    if (counter > maxCounter) { maxCounter = counter; }
}

void printStarLine(int stars) {
    for (int i = 0; i < stars; ++i) { cout << "*"; }
    cout << endl;
}

void printResult() {
    if (maxCounter == 0) { return; }
    double coeff = (double) CONSOLE_CAPACITY / ((double) maxCounter);
    for (auto &node: statistic) {
        int size = node.second;
        if (size != 0) {
            cout << node.first << endl;
            printStarLine(max((size * coeff), 1.0));
        }
    }
}