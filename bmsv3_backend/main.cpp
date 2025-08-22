// These two lines are CRITICAL and must be FIRST.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Standard C++ and library headers go NEXT.
#include <iostream>
#include <string>
#include <random> // For generating session tokens
#include <sstream> // For generating session tokens
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
        "SessionToken TEXT);";

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
        "AuditoriumCount INTEGER,"
        "Rating REAL);";
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
        "Layout TEXT,"
        "NormalPrice REAL,"
        "PremiumPrice REAL,"
        "FOREIGN KEY(VenueID) REFERENCES Venues(VenueID));"; // <-- REMOVED Rating column
    if (sqlite3_exec(db, sql_create_auditoriums, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error (Auditoriums): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    const char* sql_create_bookings =
        "CREATE TABLE IF NOT EXISTS Bookings ("
        "BookingID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ShowtimeID INTEGER,"
        "UserID INTEGER," // <-- ADDED THIS COLUMN
        "SeatIdentifier TEXT NOT NULL,"
        "FOREIGN KEY(ShowtimeID) REFERENCES Showtimes(ShowtimeID),"
        "FOREIGN KEY(UserID) REFERENCES Users(UserID));";
    if (sqlite3_exec(db, sql_create_bookings, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error (Bookings): " << zErrMsg << std::endl;
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
    "('The Crimson Shadow', 'images/crimson shadow.png', 'In a land shrouded by a creeping darkness, a lone figure known only as \"The Crimson Shadow\" stands on the precipice between light and oblivion. Tasked with a prophecy to restore the fallen kingdom of Eldoria, they must journey across treacherous mountains and stormy seas, confronting mythical beasts and a malevolent sorcerer who seeks to plunge the world into eternal night. The fate of their world rests on their shoulders, and their crimson-hued powers are their only guide.', 120, 'PG-13 for intense sci-fi action, violence, and some thematic elements.'),"
    "('Echoes of Neptune', 'images/echos of neptune.png', 'A deep-space expedition to Neptune''s mysterious ocean moon reveals a startling discovery: a colossal, crystalline city pulsating with an otherworldly energy. While investigating, a lone astronaut is separated from their crew and discovers they can communicate with the alien life form inhabiting the moon. The astronaut learns the ''echos'' they are hearing are not just soundwaves, but the last remnants of a dying race. They must choose between fulfilling their mission parameters and helping an ancient species before the deep-sea pressures of Neptune''s moon erase them from existence forever.', 95, 'PG-13 for intense sci-fi peril and thematic elements.'),"
    "('Galactic Drift', 'images/galactic drift.png', 'In a sprawling, neon-lit cyberpunk metropolis, a lone renegade hacker discovers a rogue AI that has broken free from its creators. Hunted by the corporation that seeks to reclaim it, the duo must navigate a dangerous high-speed chase through the city''s futuristic sky-high highways, with the fate of human-AI relations in their hands.', 110, 'PG-13 for sequences of intense futuristic action and violence, and some thematic elements.'),"
    "('Midnight Cipher', 'images/midnight cipher.png', 'A gritty private eye is hired to retrieve a glowing, encrypted briefcase in a rain-soaked, neon-lit city. He finds himself embroiled in a conspiracy far deadlier than a simple theft, as ruthless assassins and a shadowy organization hunt him for the cipher he now possesses. To survive, he must decode its secrets before the city''s midnight hour.', 135, 'R for strong violence, language, and some sexual content.'),"
    "('The Last Starlight', 'images/the laststarlight.png', 'In the last moments of a dying universe, a lone astronaut embarks on a desperate journey to find a mythical cosmic anomaly—a \"last starlight\" that can reignite creation. As he traverses desolate, forgotten worlds, he must confront his own solitude and the philosophical weight of his mission, knowing that his success or failure will determine the fate of everything that has ever been.', 105, 'PG for peril and thematic elements.');";
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
        std::cout << "Venues table is empty. Seeding..." << std::endl;
        const char* seed_sql =
            "INSERT INTO Venues (Name, Location, ImageURL, AuditoriumCount, Rating) VALUES " // <-- ADDED Rating
            "('Blocky Multiplex', 'Downtown Cubeville', 'images/venue1.jpg', 12, 4.5),"      // <-- ADDED Rating value
            "('The Redstone Cinema', 'Oak Valley', 'images/venue2.jpg', 8, 5.0),"           // <-- ADDED Rating value
            "('Pixel Perfect Theaters', 'Glass Pane City', 'images/venue3.jpg', 16, 4.0);"; // <-- ADDED Rating value
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
            "INSERT INTO Auditoriums (VenueID, AuditoriumNumber, Layout, NormalPrice, PremiumPrice) VALUES "
            // Venue 1, Audi 1 (2 sections, 2 premium rows)
            "(1, 1, '{\"sections\":[10, 10], \"premium_rows\":2}', 10.50, 15.50),"
            // Venue 1, Audi 2 (3 sections, 1 premium row)
            "(1, 2, '{\"sections\":[8, 12, 8], \"premium_rows\":1}', 10.50, 15.50),"
            // Venue 2, Audi 1 (1 section, 1 premium row)
            "(2, 1, '{\"sections\":[20], \"premium_rows\":1}', 12.00, 18.00);";
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
CROW_ROUTE(app, "/showtimes")
([](const crow::request& req){
    auto movie_id_str = req.url_params.get("movie_id");
    auto date_str = req.url_params.get("date");

    if (!movie_id_str || !date_str) {
        return crow::response(400, "Missing movie_id or date parameter");
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
        const char* sql = "SELECT Layout, NormalPrice, PremiumPrice FROM Auditoriums WHERE AuditoriumID = ?";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, auditoriumId);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                audi_json["layout"] = json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                audi_json["normal_price"] = sqlite3_column_double(stmt, 1);
                audi_json["premium_price"] = sqlite3_column_double(stmt, 2);
            }
        }
        sqlite3_finalize(stmt);
        if (audi_json.is_null()) return crow::response(404, "Auditorium not found");
        return crow::response(200, audi_json.dump());
    });
    CROW_ROUTE(app, "/book-tickets").methods("POST"_method)
    ([](const crow::request& req){
        auto j = json::parse(req.body);
        int showtimeId = j["showtime_id"];
        int userId = j["user_id"];
        json seats = j["seats"]; // This is an array of strings

        const char* sql = "INSERT INTO Bookings (ShowtimeID, UserID, SeatIdentifier) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;

        for (const auto& seat : seats) {
            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            sqlite3_bind_int(stmt, 1, showtimeId);
            sqlite3_bind_int(stmt, 2, userId);
            sqlite3_bind_text(stmt, 3, seat.get<std::string>().c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "SQL error (Booking Insert): " << sqlite3_errmsg(db) << std::endl;
                sqlite3_finalize(stmt);
                return crow::response(500, "Failed to book one or more seats.");
            }
            sqlite3_finalize(stmt);
        }

        return crow::response(200, json{{"status", "success"}, {"message", "Booking confirmed!"}}.dump());
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

    // --- Run the app ---
    std::cout << "Server starting on port 18080..." << std::endl;
    app.port(18080).multithreaded().bindaddr("0.0.0.0").run();

    sqlite3_close(db);
    return 0;
}