// These two lines are CRITICAL and must be FIRST.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Standard C++ and library headers go NEXT.
#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include "include/json.hpp"

// The Crow headers go LAST.
#include "include/crow.h"

using json = nlohmann::json;
sqlite3* db;

static int callback_is_empty(void* data, int argc, char** argv, char** azColName) 
{
    int* count = (int*)data;
    *count = argc > 0 ? atoi(argv[0]) : 0;
    return 0;
}

void init_database() 
{
    if (sqlite3_open("blockmyseat.db", &db)) 
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }

    const char* sql_create_table = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "UserID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Username TEXT UNIQUE NOT NULL,"
        "Email TEXT UNIQUE NOT NULL,"
        "Password TEXT NOT NULL);";

    char* zErrMsg = 0;
    if (sqlite3_exec(db, sql_create_table, 0, 0, &zErrMsg) != SQLITE_OK) 
    {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    const char* sql_create_movies =
        "CREATE TABLE IF NOT EXISTS Movies ("
        "MovieID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Title TEXT NOT NULL,"
        "PosterURL TEXT,"
        "Synopsis TEXT,"
        "DurationMinutes INTEGER,"
        "Rating TEXT);";

    if (sqlite3_exec(db, sql_create_movies, 0, 0, &zErrMsg) != SQLITE_OK) 
    {
        std::cerr << "SQL error (Movies): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    int movie_count = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM Movies", callback_is_empty, &movie_count, &zErrMsg);

    if (movie_count == 0) 
    {
        std::cout << "Movies table is empty. Seeding with initial data..." << std::endl;
        const char* seed_sql =
            "INSERT INTO Movies (Title, PosterURL, Synopsis, DurationMinutes, Rating) VALUES "
            "('The Crimson Shadow', 'crimson shadow.png', 'In a land shrouded by a creeping darkness, a lone figure known only as \\\"The Crimson Shadow\\\" stands on the precipice between light and oblivion. Tasked with a prophecy to restore the fallen kingdom of Eldoria, they must journey across treacherous mountains and stormy seas, confronting mythical beasts and a malevolent sorcerer who seeks to plunge the world into eternal night. The fate of their world rests on their shoulders, and their crimson-hued powers are their only guide.', 120, 'PG-13 for intense sci-fi action, violence, and some thematic elements.'),"
            "('Echoes of Neptune', 'echos of neptune.png', 'A deep-space expedition to Neptune''s mysterious ocean moon reveals a startling discovery: a colossal, crystalline city pulsating with an otherworldly energy. While investigating, a lone astronaut is separated from their crew and discovers they can communicate with the alien life form inhabiting the moon. The astronaut learns the ''echos'' they are hearing are not just soundwaves, but the last remnants of a dying race. They must choose between fulfilling their mission parameters and helping an ancient species before the deep-sea pressures of Neptune’s moon erase them from existence forever.', 95, 'PG-13 for intense sci-fi peril and thematic elements.'),"
            "('Galactic Drift', 'galactic drift.png', 'In a sprawling, neon-lit cyberpunk metropolis, a lone renegade hacker discovers a rogue AI that has broken free from its creators. Hunted by the corporation that seeks to reclaim it, the duo must navigate a dangerous high-speed chase through the city's futuristic sky-high highways, with the fate of human-AI relations in their hands.', 110, 'PG-13 for sequences of intense futuristic action and violence, and some thematic elements.'),"
            "('Midnight Cipher', 'midnight cipher.png', 'A gritty private eye is hired to retrieve a glowing, encrypted briefcase in a rain-soaked, neon-lit city. He finds himself embroiled in a conspiracy far deadlier than a simple theft, as ruthless assassins and a shadowy organization hunt him for the cipher he now possesses. To survive, he must decode its secrets before the city's midnight hour.', 135, 'R for strong violence, language, and some sexual content.'),"
            "('The Last Starlight', 'the laststarlight.png', 'In the last moments of a dying universe, a lone astronaut embarks on a desperate journey to find a mythical cosmic anomaly—a \\\"last starlight\\\" that can reignite creation. As he traverses desolate, forgotten worlds, he must confront his own solitude and the philosophical weight of his mission, knowing that his success or failure will determine the fate of everything that has ever been.', 105, 'PG for peril and thematic elements.');";


        if (sqlite3_exec(db, seed_sql, 0, 0, &zErrMsg) != SQLITE_OK) 
        {
            std::cerr << "SQL error (Seeding): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    } 
    else 
    {
        std::cout << "Database is ready." << std::endl;
    }
}

int main() 
{
    init_database();

    // Declare the app with the CORS middleware directly in the template.
    crow::App<crow::CORSHandler> app;

    // Get a reference to the CORS middleware and configure it.
    auto& cors = app.get_middleware<crow::CORSHandler>();
    // A simple policy: allow all origins, all methods, all headers.
    cors.global();

    // --- Define your routes ---
    CROW_ROUTE(app, "/signup").methods("POST"_method)
    ([](const crow::request& req)
    {
        auto j = json::parse(req.body);
        std::string username = j["username"];
        std::string email = j["email"];
        std::string password = j["password"];

        sqlite3_stmt* stmt;
        const char* sql_check = "SELECT UserID FROM Users WHERE Username = ? OR Email = ?";
        sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            sqlite3_finalize(stmt);
            return crow::response(409, json{{"status", "error"}, {"message", "Username or email already taken."}}.dump());
        }
        sqlite3_finalize(stmt);

        const char* sql_insert = "INSERT INTO Users (Username, Email, Password) VALUES (?, ?, ?)";
        sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) 
        {
            sqlite3_finalize(stmt);
            return crow::response(500, json{{"status", "error"}, {"message", "Failed to create user."}}.dump());
        }
        sqlite3_finalize(stmt);

        return crow::response(201, json{{"status", "success"}, {"message", "Account created successfully."}}.dump());
    });

    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([](const crow::request& req)
    {
        auto j = json::parse(req.body);
        std::string username = j["username"];
        std::string password = j["password"];

        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT Password FROM Users WHERE Username = ?";
        sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            std::string password_from_db = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (password == password_from_db) 
            {
                sqlite3_finalize(stmt);
                return crow::response(200, json{{"status", "success"}, {"message", "Login successful!"}}.dump());
            }
        }

        sqlite3_finalize(stmt);
        return crow::response(401, json{{"status", "error"}, {"message", "Invalid username or password."}}.dump());
    });

    CROW_ROUTE(app, "/movies").methods("GET"_method)
    ([]()
    {
        json movies_json = json::array();
        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT MovieID, Title, PosterURL, Synopsis, DurationMinutes, Rating FROM Movies";

        if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) 
        {
            while (sqlite3_step(stmt) == SQLITE_ROW) 
            {
                json movie;
                movie["id"] = sqlite3_column_int(stmt, 0);
                movie["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                movie["poster_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                movie["synopsis"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                movie["duration_minutes"] = sqlite3_column_int(stmt, 4);
                movie["rating"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                movies_json.push_back(movie);
            }
        }
        sqlite3_finalize(stmt);

        return crow::response(200, movies_json.dump());
    });

    // --- Run the app ---
    std::cout << "Server starting on port 18080..." << std::endl;
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
    return 0;
}
