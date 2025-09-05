import random
from datetime import datetime, time, timedelta

# --- 1. INPUT DATA (CORRECTED STRUCTURE) ---
# The AUDITORIUMS list is now nested inside each VENUE.
# The 'id' is the globally unique AuditoriumID from your database.
# The 'number' is the screen number for that venue (e.g., "Screen 1").

MOVIES = [
    {"id": 1, "title": "The Crimson Shadow", "duration": 120},
    {"id": 2, "title": "Echoes of Neptune", "duration": 95},
    {"id": 3, "title": "Galactic Drift", "duration": 110},
    {"id": 4, "title": "Midnight Cipher", "duration": 135},
    {"id": 5, "title": "The Last Starlight", "duration": 100},
    {"id": 6, "title": "Forgotten City of Zorg", "duration": 115},
    {"id": 7, "title": "Cybernetic Dawn", "duration": 125},
    {"id": 8, "title": "Project Chimera", "duration": 90},
    {"id": 9, "title": "Quantum Bloom", "duration": 140},
    {"id": 10, "title": "Solaris Rising", "duration": 105},
    {"id": 11, "title": "The Alchemist's Secret", "duration": 130},
    {"id": 12, "title": "Warden of the Void", "duration": 95},
    {"id": 13, "title": "Chrono Heist", "duration": 115},
    {"id": 14, "title": "Neon Serpent", "duration": 135},
    {"id": 15, "title": "Oracle of the Dunes", "duration": 100},
    {"id": 16, "title": "Titan's Fall", "duration": 120},
    {"id": 17, "title": "Aetherium Wars", "duration": 125},
    {"id": 18, "title": "Rogue Singularity", "duration": 90},
    {"id": 19, "title": "Whispers of the Deep", "duration": 140},
    {"id": 20, "title": "The Gilded Compass", "duration": 110},
    {"id": 21, "title": "Zero Point Anomaly", "duration": 95},
    {"id": 22, "title": "Siren's Lament", "duration": 130},
    {"id": 23, "title": "Dragon's Gambit", "duration": 100},
    {"id": 24, "title": "The Starforged Blade", "duration": 125},
    {"id": 25, "title": "Nomad of the Wastes", "duration": 115},
    {"id": 26, "title": "Crimson Peak Legacy", "duration": 105},
    {"id": 27, "title": "The Ghost Fleet", "duration": 135},
    {"id": 28, "title": "Ironclad Heart", "duration": 120},
    {"id": 29, "title": "Sands of Fury", "duration": 90},
    {"id": 30, "title": "The Celestial Map", "duration": 140},
    {"id": 31, "title": "Vanguard's Oath", "duration": 95},
    {"id": 32, "title": "Shadow of the Colossus", "duration": 110},
    {"id": 33, "title": "The Emerald Tablet", "duration": 125},
    {"id": 34, "title": "Rebel of the Red Planet", "duration": 100},
    {"id": 35, "title": "The Sunken Kingdom", "duration": 130},
    {"id": 36, "title": "Path of the Ronin", "duration": 115},
    {"id": 37, "title": "The Clockwork Conspiracy", "duration": 135},
    {"id": 38, "title": "Legacy of the Void", "duration": 105},
    {"id": 39, "title": "The Obsidian Mirror", "duration": 120},
    {"id": 40, "title": "Whispering Woods", "duration": 95},
    {"id": 41, "title": "The Final Frontier", "duration": 140},
    {"id": 42, "title": "Guardians of the Gate", "duration": 100},
    {"id": 43, "title": "The Last Spell", "duration": 110},
    {"id": 44, "title": "The Frozen Throne", "duration": 115},
    {"id": 45, "title": "The Serpent's Kiss", "duration": 125},
]


VENUES = [
    {
        "id": 1,
        "name": "Blocky Multiplex",
        "auditoriums": [
            {"id": 1, "number": 1},
            {"id": 2, "number": 2},
            {"id": 3, "number": 3},
            {"id": 4, "number": 4},
            {"id": 5, "number": 5},
            {"id": 6, "number": 6},
            {"id": 7, "number": 7},
            {"id": 8, "number": 8},
            {"id": 9, "number": 9},
            {"id": 10, "number": 10},
            {"id": 11, "number": 11},
            {"id": 12, "number": 12},
        ]
    },
    {
        "id": 2,
        "name": "The Redstone Cinema",
        "auditoriums": [
            {"id": 13, "number": 1},
            {"id": 14, "number": 2},
            {"id": 15, "number": 3},
            {"id": 16, "number": 4},
            {"id": 17, "number": 5},
            {"id": 18, "number": 6},
            {"id": 19, "number": 7},
            {"id": 20, "number": 8},
        ]
    },
    {
        "id": 3,
        "name": "Pixel Perfect Theaters",
        "auditoriums": [
            {"id": 21, "number": 1},
            {"id": 22, "number": 2},
            {"id": 23, "number": 3},
            {"id": 24, "number": 4},
            {"id": 25, "number": 5},
            {"id": 26, "number": 6},
            {"id": 27, "number": 7},
            {"id": 28, "number": 8},
            {"id": 29, "number": 9},
            {"id": 30, "number": 10},
            {"id": 31, "number": 11},
            {"id": 32, "number": 12},
            {"id": 33, "number": 13},
            {"id": 34, "number": 14},
            {"id": 35, "number": 15},
            {"id": 36, "number": 16},
        ]
    },
    {
        "id": 4,
        "name": "The Redstone Reel",
        "auditoriums": [
            {"id": 37, "number": 1},
            {"id": 38, "number": 2},
            {"id": 39, "number": 3},
            {"id": 40, "number": 4},
            {"id": 41, "number": 5},
        ]
    },
    {
        "id": 5,
        "name": "Creeper Cinemas",
        "auditoriums": [
            {"id": 42, "number": 1},
            {"id": 43, "number": 2},
            {"id": 44, "number": 3},
            {"id": 45, "number": 4},
            {"id": 46, "number": 5},
            {"id": 47, "number": 6},
            {"id": 48, "number": 7},
        ]
    },
    {
        "id": 6,
        "name": "The Ender Screen",
        "auditoriums": [
            {"id": 49, "number": 1},
            {"id": 50, "number": 2},
            {"id": 51, "number": 3},
            {"id": 52, "number": 4},
            {"id": 53, "number": 5},
            {"id": 54, "number": 6},
        ]
    },
    {
        "id": 7,
        "name": "NetherFlix Theatre",
        "auditoriums": [
            {"id": 55, "number": 1},
            {"id": 56, "number": 2},
            {"id": 57, "number": 3},
            {"id": 58, "number": 4},
            {"id": 59, "number": 5},
            {"id": 60, "number": 6},
            {"id": 61, "number": 7},
            {"id": 62, "number": 8},
        ]
    },
    {
        "id": 8,
        "name": "Diamond Screenplex",
        "auditoriums": [
            {"id": 63, "number": 1},
            {"id": 64, "number": 2},
            {"id": 65, "number": 3},
            {"id": 66, "number": 4},
            {"id": 67, "number": 5},
            {"id": 68, "number": 6},
            {"id": 69, "number": 7},
            {"id": 70, "number": 8},
            {"id": 71, "number": 9},
            {"id": 72, "number": 10},
        ]
    },
    {
        "id": 9,
        "name": "Blockbuster Pavilion",
        "auditoriums": [
            {"id": 73, "number": 1},
            {"id": 74, "number": 2},
            {"id": 75, "number": 3},
            {"id": 76, "number": 4},
        ]
    }
]


# --- 2. SCHEDULING CONFIGURATION (Unchanged) ---
DAYS_TO_SCHEDULE = 7
OPENING_TIME = time(10, 0)
LAST_SHOW_START = time(22, 30)
CLEANING_BUFFER_MINUTES = 20
SHOWINGS_PER_MOVIE = 25


def generate_realistic_schedule():
    """
    Generates a schedule using the corrected, venue-specific auditorium logic.
    """
    print("Generating realistic showtime schedule...")
    
    auditorium_timelines = {}
    screenings_to_schedule = []
    for movie in MOVIES:
        for _ in range(SHOWINGS_PER_MOVIE):
            screenings_to_schedule.append(movie)
    
    random.shuffle(screenings_to_schedule)

    final_schedule = []
    failed_attempts = 0

    while screenings_to_schedule and failed_attempts < len(screenings_to_schedule):
        movie = screenings_to_schedule.pop(0)

        scheduled = False
        for _ in range(20): # Try 20 random slots
            day = random.randint(0, DAYS_TO_SCHEDULE - 1)
            
            venue = random.choice(VENUES)

            auditorium = random.choice(venue["auditoriums"])
            
            start_hour = random.randint(OPENING_TIME.hour, LAST_SHOW_START.hour)
            start_minute = random.choice([0, 15, 30, 45])
            start_time = time(start_hour, start_minute)
            
            start_datetime = datetime.combine(datetime.today(), start_time)
            end_datetime = start_datetime + timedelta(minutes=movie["duration"] + CLEANING_BUFFER_MINUTES)
            
            timeline_key = (day, auditorium["id"])
            if timeline_key not in auditorium_timelines:
                auditorium_timelines[timeline_key] = []

            is_collision = False
            for busy_start, busy_end in auditorium_timelines[timeline_key]:
                if max(start_datetime, busy_start) < min(end_datetime, busy_end):
                    is_collision = True
                    break
            
            if not is_collision:
                auditorium_timelines[timeline_key].append((start_datetime, end_datetime))
                final_schedule.append({
                    "movie_id": movie["id"],
                    "venue_id": venue["id"], # Use the venue's ID
                    "auditorium_id": auditorium["id"], # Use the auditorium's unique ID
                    "day_offset": day,
                    "time": start_time.strftime("%H:%M:%S")
                })
                scheduled = True
                failed_attempts = 0
                break
        
        if not scheduled:
            screenings_to_schedule.append(movie)
            failed_attempts += 1

    print(f"Successfully generated {len(final_schedule)} showtimes.")
    return sorted(final_schedule, key=lambda x: (x['day_offset'], x['venue_id'], x['auditorium_id'], x['time']))


def format_cpp_vector_output(schedule):
    """
    Takes the generated schedule and formats it into a C++ vector definition.
    """
    print("\n--- Copy the text below and paste it into your seed_data.hpp file ---\n")
    
    cpp_structs = []
    for screening in schedule:
        cpp_structs.append(
            f"    {{{screening['movie_id']}, {screening['venue_id']}, {screening['auditorium_id']}, {screening['day_offset']}, \"{screening['time']}\"}}"
        )
    
    vector_string = ",\n".join(cpp_structs)

    final_output = f"""static const std::vector<ShowtimeSeed> showtime_seeds = {{
{vector_string}
}};"""

    print(final_output)
    print("\n--- End of C++ data ---")


# --- MAIN EXECUTION ---
if __name__ == "__main__":
    generated_schedule = generate_realistic_schedule()

    # Save the formatted schedule to a text file
    with open("schedule.txt", "w") as file:
        cpp_structs = []
        for screening in generated_schedule:
            cpp_structs.append(
                f"    {{{screening['movie_id']}, {screening['venue_id']}, {screening['auditorium_id']}, {screening['day_offset']}, \"{screening['time']}\"}}"
            )

        vector_string = ",\n".join(cpp_structs)

        final_output = f"""static const std::vector<ShowtimeSeed> showtime_seeds = {{
{vector_string}
}};"""

        file.write(final_output)

    print("Schedule saved to schedule.txt")