#include <iostream>
#include <restclient-cpp/restclient.h>
#include <json/json.h>

// Function to fetch an available domain dynamically
std::string getAvailableDomain() {
    std::string url = "https://api.mail.tm/domains";
    RestClient::Response response = RestClient::get(url);

    if (response.code != 200) {
        std::cerr << "Failed to fetch domains. Error code: " << response.code << std::endl;
        return "";
    }

    // Parse the JSON response
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errors;
    std::istringstream responseStream(response.body);

    if (!Json::parseFromStream(readerBuilder, responseStream, &jsonData, &errors)) {
        std::cerr << "Error parsing domains JSON: " << errors << std::endl;
        return "";
    }

    // Retrieve the first domain in the list
    if (!jsonData["hydra:member"].empty()) {
        std::string domain = jsonData["hydra:member"][0]["domain"].asString();
        std::cout << "Using domain: " << domain << std::endl;
        return domain;
    } else {
        std::cerr << "No domains found in the response." << std::endl;
        return "";
    }
}

// Function to register the email with Mail.tm
std::string registerEmail(const std::string& domain) {
    while (true) {
        std::string username;

        // Prompt the user for their desired username
        std::cout << "Enter your desired username (before @" << domain << "): ";
        std::cin >> username;

        std::string email = username + "@" + domain;

        // Prepare the JSON payload for the POST request
        Json::Value requestData;
        requestData["address"] = email;
        requestData["password"] = "temporary_password";  // Default password for the account

        Json::StreamWriterBuilder writer;
        std::string requestBody = Json::writeString(writer, requestData);

        // Send the POST request to register the email
        RestClient::Response response = RestClient::post("https://api.mail.tm/accounts", "application/json", requestBody);

        if (response.code == 201) {
            std::cout << "Email registered successfully: " << email << std::endl;
            return email;
        } else if (response.code == 422) {
            // Parse the error message to check if the address is already used
            Json::CharReaderBuilder readerBuilder;
            Json::Value responseData;
            std::string errors;
            std::istringstream responseStream(response.body);

            if (Json::parseFromStream(readerBuilder, responseStream, &responseData, &errors)) {
                std::string detail = responseData["detail"].asString();
                if (detail.find("This value is already used") != std::string::npos) {
                    std::cerr << "The email address " << email << " is already in use. Please try another username." << std::endl;
                } else {
                    std::cerr << "Failed to register email: " << detail << std::endl;
                    return "";
                }
            } else {
                std::cerr << "Failed to parse server response." << std::endl;
                return "";
            }
        } else {
            std::cerr << "Failed to register email. Response code: " << response.code << std::endl;
            std::cerr << "Server response: " << response.body << std::endl;
            return "";
        }
    }
}

int main() {
    // Dynamically fetch the available domain
    std::string domain = getAvailableDomain();
    if (domain.empty()) {
        std::cerr << "Could not fetch a valid domain. Exiting." << std::endl;
        return 1;
    }

    // Call the function to register the email
    std::string email = registerEmail(domain);

    if (!email.empty()) {
        std::cout << "Temporary email created: " << email << std::endl;
    } else {
        std::cerr << "Could not create a temporary email." << std::endl;
    }

    return 0;
}

/*g++ -o get_domain get_domain.cpp -lcurl -I/opt/homebrew/Cellar/jsoncpp/1.9.6/include -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib -ljsoncpp -std=c++11 -L/path/to/restclient/lib -lrestclient-cpp
./get_domain*/