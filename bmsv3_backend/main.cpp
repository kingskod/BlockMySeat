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
    const char* sql_create_venues =
        "CREATE TABLE IF NOT EXISTS Venues ("
        "VenueID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"
        "Location TEXT,"
        "ImageURL TEXT,"
        "AuditoriumCount INTEGER);";
    if (sqlite3_exec(db, sql_create_venues, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error (Venues): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    const char* sql_create_showtimes =
        "CREATE TABLE IF NOT EXISTS Showtimes ("
        "ShowtimeID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "MovieID INTEGER,"
        "VenueID INTEGER,"
        "AuditoriumID INTEGER," // <-- ADDED THIS COLUMN
        "ShowtimeDateTime TEXT NOT NULL,"
        "FOREIGN KEY(MovieID) REFERENCES Movies(MovieID),"
        "FOREIGN KEY(VenueID) REFERENCES Venues(VenueID),"
        "FOREIGN KEY(AuditoriumID) REFERENCES Auditoriums(AuditoriumID));";
    if (sqlite3_exec(db, sql_create_showtimes, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error (Showtimes): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    const char* sql_create_auditoriums =
        "CREATE TABLE IF NOT EXISTS Auditoriums ("
        "AuditoriumID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "VenueID INTEGER,"
        "AuditoriumNumber INTEGER NOT NULL,"
        "SeatCount INTEGER,"
        "FOREIGN KEY(VenueID) REFERENCES Venues(VenueID));";
    if (sqlite3_exec(db, sql_create_auditoriums, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error (Auditoriums): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    // SEEDING FAKE DATA FOR TESTING
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

    // SEEDING FAKE VENUES FOR TESTING

    int venue_count = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM Venues", callback_is_empty, &venue_count, &zErrMsg);
    
    if (venue_count == 0) {
        std::cout << "Venues table is empty. Seeding with initial data..." << std::endl;
        const char* seed_sql =
            "INSERT INTO Venues (Name, Location, ImageURL, AuditoriumCount) VALUES "
            "('Blocky Multiplex', 'Downtown Cubeville', 'images/venue1.jpg', 12),"
            "('The Redstone Cinema', 'Oak Valley', 'images/venue2.jpg', 8),"
            "('Pixel Perfect Theaters', 'Glass Pane City', 'images/venue3.jpg', 16);";
        
        if (sqlite3_exec(db, seed_sql, 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error (Seeding Venues): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    }

    int showtime_count = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM Showtimes", callback_is_empty, &showtime_count, &zErrMsg);
    
    if (showtime_count == 0) {
        std::cout << "Showtimes table is empty. Seeding with initial data..." << std::endl;
        // Let's create showtimes for today (e.g., 2025-08-22)
        const char* seed_sql =
            "INSERT INTO Showtimes (MovieID, VenueID, ShowtimeDateTime) VALUES "
            // Movie 1 at Venue 1
            "(1, 1, '2025-08-22 10:00:00'), (1, 1, '2025-08-22 13:30:00'), (1, 1, '2025-08-22 17:00:00'),"
            // Movie 1 at Venue 2
            "(1, 2, '2025-08-22 11:00:00'), (1, 2, '2025-08-22 14:30:00'),"
            // Movie 2 at Venue 1
            "(2, 1, '2025-08-22 18:00:00'), (2, 1, '2025-08-22 21:00:00'),"
            // Movie 3 at Venue 3
            "(3, 3, '2025-08-22 19:00:00'), (3, 3, '2025-08-22 22:00:00'),"
            // Showtimes for tomorrow (e.g., 2025-08-23)
            "(1, 1, '2025-08-23 10:00:00'), (1, 3, '2025-08-23 15:00:00');";
        
        if (sqlite3_exec(db, seed_sql, 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error (Seeding Showtimes): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    }

    int auditorium_count = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM Auditoriums", callback_is_empty, &auditorium_count, &zErrMsg);
    if (auditorium_count == 0) {
        std::cout << "Auditoriums table is empty. Seeding..." << std::endl;
        const char* seed_sql =
            "INSERT INTO Auditoriums (VenueID, AuditoriumNumber, SeatCount) VALUES "
            // Auditoriums for Venue 1 (Blocky Multiplex)
            "(1, 1, 150), (1, 2, 150), (1, 3, 200),"
            // Auditoriums for Venue 2 (The Redstone Cinema)
            "(2, 1, 100), (2, 2, 120),"
            // Auditoriums for Venue 3 (Pixel Perfect Theaters)
            "(3, 1, 250), (3, 2, 250);";
        if (sqlite3_exec(db, seed_sql, 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error (Seeding Auditoriums): " << zErrMsg << std::endl;
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

    CROW_ROUTE(app, "/venues").methods("GET"_method)
    ([](){
        json venues_json = json::array();
        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT VenueID, Name, Location, ImageURL, AuditoriumCount FROM Venues";

        if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) 
        {
            while (sqlite3_step(stmt) == SQLITE_ROW) 
            {
                json venue;
                venue["id"] = sqlite3_column_int(stmt, 0);
                venue["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                venue["location"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                venue["image_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                venue["auditorium_count"] = sqlite3_column_int(stmt, 4);
                venues_json.push_back(venue);
            }
        }
        sqlite3_finalize(stmt);

        return crow::response(200, venues_json.dump());
    });
    CROW_ROUTE(app, "/movies/<int>")
([](int movieID){
    json movie_json;
    sqlite3_stmt* stmt;
    const char* sql_select = "SELECT MovieID, Title, PosterURL, Synopsis, DurationMinutes, Rating FROM Movies WHERE MovieID = ?";

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, movieID);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            movie_json["id"] = sqlite3_column_int(stmt, 0);
            movie_json["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            movie_json["poster_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            movie_json["synopsis"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            movie_json["duration_minutes"] = sqlite3_column_int(stmt, 4);
            movie_json["rating"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        }
    }
    sqlite3_finalize(stmt);

    if (movie_json.is_null()) {
        return crow::response(404, "Movie not found");
    }
    return crow::response(200, movie_json.dump());
});

// === NEW ENDPOINT 2: Get showtimes for a movie on a specific date ===
// === NEW ENDPOINT 2: Get showtimes for a movie on a specific date ===
CROW_ROUTE(app, "/showtimes")
([](const crow::request& req){
    // Get movie_id and date from the query string (e.g., /showtimes?movie_id=1&date=2025-08-22)
    auto movie_id_str = req.url_params.get("movie_id");
    auto date_str = req.url_params.get("date");

    if (!movie_id_str || !date_str) {
        return crow::response(400, "Missing movie_id or date parameter");
    }

    std::string sql = "SELECT T2.VenueID, T2.Name, T2.ImageURL, strftime('%H:%M', T1.ShowtimeDateTime) "
                  "FROM Showtimes AS T1, Venues AS T2 "
                  "WHERE T1.VenueID = T2.VenueID AND T1.MovieID = ? AND T1.ShowtimeDateTime LIKE ? || '%' "
                  "ORDER BY T2.VenueID, T1.ShowtimeDateTime";
    
    sqlite3_stmt* stmt;
    json venues_with_showtimes = json::object();

    // Add error checking for sqlite3_prepare_v2
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL PREPARE ERROR: " << sqlite3_errmsg(db) << std::endl;
        return crow::response(500, "Database query preparation failed");
    }

    sqlite3_bind_int(stmt, 1, std::stoi(movie_id_str));
    sqlite3_bind_text(stmt, 2, date_str, -1, SQLITE_STATIC);

    // Loop through the results
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int venue_id = sqlite3_column_int(stmt, 0);
        std::string venue_id_key = std::to_string(venue_id);

        // If we haven't seen this venue yet, create its object
        if (venues_with_showtimes.find(venue_id_key) == venues_with_showtimes.end()) {
            venues_with_showtimes[venue_id_key]["venue_id"] = venue_id;
            venues_with_showtimes[venue_id_key]["venue_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            venues_with_showtimes[venue_id_key]["venue_image_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            venues_with_showtimes[venue_id_key]["showtimes"] = json::array();
        }
        // Add the showtime to this venue's list
        venues_with_showtimes[venue_id_key]["showtimes"].push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
    }

    // Check for errors that might have occurred after the loop
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL EXECUTION ERROR: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    // Convert the object of venues into an array, which is easier for JavaScript to loop through
    json final_response = json::array();
    for (auto& el : venues_with_showtimes.items()) {
        final_response.push_back(el.value());
    }

    return crow::response(200, final_response.dump());
});

    // --- Run the app ---
    std::cout << "Server starting on port 18080..." << std::endl;
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
    return 0;
}