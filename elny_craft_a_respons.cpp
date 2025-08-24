#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class APIParser {
private:
    std::string apiEndpoint;
    std::vector<std::string> headers;
    std::map<std::string, std::string> queryParameters;

    CURL* curl;
    CURLcode res;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

public:
    APIParser(std::string endpoint) : apiEndpoint(endpoint), curl(curl_easy_init()) {}

    ~APIParser() {
        curl_easy_cleanup(curl);
    }

    void addHeader(std::string key, std::string value) {
        headers.push_back(key + ": " + value);
    }

    void addQueryParameter(std::string key, std::string value) {
        queryParameters[key] = value;
    }

    json parse() {
        std::string readBuffer;

        struct curl_slist* chunk = NULL;
        for (const auto& header : headers) {
            chunk = curl_slist_append(chunk, header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        std::string url = apiEndpoint;
        for (const auto& param : queryParameters) {
            url += "?" + param.first + "=" + param.second;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            return json();
        }

        json jsonData = json::parse(readBuffer);
        return jsonData;
    }
};

int main() {
    APIParser parser("https://api.example.com/data");
    parser.addHeader("Content-Type", "application/json");
    parser.addQueryParameter("key", "value");

    json data = parser.parse();
    std::cout << data.dump(4) << std::endl;

    return 0;
}