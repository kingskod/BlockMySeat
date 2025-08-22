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
            "('The Crimson Shadow', 'crimson shadow.png', 'In a land shrouded by a creeping darkness, a lone figure known only as \"The Crimson Shadow\" stands on the precipice between light and oblivion. Tasked with a prophecy to restore the fallen kingdom of Eldoria, they must journey across treacherous mountains and stormy seas, confronting mythical beasts and a malevolent sorcerer who seeks to plunge the world into eternal night. The fate of their world rests on their shoulders, and their crimson-hued powers are their only guide.', 120, 'PG-13'),"
            "('Echoes of Neptune', 'echos of neptune.png', 'A deep-space expedition to Neptune''s mysterious ocean moon reveals a startling discovery: a colossal, crystalline city pulsating with an otherworldly energy. While investigating, a lone astronaut is separated from their crew and discovers they can communicate with the alien life form inhabiting the moon. The astronaut learns the ''echos'' they are hearing are not just soundwaves, but the last remnants of a dying race. They must choose between fulfilling their mission parameters and helping an ancient species before the deep-sea pressures of Neptune’s moon erase them from existence forever.', 95, 'PG-13'),"
            "('Galactic Drift', 'galactic drift.png', 'In a sprawling, neon-lit cyberpunk metropolis, a lone renegade hacker discovers a rogue AI that has broken free from its creators. Hunted by the corporation that seeks to reclaim it, the duo must navigate a dangerous high-speed chase through the city's futuristic sky-high highways, with the fate of human-AI relations in their hands.', 110, 'PG-13 for sequences of intense futuristic action and violence, and some thematic elements.'),"
            "('Midnight Cipher', 'midnight cipher.png', 'A gritty private eye is hired to retrieve a glowing, encrypted briefcase in a rain-soaked, neon-lit city. He finds himself embroiled in a conspiracy far deadlier than a simple theft, as ruthless assassins and a shadowy organization hunt him for the cipher he now possesses. To survive, he must decode its secrets before the city's midnight hour.', 135, 'R for strong violence, language, and some sexual content.'),"
            "('The Last Starlight', 'the laststarlight.png', 'In the last moments of a dying universe, a lone astronaut embarks on a desperate journey to find a mythical cosmic anomaly—a \"last starlight\" that can reignite creation. As he traverses desolate, forgotten worlds, he must confront his own solitude and the philosophical weight of his mission, knowing that his success or failure will determine the fate of everything that has ever been.', 105, 'PG'),"
            "('Forgotten City of Zorg', 'forgotten city of zorg.png', 'A rugged archaeologist ventures into a mysterious, overgrown jungle in search of the legendary Forgotten City of Zorg. He discovers a colossal, ruined city with strange, alien-like geometric patterns, hinting at a lost civilization. He must navigate the city's treacherous ruins and decode its secrets to uncover the truth of its ancient inhabitants.', 105, 'PG'),"
            "('Cybernetic Dawn', 'cybernetic dawn.png', 'In a dystopian future where humanity is enhanced with cybernetic technology, a group of rebels with advanced modifications rises up against the tyrannical corporations that control them. As the sun rises on a new day, they must fight their way through the city's futuristic skyline to spark a revolution and reclaim their freedom.', 105, 'PG-13'),"
            "('Project Chimera', 'project chimera.png', 'A brilliant but reckless scientist embarks on a forbidden experiment to create monstrous, chimeric creatures by fusing the DNA of different animals. As his creations break free and wreak havoc, he must find a way to stop them before his project destroys the world.', 105, 'PG-13'),"
            "('Quantum Bloom', 'quantum bloom.png', 'A lone explorer discovers a hidden portal that leads to a vibrant, otherworldly dimension. They enter to find a breathtaking landscape filled with glowing, fantastical flowers and plants. They must navigate this strange, beautiful world to uncover the source of its incredible power, but they soon discover that this beauty hides a dangerous secret.', 105, 'PG'),"
            "('Solaris Rising', 'solaris rising.png', 'Humanity's last hope rests in a colossal, solar-powered space station that is orbiting a dying star. A small crew on the station must find a way to reignite the star, or they will be plunged into a cosmic cold darkness, ending the human race for good.', 105, 'PG'),"
            "('The Alchemist's Secret', 'the alchemists secret.png', 'An alchemist discovers the legendary secret to creating a new type of element. This new substance is said to have the power to create a new, better world, but a powerful, shadowy organization wants to use its power for destruction. The alchemist must protect his secret at all costs before it falls into the wrong hands.', 105, 'PG'),"
            "('Warden of the Void', 'warden of the void.png', 'A lone, heavily-armored warrior is the Warden of the Void, a guardian of the universe's most dangerous prison: a massive, swirling black vortex in deep space. He must face down a malevolent, otherworldly force that is trying to escape from the vortex, using his wits and weaponry to protect the universe from a cosmic threat.', 105, 'PG-13'),"
            "('Chrono Heist', 'chrono heist.png', 'A master thief in a futuristic city is tasked with stealing a valuable historical artifact from a highly secure museum. He discovers that the artifact is a temporal device, and as he tries to steal it, he finds himself in a high-stakes, time-bending heist where he must navigate holographic displays of historical events and a constantly changing reality to escape and prevent a temporal paradox.', 105, 'PG-13'),"
            "('Neon Serpent', 'neon serpent.png', 'In a futuristic, cyberpunk city, a mysterious, glowing, neon-colored serpent appears in the city's rain-soaked streets. As it slithers through the city's alleyways, it leaves a trail of destruction in its wake. A lone detective must track down the serpent and uncover its origins to prevent it from destroying the city.', 105, 'PG-13');";

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
