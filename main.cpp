#include <Windows.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <regex>
#include <omp.h>

/*21. Zrównoleglone poszukiwanie plików o zadanym zakresie rozmiarów i zadanym zakresie dat utworzenia w strukturze katalogów - realizacja OpenMP.*/

using namespace std;

extern "C" char* strptime(const char* s,
                          const char* f,
                          struct tm* tm) {
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail()) {
        return nullptr;
    }
    return (char*)(s + input.tellg());
}

unsigned long int getMinSize() {
    unsigned long int min_size;
    cout << "Podaj dolna granice rozmiaru pliku w bajtach:" << endl;
    cin >> min_size;
    return min_size;
}

unsigned long int getMaxSize() {
    unsigned long int max_size;
    cout << "Podaj gorna granice rozmiaru pliku w bajtach:" << endl;
    cin >> max_size;
    return max_size;
}

string isValidateDate() {
    regex pattern("\\d{4}[-]\\d{2}[-]\\d{2}");
    string str;
    fflush(stdin);
    while (getline(cin, str)) {
        if (regex_match(str, pattern)) {
            return str;
        } else {
            cout << "Podana data nie jest poprawna, sprobuj jeszcze raz!" << endl;
        }
    }
}

string getMinDate() {
    string min_date;
    cout << "Podaj dolną granice utworzenia pliku w formacie YYYY-MM-DD:" << endl;
    min_date = isValidateDate();
    return min_date;
}
string getMaxDate() {
    string max_date;
    cout << "Podaj gorna granice utworzenia pliku w formacie YYYY-MM-DD:" << endl;
    max_date = isValidateDate();
    return max_date;
}

string addCharacter(WORD str) {
    string tmp = to_string(str);
    if(tmp.length() == 1) {
        tmp = "0" + tmp;
    }
    return tmp;
}

long long convertToMilisecond(string date) {

    tm tm = {};
    char *date_char = strdup(date.c_str());
    const char* snext = ::strptime(date_char, "%Y-%m-%d %H:%M:%S", &tm);
//    printf("%d/%d/%d %d:%d:%d", tm.tm_mday, tm.tm_mon, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
    auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    long long  duration_ms = time_point.time_since_epoch() / std::chrono::milliseconds(1);

    return duration_ms;
}

void saveDirectories(const wstring &directory, vector<string> &dirs)
{
    wstring tmp = directory + L"\\*";
    WIN32_FIND_DATAW file;
    HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);
    if (search_handle != INVALID_HANDLE_VALUE)
    {
        vector<wstring> directories;
        do
        {
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                tmp = directory + L"\\" + wstring(file.cFileName);
                string str(tmp.begin(), tmp.end());
                directories.push_back(tmp);
//                cout << str << endl;
                dirs.push_back(str);
            }
        }
        while (FindNextFileW(search_handle, &file));
        FindClose(search_handle);
//        cout << "Directories size: " <<directories.size() << endl;
        for(vector<wstring>::iterator iter = directories.begin(), end = directories.end(); iter != end; ++iter) {
            saveDirectories(*iter, dirs);
        }
    }
}

void listFiles(const string &directory, const unsigned long int min_size, const unsigned long int max_size, const long long min_date, const long long max_date) {
    wstring dir(directory.begin(), directory.end());
    wstring tmp = dir + L"\\*";
    WIN32_FIND_DATAW file;
    HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);
    if (search_handle != INVALID_HANDLE_VALUE) {
        do {
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            unsigned long int fileSize = static_cast<unsigned long int>(file.nFileSizeLow);
//            long long fileDate = static_cast<long long>(file.ftCreationTime.dwLowDateTime);
            SYSTEMTIME stLocal, UTCTime;
            SystemTimeToFileTime(&UTCTime, &file.ftCreationTime);
            FileTimeToLocalFileTime(&file.ftCreationTime, &file.ftCreationTime);
            FileTimeToSystemTime(&file.ftCreationTime, &stLocal);
            string fileDate =
                    to_string(stLocal.wYear) + "-" + addCharacter(stLocal.wMonth) + "-" + addCharacter(stLocal.wDay) +
                    " " + addCharacter(stLocal.wHour) + ":" + addCharacter(stLocal.wMinute) + ":" +
                    addCharacter(stLocal.wSecond);
            long long fileDateMili = convertToMilisecond(fileDate);
            tmp = dir + L"\\" + wstring(file.cFileName);
            string str(tmp.begin(), tmp.end());

            if ((fileSize >= min_size && fileSize <= max_size) &&
                (fileDateMili >= min_date && fileDateMili <= max_date) &&  (!(file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))) {
//                cout << "Date created time string: " << fileDate << endl;
                cout << str << " " << fileSize << " " << stLocal.wYear << "-" << stLocal.wMonth << "-" << stLocal.wDay
                     << endl;
//                cout << fileDateMili << endl;
            }
        } while (FindNextFileW(search_handle, &file));

        FindClose(search_handle);
    }
}

int main()
{
    system("chcp 1250");
    wstring path;
    string min_date, max_date;
    cout << "Podaj sciezke: " << endl;
    wcin >> path;
    min_date = getMinDate();
    max_date = getMaxDate();
    min_date += " 00:00:00";
    max_date += " 23:59:59";
    long long min_date_mili = convertToMilisecond(min_date);
    long long max_date_mili = convertToMilisecond(max_date);
//    cout << min_date_mili << " " << max_date_mili << endl;
    unsigned long int min_size = getMinSize();
    unsigned long int max_size = getMaxSize();

    DWORD dw2;
    double diff;
    string firs_dir(path.begin(), path.end());
    vector<string> dirs;
    dirs.push_back(firs_dir);
    saveDirectories(path, dirs);
    omp_set_num_threads(8);
    string directories[dirs.size()];
//    for (vector<string>::iterator iter = dirs.begin(), end = dirs.end(); iter != end; ++iter) {
//        cout << *iter << endl;
//    }
    for (int i = 0; i < dirs.size(); i++) {
        directories[i] = dirs.at(i);
    }
    DWORD dw1 = GetTickCount();
    /*#pragma omp parallel for
    for (int i = 0; i < dirs.size() ; i++) {
        int id = omp_get_thread_num();
        listFiles(dirs[i], min_size, max_size, min_date_mili, max_date_mili);
        cout << "Hello from thread: " << id << " from " << omp_get_num_threads() << " threads" << endl;
    }*/
#pragma omp parallel
    {
#pragma omp sections
        {
#pragma omp section
            {
                for (int i = 0; i < (dirs.size() / 4); i++) {
//                int id = omp_get_thread_num();
                    cout << "Hello from section 0" << endl;
                    listFiles(dirs[i], min_size, max_size, min_date_mili, max_date_mili);
//                cout << "Hello from thread: " << id << " from " << omp_get_num_threads() << " threads" << endl;
                }
            }

#pragma omp section
            {
                for (int i = (dirs.size() / 4); i < (dirs.size() / 2); i++) {
//                int id = omp_get_thread_num();
                    cout << "Hello from section 1" << endl;
                    listFiles(dirs[i], min_size, max_size, min_date_mili, max_date_mili);
//                cout << "Hello from thread: " << id << " from " << omp_get_num_threads() << " threads" << endl;
                }
            }
#pragma omp section
            {
                for (int i = (dirs.size() / 2); i < ((dirs.size() / 2) + (dirs.size() / 4)); i++) {
//                int id = omp_get_thread_num();
                    cout << "Hello from section 2" << endl;
                    listFiles(dirs[i], min_size, max_size, min_date_mili, max_date_mili);
//                cout << "Hello from thread: " << id << " from " << omp_get_num_threads() << " threads" << endl;
                }
            }
#pragma omp section
            {
                for (int i = ((dirs.size() / 2) + (dirs.size() / 4)); i < dirs.size(); i++) {
//                int id = omp_get_thread_num();
                    cout << "Hello from section 3" << endl;
                    listFiles(dirs[i], min_size, max_size, min_date_mili, max_date_mili);
//                cout << "Hello from thread: " << id << " from " << omp_get_num_thread1s() << " threads" << endl;
                }
            }
        }
    }

    dw2 = GetTickCount();
    diff = (dw2 - dw1) / 1000;
    cout << "Czas ktory uplynal: " << diff << "s" <<endl;


    system("pause");
    return 0;
}

/* NOT USED */
//        #pragma omp parallel
//        {
//            auto it_v = directories.begin();
//        #pragma openmp for
//            for(; it_v != directories.end(); ++it_v)
//            {
//                FindFile(*it_v, minSize, maxSize, minDate, maxDate);
//            }
//        }