#include <iostream>
#include <restclient-cpp/restclient.h>
#include <json/json.h>
#include <ctime>

// Function to generate a random username
std::string generateRandomUsername() {
    const int length = 8;  // Length of the username
    std::string username;
    srand(time(0));  // Seed for randomness
    for (int i = 0; i < length; i++) {
        char c = 'a' + rand() % 26;  // Random lowercase letter
        username += c;
    }
    return username;
}

// Function to register the email with Mail.tm
std::string registerEmail(const std::string& domain) {
    std::string username = generateRandomUsername();
    std::string email = username + "@" + domain;

    // Prepare the JSON payload for the POST request
    Json::Value requestData;
    requestData["address"] = email;
    requestData["password"] = "temporary_password";  // Using a default password

    Json::StreamWriterBuilder writer;
    std::string requestBody = Json::writeString(writer, requestData);

    // Send the POST request to register the email
    RestClient::Response response = RestClient::post("https://api.mail.tm/accounts", "application/json", requestBody);

    if (response.code == 201) {
        std::cout << "Email registered successfully: " << email << std::endl;
        return email;
    } else {
        std::cerr << "Failed to register email. Response code: " << response.code << std::endl;
        return "";
    }
}

int main() {
    std::string domain = "livinitlarge.net";  // Example domain, replace with one retrieved earlier
    std::string email = registerEmail(domain);

    if (!email.empty()) {
        std::cout << "Temporary email created: " << email << std::endl;
    } else {
        std::cerr << "Could not create a temporary email." << std::endl;
    }

    return 0;
}

/*to compile and run: 
g++ -o register_email register_email.cpp -I/opt/homebrew/Cellar/jsoncpp/1.9.6/include -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib -lrestclient-cpp -ljsoncpp -std=c++11
./register_email
*/
