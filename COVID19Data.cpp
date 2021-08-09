#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <string>
#include <sstream>
#include <algorithm>
#include <curl/curl.h>
#include <windows.h>

class Covid19Data
{
    std::string prefecture;
    int date;
    int infection;
    int dead;

public:
    void setPrefecture(const std::string& newPrefecture) {
        prefecture = newPrefecture;
    }
    void setDate(int newDate) {
        date = newDate;
    }
    void setInfection(int newInfection) {
        infection = newInfection;
    }
    void setDead(int newDead) {
        dead = newDead;
    }
    std::string getPrefecture() {
        return prefecture;
    }
    int getDate() {
        return date;
    }
    int getInfection() {
        return infection;
    }
    int getDead() {
        return dead;
    }
};

std::string UTF8toSjis(std::string srcUTF8)
{
    int lenghtUnicode = MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), srcUTF8.size() + 1, NULL, 0);
    wchar_t* bufUnicode = new wchar_t[lenghtUnicode];
    MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), srcUTF8.size() + 1, bufUnicode, lenghtUnicode);

    int lengthSJis = WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, -1, NULL, 0, NULL, NULL);
    char* bufShiftJis = new char[lengthSJis];
    WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, lenghtUnicode + 1, bufShiftJis, lengthSJis, NULL, NULL);
    std::string strSJis(bufShiftJis);

    delete bufUnicode;
    delete bufShiftJis;

    return strSJis;
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userp)
{
    std::vector<char>* recvBuffer = (std::vector<char>*)userp;
    const size_t sizes = size * nmemb;
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr + sizes);
    return sizes;
}

void downloadFile(std::string url)
{
    CURL* handle = curl_easy_init();
    if (handle != nullptr)
    {
        std::vector<char> responseData;
        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &responseData);
        CURLcode result = curl_easy_perform(handle);
        if (result == CURLE_OK)
        {
            curl_easy_cleanup(handle);
            std::ofstream ofs("covid19.txt", std::ios::out);
            ofs << responseData.data() << std::endl;
        }
        else
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(result) << '\n';
            curl_easy_cleanup(handle);
        }
    }
    else
    {
        curl_easy_cleanup(handle);
    }
}

std::vector<Covid19Data> readFile()
{
    std::vector<Covid19Data> cData;
    std::ifstream ifs("covid19.txt");
    std::string line;
    std::regex rx(R"(,|/)");
    std::getline(ifs, line);

    while (std::getline(ifs, line))
    {
        line = UTF8toSjis(line);

        if (!std::regex_search(line, rx)) { break; }

        /*方法① (処理にとても時間がかかる)
        std::sregex_token_iterator it(line.begin(), line.end(), rx, -1);
        std::sregex_token_iterator end;
        int date = (stoi(*it++) * 10000) + (stoi(*it++) * 100) + (stoi(*it++));
        cData.emplace_back();
        cData[cData.size() - 1].setDate(date);
        cData[cData.size() - 1].setPrefecture(*(++it));
        cData[cData.size() - 1].setInfection(stoi(*(++it))); it++;
        cData[cData.size() - 1].setDead(stoi(*(++it)));
        */

        //方法②
        int bDate1, bDate2, bDate3, bInfection, bDead, other1, other2, other3;
        char prefecture[64];
        sscanf_s(line.c_str(), "%d/%d/%d,%d,%[^,],%d,%d,%d,%d", &bDate1, &bDate2, &bDate3, &other1, prefecture, 64, &bInfection, &other2, &bDead, &other3);
        int date = (bDate1 * 10000) + (bDate2 * 100) + (bDate3);
        cData.emplace_back();
        cData[cData.size() - 1].setDate(date);
        cData[cData.size() - 1].setPrefecture(prefecture);
        cData[cData.size() - 1].setInfection(bInfection);
        cData[cData.size() - 1].setDead(bDead);
    }

    return cData;
}

int main()
{
    std::string url = "https://www3.nhk.or.jp/n-data/opendata/coronavirus/nhk_news_covid19_prefectures_daily_data.csv";

    downloadFile(url);
    std::vector<Covid19Data> covid19Data = readFile();

    std::string fPrefecture;
    int fDate, a, b, c;
    std::cout << "都道府県を入力してください\n ";
    std::cin >> fPrefecture;
    std::cout << "日付を入力してください\n 西暦;";
    std::cin >> a;
    std::cout << " 月:";
    std::cin >> b;
    std::cout << " 日:";
    std::cin >> c;
    fDate = (a * 10000) + (b * 100) + c;

    size_t s = covid19Data.size();
    int index1;
    std::vector<std::string> covid19Prefecture(s);
    for (size_t i = 0; i < s; i++)
    {
        covid19Prefecture[i] = covid19Data[i].getPrefecture();
    }
    auto itr1 = std::find(covid19Prefecture.begin(), covid19Prefecture.end(), fPrefecture);
    if (itr1 != covid19Prefecture.end())
    {
        index1 = std::distance(covid19Prefecture.begin(), itr1);

        int index2;
        std::vector<int> covid19Date(s);
        for (size_t i = 0; i < s; i++)
        {
            covid19Date[i] = covid19Data[i].getDate();
        }
        auto itr2 = std::find(covid19Date.begin() + index1, covid19Date.begin() + index1 + (s / 47), fDate);
        if (itr2 != covid19Date.end())
        {
            index2 = std::distance(covid19Date.begin(), itr2);
        }
        else
        {
            std::cout << "見つかりません\n";
        }

        std::cout << "新規感染者数:" << covid19Data[index2].getInfection() << "人" << std::endl;
        std::cout << "新規死者数:" << covid19Data[index2].getDead() << "人" << std::endl;
    }
    else
    {
        std::cout << "見つかりません\n";
    }
}
