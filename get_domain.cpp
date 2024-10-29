#include <iostream>
#include <restclient-cpp/restclient.h>
#include <json/json.h>  // For JSON parsing

std::string getAvailableDomain() {
    std::string url = "https://api.mail.tm/domains";
    RestClient::Response response = RestClient::get(url);

    if (response.code != 200) {
        std::cerr << "Failed to fetch domains. Error code: " << response.code << std::endl;
        return "";
    }

    // Parse the JSON response
    Json::Reader reader;
    Json::Value jsonData;
    reader.parse(response.body, jsonData);

    // Retrieve the first domain in the list
    if (!jsonData["hydra:member"].empty()) {
        std::string domain = jsonData["hydra:member"][0]["domain"].asString();
        std::cout << "Available domain: " << domain << std::endl;
        return domain;
    } else {
        std::cerr << "No domains found in response." << std::endl;
        return "";
    }
}

int main() {
    std::string domain = getAvailableDomain();
    if (!domain.empty()) {
        std::cout << "Domain retrieved successfully: " << domain << std::endl;
    } else {
        std::cerr << "Failed to retrieve domain." << std::endl;
    }
    return 0;
}
