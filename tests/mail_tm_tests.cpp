#include <gtest/gtest.h>
#include "MailTM.h"
#include <chrono>
#include <thread>
#include <optional>
#include <functional>
#include <iostream>

// Helper function to generate unique usernames
std::string generateUniqueUsername() {
    return "user_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}

// Retry helper function
template <typename Func>
auto retry(Func func, int retries = 3, std::chrono::milliseconds delay = std::chrono::seconds(2)) {
    using ResultType = decltype(func());
    if constexpr (std::is_same_v<ResultType, std::optional<typename ResultType::value_type>>) {
        // Handle std::optional
        for (int i = 0; i < retries; ++i) {
            auto result = func();
            if (result.has_value()) return result;
            std::this_thread::sleep_for(delay);
        }
        return ResultType();
    } else {
        // Handle non-optional types
        for (int i = 0; i < retries; ++i) {
            auto result = func();
            if (!result.empty()) return result;
            std::this_thread::sleep_for(delay);
        }
        return ResultType(); // Return an empty container if all retries fail
    }
}

// Test getAvailableDomain
TEST(MailTMTests, GetAvailableDomainTest) {
    MailTM mailTm;
    std::string domain = mailTm.getAvailableDomain();

    // Check that a domain is returned and is not empty
    EXPECT_FALSE(domain.empty()) << "Domain should not be empty";
}

// Test registerEmail
TEST(MailTMTests, RegisterEmailTest) {
    MailTM mailTm;
    std::string username = generateUniqueUsername();
    std::string password = "testpassword";

    auto domain = mailTm.getAvailableDomain();
    ASSERT_FALSE(domain.empty()) << "Domain should not be empty";

    std::string email = username + "@" + domain;

    auto accountId = retry([&]() { return mailTm.registerEmail(email, password); });
    ASSERT_TRUE(accountId.has_value()) << "Account ID should be returned";
    EXPECT_FALSE(accountId->empty()) << "Account ID should not be empty";
}

// Test authenticate
TEST(MailTMTests, AuthenticateTest) {
    MailTM mailTm;
    std::string username = generateUniqueUsername();
    std::string password = "testpassword";

    auto domain = mailTm.getAvailableDomain();
    ASSERT_FALSE(domain.empty()) << "Domain should not be empty";

    std::string email = username + "@" + domain;

    auto accountId = retry([&]() { return mailTm.registerEmail(email, password); });
    ASSERT_TRUE(accountId.has_value()) << "Account ID should be returned";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto token = retry([&]() { return mailTm.authenticate(email, password); });
    ASSERT_TRUE(token.has_value()) << "Token should be returned";
    EXPECT_FALSE(token->empty()) << "Token should not be empty";
}

// Test checkInbox
TEST(MailTMTests, CheckInboxTest) {
    MailTM mailTm;
    std::string username = generateUniqueUsername();
    std::string password = "testpassword";

    auto domain = mailTm.getAvailableDomain();
    ASSERT_FALSE(domain.empty()) << "Domain should not be empty";

    std::string email = username + "@" + domain;

    auto accountId = retry([&]() { return mailTm.registerEmail(email, password); });
    ASSERT_TRUE(accountId.has_value()) << "Account ID should be returned";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto token = retry([&]() { return mailTm.authenticate(email, password); });
    ASSERT_TRUE(token.has_value()) << "Authentication failed";

    auto messages = retry([&]() { return mailTm.checkInbox(token.value()); });
    EXPECT_GE(messages.size(), 0) << "Inbox size should be valid";
}

// Test getAccountId
TEST(MailTMTests, GetAccountIdTest) {
    MailTM mailTm;
    std::string username = generateUniqueUsername();
    std::string password = "testpassword";

    auto domain = mailTm.getAvailableDomain();
    ASSERT_FALSE(domain.empty()) << "Domain should not be empty";

    std::string email = username + "@" + domain;

    auto accountId = retry([&]() { return mailTm.registerEmail(email, password); });
    ASSERT_TRUE(accountId.has_value()) << "Account ID should be returned";

    std::this_thread::sleep_for(std::chrono::seconds(4));

    auto token = retry([&]() { return mailTm.authenticate(email, password); });
    ASSERT_TRUE(token.has_value()) << "Authentication failed";

    auto fetchedAccountId = retry([&]() { return mailTm.getAccountId(token.value()); });
    ASSERT_TRUE(fetchedAccountId.has_value()) << "Account ID should be returned";
    EXPECT_FALSE(fetchedAccountId->empty()) << "Account ID should not be empty";
}

// Test deleteAccount
TEST(MailTMTests, DeleteAccountTest) {
    MailTM mailTm;
    std::string username = generateUniqueUsername();
    std::string password = "testpassword";

    auto domain = mailTm.getAvailableDomain();
    ASSERT_FALSE(domain.empty()) << "Domain should not be empty";

    std::string email = username + "@" + domain;

    auto accountId = retry([&]() { return mailTm.registerEmail(email, password); });
    ASSERT_TRUE(accountId.has_value()) << "Account ID should be returned";

    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto token = retry([&]() { return mailTm.authenticate(email, password); });
    ASSERT_TRUE(token.has_value()) << "Authentication failed";

    auto fetchedAccountId = retry([&]() { return mailTm.getAccountId(token.value()); });
    ASSERT_TRUE(fetchedAccountId.has_value()) << "Account ID should be returned";

    auto deleteResult = retry([&]() { return mailTm.deleteAccount(token.value(), fetchedAccountId.value()); });
    ASSERT_TRUE(deleteResult.has_value()) << "Account deletion failed";
    EXPECT_EQ(deleteResult.value(), "Account deleted successfully");
}
