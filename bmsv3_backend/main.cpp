// These two lines are CRITICAL and must be FIRST.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Standard C++ and library headers go NEXT.
#include <iostream>
#include <string>
#include <random> 
#include <sstream>
#include <vector>
#include <sqlite3.h>
#include "include/json.hpp"
#include "seed_data.hpp"
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
std::string generate_session_token() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << distrib(gen);
    }
    return ss.str();
}

void init_database() {
    if (sqlite3_open("blockmyseat.db", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
    char* zErrMsg = 0;

    // --- All Table Schemas, Fully Defined ---
    const char* schemas[] = {
        "CREATE TABLE IF NOT EXISTS Users ("
        "UserID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Username TEXT NOT NULL,"
        "Email TEXT NOT NULL,"
        "Password TEXT NOT NULL,"
        "SessionToken TEXT);",

        "CREATE TABLE IF NOT EXISTS Movies ("
        "MovieID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Title TEXT NOT NULL,"
        "PosterURL TEXT,"
        "Synopsis TEXT,"
        "DurationMinutes INTEGER,"
        "Rating TEXT);",

        "CREATE TABLE IF NOT EXISTS Venues ("
        "VenueID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"
        "Location TEXT,"
        "ImageURL TEXT,"
        "AuditoriumCount INTEGER,"
        "Rating REAL);",

        "CREATE TABLE IF NOT EXISTS AuditoriumTemplates ("
        "TemplateID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Description TEXT,"
        "PremiumRows INTEGER,"
        "NormalRows INTEGER,"
        "Section1Seats INTEGER,"
        "Section2Seats INTEGER,"
        "Section3Seats INTEGER);",

        "CREATE TABLE IF NOT EXISTS Auditoriums ("
        "AuditoriumID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "VenueID INTEGER,"
        "AuditoriumNumber INTEGER NOT NULL,"
        "TemplateID INTEGER,"
        "NormalPrice REAL,"
        "PremiumPrice REAL,"
        "FOREIGN KEY(VenueID) REFERENCES Venues(VenueID),"
        "FOREIGN KEY(TemplateID) REFERENCES AuditoriumTemplates(TemplateID));",

        "CREATE TABLE IF NOT EXISTS Showtimes ("
        "ShowtimeID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "MovieID INTEGER,"
        "VenueID INTEGER,"
        "AuditoriumID INTEGER,"
        "ShowtimeDateTime TEXT NOT NULL,"
        "FOREIGN KEY(MovieID) REFERENCES Movies(MovieID),"
        "FOREIGN KEY(VenueID) REFERENCES Venues(VenueID),"
        "FOREIGN KEY(AuditoriumID) REFERENCES Auditoriums(AuditoriumID));",

        "CREATE TABLE IF NOT EXISTS Bookings ("
        "BookingID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ShowtimeID INTEGER,"
        "UserID INTEGER,"
        "SeatIdentifier TEXT NOT NULL,"
        "FOREIGN KEY(ShowtimeID) REFERENCES Showtimes(ShowtimeID),"
        "FOREIGN KEY(UserID) REFERENCES Users(UserID));"
    };

    for (const char* schema : schemas) {
        if (sqlite3_exec(db, schema, 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error (Schema Creation): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
            return;
        }
    }
    std::cout << "All table schemas are ready." << std::endl;

    // Check if the database is already seeded by looking for any user
     int user_count = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM Users", callback_is_empty, &user_count, &zErrMsg);

    if (user_count == 0) {
        std::cout << "Database is empty. Seeding with master data..." << std::endl;
        
        
        if (sqlite3_exec(db, data::getUsersSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Users): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Users seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::getMoviesSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Movies): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Movies seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::getVenuesSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Venues): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Venues seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::getAuditoriumTemplatesSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding AuditoriumTemplates): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "AuditoriumTemplates seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::getAuditoriumsSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Auditoriums): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Auditoriums seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::generateShowtimesSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Showtimes): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Showtimes seeded successfully." << std::endl;

        if (sqlite3_exec(db, data::getBookingsSQL().c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "FATAL SQL ERROR (Seeding Bookings): " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg); return;
        }
        std::cout << "Bookings seeded successfully." << std::endl;
    }

    std::cout << "Database is ready." << std::endl;
}
int main() 
{
    init_database();

    // Declare the app with the CORS middleware directly in the template.
    crow::App<crow::CORSHandler> app;

    // Get a reference to the CORS middleware and configure it.
    auto& cors = app.get_middleware<crow::CORSHandler>();
    // A simple policy: allow all origins, all methods, all headers.
    cors
    .global()
    .headers("Content-Type") // Allow the frontend to specify the content type
    .methods("POST"_method, "GET"_method, "OPTIONS"_method) // Allow these HTTP methods
    .origin("*"); // Allow any origin (including file://)

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
    ([](const crow::request& req){
        auto j = json::parse(req.body);
        std::string username = j["username"];
        std::string password = j["password"];

        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT UserID, Password FROM Users WHERE Username = ?";
        
        if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) != SQLITE_OK) {
            return crow::response(500, "DB error");
        }
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int userId = sqlite3_column_int(stmt, 0);
            std::string password_from_db = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            if (password == password_from_db) {
                sqlite3_finalize(stmt);
                std::string token = generate_session_token();
                
                // Store token in DB
                const char* sql_update = "UPDATE Users SET SessionToken = ? WHERE UserID = ?";
                sqlite3_prepare_v2(db, sql_update, -1, &stmt, 0);
                sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 2, userId);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                json res_json;
                res_json["status"] = "success";
                res_json["message"] = "Login successful!";
                res_json["token"] = token;
                res_json["userId"] = userId;
                return crow::response(200, res_json.dump());
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
    ([]()
    {
        json venues_json = json::array();
        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT VenueID, Name, Location, ImageURL, AuditoriumCount, Rating FROM Venues";

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
                venue["rating"] = sqlite3_column_double(stmt, 5);
                venues_json.push_back(venue);
            }
        }
        sqlite3_finalize(stmt);

        return crow::response(200, venues_json.dump());
    });

    CROW_ROUTE(app, "/venues/<int>")
    ([](int venueID){
        json venue_json;
        sqlite3_stmt* stmt;
        const char* sql_select = "SELECT VenueID, Name, Location, ImageURL, AuditoriumCount, Rating FROM Venues WHERE VenueID = ?";

        if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, venueID);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                venue_json["id"] = sqlite3_column_int(stmt, 0);
                venue_json["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                venue_json["location"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                venue_json["image_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                venue_json["auditorium_count"] = sqlite3_column_int(stmt, 4);
                venue_json["rating"] = sqlite3_column_double(stmt, 5);
            }
        }
        sqlite3_finalize(stmt);

        if (venue_json.is_null()) {
            return crow::response(404, "Venue not found");
        }
        return crow::response(200, venue_json.dump());
    });
    CROW_ROUTE(app, "/venue-showtimes")
    ([](const crow::request& req){
        auto venue_id_str = req.url_params.get("venue_id");
        auto date_str = req.url_params.get("date");

        if (!venue_id_str) {
            return crow::response(400, "Missing venue_id parameter");
        }
        if(!date_str) {
            return crow::response(400, "Missing date parameter");
        }

        std::string sql = "SELECT "
                          "  M.MovieID, M.Title, M.PosterURL, M.DurationMinutes, M.Rating, M.Synopsis, "
                          "  strftime('%H:%M', S.ShowtimeDateTime), S.ShowtimeID, S.AuditoriumID "
                          "FROM Showtimes AS S "
                          "JOIN Movies AS M ON S.MovieID = M.MovieID "
                          "WHERE S.VenueID = ? AND S.ShowtimeDateTime LIKE ? || '%' "
                          "ORDER BY M.MovieID, S.ShowtimeDateTime";
        
        sqlite3_stmt* stmt;
        json movies_with_showtimes = json::object();
        int rc;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "SQL PREPARE ERROR: " << sqlite3_errmsg(db) << std::endl;
            return crow::response(500, "Database query preparation failed");
        }

        sqlite3_bind_int(stmt, 1, std::stoi(venue_id_str));
        sqlite3_bind_text(stmt, 2, date_str, -1, SQLITE_STATIC);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int movie_id = sqlite3_column_int(stmt, 0);
            std::string movie_id_key = std::to_string(movie_id);

            if (movies_with_showtimes.find(movie_id_key) == movies_with_showtimes.end()) {
                movies_with_showtimes[movie_id_key]["movie_id"] = movie_id;
                movies_with_showtimes[movie_id_key]["movie_title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                movies_with_showtimes[movie_id_key]["poster_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                movies_with_showtimes[movie_id_key]["duration_minutes"] = sqlite3_column_int(stmt, 3);
                movies_with_showtimes[movie_id_key]["rating"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                movies_with_showtimes[movie_id_key]["synopsis"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                movies_with_showtimes[movie_id_key]["showtimes"] = json::array();
            }
            
            json showtime_obj;
            showtime_obj["time"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            showtime_obj["showtime_id"] = sqlite3_column_int(stmt, 7);
            showtime_obj["auditorium_id"] = sqlite3_column_int(stmt, 8);

            movies_with_showtimes[movie_id_key]["showtimes"].push_back(showtime_obj);
        }

        if (rc != SQLITE_DONE) {
            std::cerr << "SQL EXECUTION ERROR: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);

        json final_response = json::array();
        for (auto& el : movies_with_showtimes.items()) {
            final_response.push_back(el.value());
        }

        return crow::response(200, final_response.dump());
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
CROW_ROUTE(app, "/showtimes")
([](const crow::request& req){
    auto movie_id_str = req.url_params.get("movie_id");
    auto date_str = req.url_params.get("date");

    if (!movie_id_str) {
        return crow::response(400, "Missing movie_id parameter");
    }
    if(!date_str)
    {
        return crow::response(400, "Missing date parameter");
    }

    // This SQL query is now correct because V.Rating exists.
    std::string sql = "SELECT V.VenueID, V.Name, V.Rating, V.ImageURL, strftime('%H:%M', S.ShowtimeDateTime), S.ShowtimeID, S.AuditoriumID "
                      "FROM Showtimes AS S JOIN Venues AS V ON S.VenueID = V.VenueID "
                      "WHERE S.MovieID = ? AND S.ShowtimeDateTime LIKE ? || '%' "
                      "ORDER BY V.VenueID, S.ShowtimeDateTime";
    
    sqlite3_stmt* stmt;
    json venues_with_showtimes = json::object();
    int rc;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "SQL PREPARE ERROR: " << sqlite3_errmsg(db) << std::endl;
        return crow::response(500, "Database query preparation failed");
    }

    sqlite3_bind_int(stmt, 1, std::stoi(movie_id_str));
    sqlite3_bind_text(stmt, 2, date_str, -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int venue_id = sqlite3_column_int(stmt, 0);
        std::string venue_id_key = std::to_string(venue_id);

        if (venues_with_showtimes.find(venue_id_key) == venues_with_showtimes.end()) {
            venues_with_showtimes[venue_id_key]["venue_id"] = venue_id;
            venues_with_showtimes[venue_id_key]["venue_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            venues_with_showtimes[venue_id_key]["venue_rating"] = sqlite3_column_double(stmt, 2);
            venues_with_showtimes[venue_id_key]["venue_image_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            venues_with_showtimes[venue_id_key]["showtimes"] = json::array();
        }
        
        json showtime_obj;
        showtime_obj["time"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        showtime_obj["showtime_id"] = sqlite3_column_int(stmt, 5);
        showtime_obj["auditorium_id"] = sqlite3_column_int(stmt, 6);

        venues_with_showtimes[venue_id_key]["showtimes"].push_back(showtime_obj);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "SQL EXECUTION ERROR: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    json final_response = json::array();
    for (auto& el : venues_with_showtimes.items()) {
        final_response.push_back(el.value());
    }

    return crow::response(200, final_response.dump());
});
CROW_ROUTE(app, "/auditorium-details/<int>")
([](int auditoriumId){
    json audi_json;
    sqlite3_stmt* stmt;
    // This query now JOINS the two tables to get all info at once
    const char* sql = "SELECT A.NormalPrice, A.PremiumPrice, T.PremiumRows, T.NormalRows, T.Section1Seats, T.Section2Seats, T.Section3Seats "
                      "FROM Auditoriums AS A JOIN AuditoriumTemplates AS T ON A.TemplateID = T.TemplateID "
                      "WHERE A.AuditoriumID = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, auditoriumId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            audi_json["normal_price"] = sqlite3_column_double(stmt, 0);
            audi_json["premium_price"] = sqlite3_column_double(stmt, 1);
            
            // Nest the layout details in a "layout" object for cleaner JS access
            json layout;
            layout["premium_rows"] = sqlite3_column_int(stmt, 2);
            layout["normal_rows"] = sqlite3_column_int(stmt, 3);
            
            // Create a 'sections' array for easier looping in JavaScript
            json sections = json::array();
            sections.push_back(sqlite3_column_int(stmt, 4));
            if (sqlite3_column_int(stmt, 5) > 0) {
                sections.push_back(sqlite3_column_int(stmt, 5));
            }
            if (sqlite3_column_int(stmt, 6) > 0) {
                sections.push_back(sqlite3_column_int(stmt, 6));
            }
            layout["sections"] = sections;
            
            audi_json["layout"] = layout;
        }
    } else {
        std::cerr << "SQL PREPARE ERROR (Audi Details): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    if (audi_json.is_null()) return crow::response(404, "Auditorium not found");
    return crow::response(200, audi_json.dump());
});
CROW_ROUTE(app, "/occupied-seats")
    ([](const crow::request& req){
        auto showtime_id_str = req.url_params.get("showtime_id");
        if (!showtime_id_str) {
            return crow::response(400, "Missing showtime_id parameter");
        }

        json occupied_seats = json::array();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT SeatIdentifier FROM Bookings WHERE ShowtimeID = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, std::stoi(showtime_id_str));
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                occupied_seats.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            }
        }
        sqlite3_finalize(stmt);

        return crow::response(200, occupied_seats.dump());
    });

    // ... (end of the /book-tickets route)

// === NEW: /my-bookings ENDPOINT ===   
CROW_ROUTE(app, "/my-bookings/<int>")
([](int userId) {
    json response_json;
    response_json["upcoming"] = json::array();
    response_json["previous"] = json::array();

    // Get the current time in the same format as the database (YYYY-MM-DD HH:MM:SS)
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tt);
    char current_time_str[20];
    strftime(current_time_str, sizeof(current_time_str), "%Y-%m-%d %H:%M:%S", &tm);

    sqlite3_stmt* stmt;
    const char* sql = 
        "SELECT "
        "  B.ShowtimeID, "
        "  M.Title, "
        "  M.PosterURL, "
        "  V.Name, "
        "  S.ShowtimeDateTime, "
        "  COUNT(B.BookingID) AS SeatCount "
        "FROM Bookings AS B "
        "JOIN Showtimes AS S ON B.ShowtimeID = S.ShowtimeID "
        "JOIN Movies AS M ON S.MovieID = M.MovieID "
        "JOIN Venues AS V ON S.VenueID = V.VenueID "
        "WHERE B.UserID = ? "
        "GROUP BY B.ShowtimeID "
        "ORDER BY S.ShowtimeDateTime DESC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            json booking_item;
            booking_item["showtime_id"] = sqlite3_column_int(stmt, 0);
            booking_item["movie_title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            booking_item["poster_url"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            booking_item["venue_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            std::string showtime_datetime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            booking_item["show_date"] = showtime_datetime.substr(0, 10); // "YYYY-MM-DD"

            booking_item["seat_count"] = sqlite3_column_int(stmt, 5);

            // Sort into upcoming or previous
            if (showtime_datetime > current_time_str) {
                response_json["upcoming"].push_back(booking_item);
            } else {
                response_json["previous"].push_back(booking_item);
            }
        }
    } else {
        std::cerr << "SQL PREPARE ERROR (My Bookings): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    return crow::response(200, response_json.dump());
});

// --- Run the app ---
std::cout << "Server starting on port 18080..." << std::endl;
app.port(18080).multithreaded().bindaddr("0.0.0.0").run();

sqlite3_close(db);
return 0;
}
