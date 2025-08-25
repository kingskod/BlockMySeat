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
        "SeatIdentifier TEXT NOT NULL," // e.g., "A5", "C12"
        "FOREIGN KEY(ShowtimeID) REFERENCES Showtimes(ShowtimeID));";
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
            "('Neon Serpent', 'neon serpent.png', 'In a futuristic, cyberpunk city, a mysterious, glowing, neon-colored serpent appears in the city's rain-soaked streets. As it slithers through the city's alleyways, it leaves a trail of destruction in its wake. A lone detective must track down the serpent and uncover its origins to prevent it from destroying the city.', 105, 'PG-13'),"
            "('Oracle of the Dunes', 'oracle of the dunes.png', 'A lone wanderer braves a desolate, sand-swept world to find the legendary Oracle, said to reside within a colossal, ancient temple carved from the desert itself. As colossal sandstorms loom, they must uncover the secrets of the shifting landscape and face the trials of the desert to hear the Oracle's prophecy, which promises to change their world forever.', 105, 'PG'),"
            "('Titan's Fall', 'titans fall.png', 'In a world where colossal mechanical gods once roamed, an expedition team discovers a long-lost titan, now overgrown and dormant in a misty valley. As they explore its immense, silent form, they uncover the secrets of its catastrophic downfall, only to realize that their presence has awakened something far older and more dangerous than they could have imagined.', 105, 'PG-13'),"
            "('Aetherium Wars', 'aetherium wars.png', 'The sky-high conflict between two warring floating cities, one a kingdom of arcane magic and the other a bastion of industrial science, reaches a fever pitch. As their grand airships and fantastical weapons clash in the heavens, a hero from each side must race against time to expose a hidden conspiracy that threatens to bring both their civilizations crashing down.', 105, 'PG'),"
            "('Rogue Singularity', 'rogue singularity.png', 'When a maverick pilot on a reconnaissance mission stumbles upon a chaotic temporal anomaly, their advanced starship is drawn into a maelstrom of cosmic energy and shattered realities. They must fight against the impossible forces of the singularity to find a way back home, navigating a field of swirling debris and twisted spacetime.', 105, 'PG-13'),"
            "('Whispers of the Deep', 'whispers of the deep.png', 'A submarine crew on a deep-sea mining expedition makes a chilling discovery. When their sonar picks up an unknown signal, they descend into the ocean's darkest abyss, only to find themselves stalked by a monstrous, ancient creature from beyond the light. Trapped in the crushing darkness, they must find a way to escape before the terrifying whispers of the deep claim their sanity.', 105, 'PG-13'),"
            "('The Gilded Compass', 'the glided compass.png', 'In a world where clockwork marvels and steampunk contraptions power civilization, a skilled adventurer comes into possession of a mystical, gilded compass that points not to true north, but to the location of a legendary hidden city. Pursued by a ruthless corporate guild, they must decipher the compass's secrets and navigate a treacherous world of automatons, airships, and grand contraptions.', 105, 'PG'),"
            "('Zero Point Anomaly', 'zero point anomaly.png', 'In a high-security research facility, a team of brilliant but reckless scientists successfully creates a stable temporal anomaly. But their groundbreaking discovery quickly spirals out of control, revealing that the anomaly is not just a scientific breakthrough but a portal to another dimension. They must contain the anomaly and prevent its catastrophic effects from bleeding into their reality.', 105, 'PG-13'),"
            "('Siren's Lament', 'sirens lament.png', 'On a dark and stormy night, a grieving sailor is lured to a treacherous shipwreck by the haunting song of a spectral siren. As a furious storm rages, he must resist her enchanting call, for the whispers promise to reunite him with his lost love. He must face the truth of her beautiful lie and escape the Siren's embrace before he is lost to the sea forever.', 105, 'PG-13'),"
            "('Dragon's Gambit', 'dragons gambit.png', 'The land is at the mercy of a fearsome dragon, and the last remaining kingdom makes a desperate final move. The most skilled knight is sent on a perilous quest to challenge the dragon and its colossal might, standing on a misty mountaintop as the sun sets, ready for the final, legendary battle that will determine the fate of their world.', 105, 'PG'),"
            "('The Starforged Blade', 'the starforged blade.png', 'As a cataclysmic war rages in the cosmos, a lonely warrior discovers a legendary sword on a distant, icy mountaintop. Forged from the very stars themselves, the blade grants them immense power, and with it, they are humanity's last hope. They must wield the blade to face down an unstoppable enemy and bring an end to the celestial conflict.', 105, 'PG-13'),"
            "('Nomad of the Wastes', 'nomad of the wastes.png', 'In a sun-scorched, post-apocalyptic wasteland, a lone survivor travels across vast, sand-swept dunes with a battered vehicle. Hunted by a relentless storm of sand and scavengers, they must use their wits and combat skills to survive the unforgiving landscape, holding on to a secret that could change the fate of what's left of humanity.', 105, 'PG-13'),"
            "('Crimson Peak Legacy', 'crimson peak legacy.png', 'In a desolate, mist-shrouded mountain, a young woman inherits a crumbling, gothic mansion from a long-lost relative. As she explores its labyrinthine halls, she discovers that the mansion is home to more than just dust and cobwebs—it is haunted by a malevolent, crimson-colored entity that is connected to her family's past. She must unravel the chilling mystery before it's too late.', 105, 'PG-13'),"
            "('The Ghost Fleet', 'the ghost flet.png', 'A seasoned sailor on a quiet night at sea stumbles upon a terrifying sight: a fleet of ghostly, transparent ships emerges from a thick fog, their presence chilling the water to a dead calm. He must use his skills to outmaneuver the spectral armada and break free from their eerie supernatural pull before his own ship joins the ranks of the ghost fleet.', 105, 'PG'),"
            "('Ironclad Heart', 'ironclad heart.png', 'In a dystopian future ruled by corporate war, a battle-worn mech warrior is the last line of defense for a rebellion. Powered by a mysterious, glowing heart, the mech must fight its way through a ruined metropolis, confronting the city's ruthless robotic enforcers to deliver a message of hope to the surviving citizens.', 105, 'PG-13'),"
            "('Sands of Fury', 'sands of fury.png', 'In a world consumed by an endless desert, a lone warrior on a futuristic armored bike races against a colossal, ever-expanding sandstorm. Pursued by both the forces of nature and a gang of ruthless raiders, they must navigate the treacherous landscape and find a legendary oasis that holds the key to the survival of humanity.', 105, 'PG-13'),"
            "('The Celestial Map', 'the celestial map.png', 'An astronomer at a remote observatory discovers that the constellations are not just patterns of light but a cosmic map left behind by a celestial intelligence. As he deciphers its riddles, he realizes that the map is a guide to a new world. He must now protect the map from forces that want to exploit its power and embark on a journey of galactic proportions.', 105, 'PG'),"
            "('Vanguard's Oath', 'vanguards oath.png', 'In a world where magic and machinery have merged, a legendary armored warrior known as the Vanguard faces the ultimate test. They stand as the sole guardian of humanity, confronting a formidable, corrupted dragon-like creature that has laid waste to the land. After a final, devastating battle, the Vanguard must uphold their solemn oath to protect what remains and rebuild their world.', 105, 'PG-13'),"
            "('Shadow of the Colossus', 'shadow of the colossus.png', 'In a world of colossal, slumbering giants, a lone swordsman embarks on a forbidden journey to revive his lost love. His quest requires him to awaken and defeat the ancient stone colossi that roam the land, but as he fights each one, he discovers a horrifying truth about their connection to the land and his own destiny.', 105, 'PG-13'),"
            "('The Emerald Tablet', 'the emarald tablet.png', 'A resourceful explorer ventures deep into a lost temple, where legend says the secrets of alchemy are hidden. After navigating treacherous traps and puzzles, she discovers the fabled Emerald Tablet, which glows with an arcane energy. But a shadowy syndicate is close behind her, and she must use her wits to escape with the tablet, its secrets promising to change the course of science and magic.', 105, 'PG'),"
            "('Rebel of the Red Planet', 'rebel of the red planet.png', 'In a Martian colony controlled by a tyrannical corporation, a lone rebel soldier uncovers a conspiracy that threatens to enslave all of humanity. He must fight his way across the desolate red planet, battling ruthless corporate forces while seeking a way to transmit his discovery to Earth. He is a symbol of hope for a future free from oppression.', 105, 'PG-13'),"
            "('The Sunken Kingdom', 'the sunken kingdom.png', 'In a future where the oceans have reclaimed the land, a deep-sea explorer follows a cryptic signal to the bottom of the ocean. He discovers the mythical, technologically advanced kingdom of Atlantis, still pulsing with a faint light. He must navigate the city's treacherous ruins and decipher its secrets, discovering a dark truth about its fall and the forces that still haunt it.', 105, 'PG'),"
            "('Path of the Ronin', 'path of robin.png', 'A masterless samurai, disgraced by a past he cannot escape, wanders through a mystical land of spirits and treacherous forests. Haunted by his past failures, he seeks to atone for his sins, facing spiritual battles and internal demons along a misty path, with a single goal: to find a place of peace, or a worthy end.', 105, 'PG-13'),"
            "('The Clockwork Conspiracy', 'the clockwork conspiracy.png', 'In a sprawling, steampunk metropolis, a cynical private detective is hired to investigate a string of murders linked to an intricate clockwork device. The trail leads him into a hidden world of brilliant but dangerous inventors and a conspiracy to control the city's power with a sinister clockwork contraption. He must solve the mystery before the city's time runs out.', 105, 'PG-13'),"
            "('Legacy of the Void', 'legacy of the void.png', 'A lone pilot on a scavenging mission in deep space discovers a derelict, ancient space station. Upon boarding, he finds that the station holds the last remnants of a forgotten civilization and a powerful secret. As he uncovers the truth of their downfall, he realizes that the same fate awaits him if he can't escape the station and its haunting legacy.', 105, 'PG'),"
            "('The Obsidian Mirror', 'the obsidian mirror.png', 'An antiquarian acquires an ornate, ancient obsidian mirror with a dark reputation. When he looks into its depths, he discovers that the mirror is a gateway to a terrifying other-dimensional realm. As a malevolent entity from the other side tries to break through, he must find a way to destroy the mirror before its dark reflection consumes his world.', 105, 'PG-13'),"
            "('Whispering Woods', 'whispering woods.png', 'A young traveler, seeking a missing sibling, enters an enchanted forest where the trees themselves seem to watch and whisper. As she ventures deeper, she discovers the woods are home to ancient spirits, some benevolent and some sinister. She must navigate the forest's magical puzzles and face the truth of its hidden mysteries to find her missing sibling.', 105, 'PG'),"
            "('The Final Frontier', 'the final fromtier.png', 'After a devastating war, a lone, battle-worn pilot on a mission of exploration discovers a wormhole-like portal in the farthest reaches of space. He decides to enter the portal, embarking on a dangerous journey to an unknown destination, hoping to find a new world for humanity and leave behind the broken universe he knows.', 105, 'PG'),"
            "('Guardians of the Gate', 'guardians of the gate.png', 'A powerful, magical gate stands as the only barrier between two dimensions—one of light and one of darkness. Two elite guardians, bound by an ancient oath, must defend the gate against a malevolent force seeking to cross into their world. With their swords and magic, they are the last line of defense in a war between realms.', 105, 'PG-13'),"
            "('The Last Spell', 'the last spell.png', 'After a devastating battle, a lone sorcerer is all that stands between a magical kingdom and an invading army. With his allies defeated and his power nearly depleted, he must use the last of his strength to cast a final, forbidden spell that could either save his world or destroy it.', 105, 'PG-13'),"
            "('The Frozen Throne', 'the frozen throne.png', 'In a world covered in a perpetual blizzard, a young warrior embarks on a quest to defeat the tyrannical ruler who sits on the Frozen Throne. The warrior must brave the punishing snowstorms and treacherous icy lands to reach the throne room and challenge the figure who holds a dark secret about the world's endless winter.', 105, 'PG-13'),"
            "('The Serpent's Kiss', 'the serpants kiss.png', 'A young woman seeks a cure for her village's mysterious illness and is told of a mystical serpent in a forgotten swamp. She travels to the swamp and finds the serpent, which promises to heal her people in exchange for her soul. She must choose between the well-being of her village and her own life.', 105, 'PG-13');";

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
            "INSERT INTO Venues (Name, Location, ImageURL, AuditoriumCount, Rating) VALUES "
            "('Blocky Multiplex', 'Downtown Cubeville', 'images/blocky multiplex.png', 12, 4.5),"
            "('The Redstone Cinema', 'Oak Valley', 'images/the redstone cinema.png', 8, 5.0),"
            "('Pixel Perfect Theaters', 'Glass Pane City', 'images/pixel perfect.png', 16, 4.0),"
            "('The Redstone Reel', 'Block City', 'images/the redstone reel.png', 5, 4.6),"
            "('Creeper Cinemas', 'Creeperville', 'images/creeper cinemas.png', 7, 4.4),"
            "('The Ender Screen', 'Endertown', 'images/the ender screen.png', 6, 4.7),"
            "('NetherFlix Theatre', 'Nether District', 'images/netherflix.png', 8, 4.5),"
            "('Diamond Screenplex', 'Minecart Central', 'images/diamond screenplex.png', 10, 4.8),"
            "('Blockbuster Pavilion', 'Craftsville', 'images/blockbuster pavilion.png', 4, 4.3);";

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
            //Movie 3 at Venue 6
            "(3, 6, '2025-08-22 15:00:00'), (3, 6, '2025-08-22 18:30:00'), (3, 6, '2025-08-22 22:00:00'),"
            // Showtimes for tomorrow (e.g., 2025-08-23)
            "(1, 1, '2025-08-23 10:00:00'), (1, 3, '2025-08-23 15:00:00'), (3, 3, '2025-08-23 14:30:00'), (1, 4, '2025-08-23 17:00:00'),"
            "(2, 5, '2025-08-23 19:30:00'), (3, 6, '2025-08-23 21:45:00');";

        
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