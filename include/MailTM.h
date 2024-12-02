#pragma once
#include <string>
#include <vector>
#include <optional>
#include <json/json.h>

/**
 * @file MailTM.h
 * @brief Provides an interface for interacting with the Mail.tm API.
 */

namespace MailTMAPI {

/**
 * @class MailTM
 * @brief A class to interact with the Mail.tm API.
 *
 * This class provides methods to register email accounts, authenticate users,
 * retrieve messages, and manage accounts.
 */
class MailTM {
private:
    /**
     * @brief Callback function for writing CURL response data.
     * @param contents Pointer to the data.
     * @param size Size of each data element.
     * @param nmemb Number of data elements.
     * @param userp Pointer to the string buffer.
     * @return The number of bytes written.
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    /**
     * @brief Sends an HTTP request to the Mail.tm API.
     * @param url The endpoint URL.
     * @param method The HTTP method (e.g., "GET", "POST").
     * @param payload The request payload (optional).
     * @param authToken The authentication token (optional).
     * @return The response body as a string.
     */
    std::string sendRequest(const std::string& url, const std::string& method,
                            const std::string& payload = "", const std::string& authToken = "");

    const std::string baseUrl = "https://api.mail.tm"; /**< Base URL for the Mail.tm API. */

public:
    /**
     * @brief Default constructor.
     */
    MailTM() = default;

    /**
     * @brief Default destructor.
     */
    ~MailTM() = default;

    /**
     * @brief Fetches an available email domain.
     * @return The domain string if successful, or an empty string otherwise.
     */
    std::string getAvailableDomain();

    /**
     * @brief Registers a new email account.
     * @param username The email username.
     * @param password The email password.
     * @return The account ID if successful, or std::nullopt otherwise.
     */
    std::optional<std::string> registerEmail(const std::string& username, const std::string& password);

    /**
     * @brief Authenticates an email account.
     * @param email The email address.
     * @param password The email password.
     * @return The authentication token if successful, or std::nullopt otherwise.
     */
    std::optional<std::string> authenticate(const std::string& email, const std::string& password);

    /**
     * @brief Retrieves the list of messages in the inbox.
     * @param token The authentication token.
     * @return A vector of JSON objects representing the messages.
     */
    std::vector<Json::Value> checkInbox(const std::string& token);

    /**
     * @brief Gets the account ID of the authenticated user.
     * @param token The authentication token.
     * @return The account ID if successful, or std::nullopt otherwise.
     */
    std::optional<std::string> getAccountId(const std::string& token);

    /**
     * @brief Deletes the authenticated user's account.
     * @param token The authentication token.
     * @param accountId The account ID to delete.
     * @return A success message if successful, or std::nullopt otherwise.
     */
    std::optional<std::string> deleteAccount(const std::string& token, const std::string& accountId);

    /**
     * @brief Retrieves a specific message by its ID.
     * @param token The authentication token.
     * @param messageId The ID of the message to retrieve.
     * @return A JSON object representing the message.
     */
    Json::Value getMessage(const std::string& token, const std::string& messageId);
};

} // namespace MailTMAPI
