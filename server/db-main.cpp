#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bson.h>
#include <bsoncxx/json.hpp>

using namespace std;

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

// Connect to MongoDB
mongocxx::instance inst;
auto client = mongocxx::client{mongocxx::uri{"mongodb+srv://flav:1984@cluster0.wd3vd.mongodb.net/chess?retryWrites=true&w=majority"}};

// Retrieve chess database and users collection
mongocxx::database db = client["chess"];
mongocxx::collection users = db["users"];

int authenticateUser (string username, string password) {
    mongocxx::cursor maybe_result =
            users.find(make_document(kvp("username", username)));

    int logged = 0;

    for (auto doc : maybe_result) {
        string pw = std::string(doc["password"].get_utf8().value);
        if (pw == password) logged = 1;
    }

    return logged;
}

int registerUser (string username, string password) {
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
            users.find_one(make_document(kvp("username", username)));

    if (maybe_result) {
        return 0;
    }
    else {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
                users.insert_one(make_document(kvp("username", username), kvp("password", password)));

        cout << "registered" << endl;
    }
}

int main() {
    cout << authenticateUser("user", "testsad");

    return 0;
}