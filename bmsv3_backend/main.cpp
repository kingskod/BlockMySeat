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
            "('Minecraft: The First Movie', 'images/poster1.jpg', 'The story of a hero who must save the Overworld from the Ender Dragon.', 120, 'PG'),"
            "('Creeper''s Revenge', 'images/poster2.jpg', 'A misunderstood Creeper just wants to make friends, but things keep exploding.', 95, 'PG-13'),"
            "('The Enderman''s Gaze', 'images/poster3.jpg', 'A psychological thriller about what happens when you look for too long.', 110, 'R'),"
            "('Village & Pillage', 'images/poster4.jpg', 'An epic saga of villagers defending their home from a relentless raid.', 135, 'PG-13'),"
            "('Journey to the Nether', 'images/poster5.jpg', 'Two adventurers brave the fiery depths to find a rare artifact.', 105, 'PG');";

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