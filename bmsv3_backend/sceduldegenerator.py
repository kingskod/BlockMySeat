import random
import re
from datetime import datetime, time, timedelta
from faker import Faker

# --- 1. CONFIGURATION ---
CONFIG = {
    "NUM_USERS": 1500,
    "AVG_BOOKINGS_PER_USER": 4,
    "AVG_SEATS_PER_BOOKING": 4,
    "DAYS_TO_SCHEDULE": 7,
    "SHOWINGS_PER_MOVIE_TARGET": 30,
    "OPENING_TIME": time(10, 0),
    "LAST_SHOW_START": time(22, 30),
    "CLEANING_BUFFER_MINUTES": 20,
}

# --- 2. INPUT DATA ---
# PASTE YOUR MOVIE AND VENUE INSERT STATEMENTS HERE

MOVIES_SQL = """"
"INSERT INTO Movies (Title, PosterURL, Synopsis, DurationMinutes, Rating) VALUES "
            "('The Crimson Shadow', 'images/crimson shadow.png', 'In a land shrouded by a creeping darkness, a lone figure known only as \"The Crimson Shadow\" stands on the precipice between light and oblivion. Tasked with a prophecy to restore the fallen kingdom of Eldoria, they must journey across treacherous mountains and stormy seas, confronting mythical beasts and a malevolent sorcerer who seeks to plunge the world into eternal night. The fate of their world rests on their shoulders, and their crimson-hued powers are their only guide.', 120, 'PG-13'),"
            "('Echoes of Neptune', 'images/echos of neptune.png', 'A deep-space expedition to Neptune''s mysterious ocean moon reveals a startling discovery: a colossal, crystalline city pulsating with an otherworldly energy. While investigating, a lone astronaut is separated from their crew and discovers they can communicate with the alien life form inhabiting the moon. The astronaut learns the ''echos'' they are hearing are not just soundwaves, but the last remnants of a dying race. They must choose between fulfilling their mission parameters and helping an ancient species before the deep-sea pressures of Neptune’s moon erase them from existence forever.', 95, 'PG-13'),"
            "('Galactic Drift', 'images/galactic drift.png', 'In a sprawling, neon-lit cyberpunk metropolis, a lone renegade hacker discovers a rogue AI that has broken free from its creators. Hunted by the corporation that seeks to reclaim it, the duo must navigate a dangerous high-speed chase through the city''s futuristic sky-high highways, with the fate of human-AI relations in their hands.', 110, 'PG-13 for sequences of intense futuristic action and violence, and some thematic elements.'),"
            "('Midnight Cipher', 'images/midnight cipher.png', 'A gritty private eye is hired to retrieve a glowing, encrypted briefcase in a rain-soaked, neon-lit city. He finds himself embroiled in a conspiracy far deadlier than a simple theft, as ruthless assassins and a shadowy organization hunt him for the cipher he now possesses. To survive, he must decode its secrets before the city''s midnight hour.', 135, 'R for strong violence, language, and some sexual content.'),"
            "('The Last Starlight', 'images/the laststarlight.png', 'In the last moments of a dying universe, a lone astronaut embarks on a desperate journey to find a mythical cosmic anomaly—a \"last starlight\" that can reignite creation. As he traverses desolate, forgotten worlds, he must confront his own solitude and the philosophical weight of his mission, knowing that his success or failure will determine the fate of everything that has ever been.', 105, 'PG'),"
            "('Forgotten City of Zorg', 'images/forgotten city of zorg.png', 'A rugged archaeologist ventures into a mysterious, overgrown jungle in search of the legendary Forgotten City of Zorg. He discovers a colossal, ruined city with strange, alien-like geometric patterns, hinting at a lost civilization. He must navigate the city''s treacherous ruins and decode its secrets to uncover the truth of its ancient inhabitants.', 105, 'PG'),"
            "('Cybernetic Dawn', 'images/cybernetic dawn.png', 'In a dystopian future where humanity is enhanced with cybernetic technology, a group of rebels with advanced modifications rises up against the tyrannical corporations that control them. As the sun rises on a new day, they must fight their way through the city''s futuristic skyline to spark a revolution and reclaim their freedom.', 105, 'PG-13'),"
            "('Project Chimera', 'images/project chimera.png', 'A brilliant but reckless scientist embarks on a forbidden experiment to create monstrous, chimeric creatures by fusing the DNA of different animals. As his creations break free and wreak havoc, he must find a way to stop them before his project destroys the world.', 105, 'PG-13'),"
            "('Quantum Bloom', 'images/quantum bloom.png', 'A lone explorer discovers a hidden portal that leads to a vibrant, otherworldly dimension. They enter to find a breathtaking landscape filled with glowing, fantastical flowers and plants. They must navigate this strange, beautiful world to uncover the source of its incredible power, but they soon discover that this beauty hides a dangerous secret.', 105, 'PG'),"
            "('Solaris Rising', 'images/solaris rising.png', 'Humanity''s last hope rests in a colossal, solar-powered space station that is orbiting a dying star. A small crew on the station must find a way to reignite the star, or they will be plunged into a cosmic cold darkness, ending the human race for good.', 105, 'PG'),"
            "('The Alchemist''s Secret', 'images/the alchemists secret.png', 'An alchemist discovers the legendary secret to creating a new type of element. This new substance is said to have the power to create a new, better world, but a powerful, shadowy organization wants to use its power for destruction. The alchemist must protect his secret at all costs before it falls into the wrong hands.', 105, 'PG'),"
            "('Warden of the Void', 'images/warden of the void.png', 'A lone, heavily-armored warrior is the Warden of the Void, a guardian of the universe''s most dangerous prison: a massive, swirling black vortex in deep space. He must face down a malevolent, otherworldly force that is trying to escape from the vortex, using his wits and weaponry to protect the universe from a cosmic threat.', 105, 'PG-13'),"
            "('Chrono Heist', 'images/chrono heist.png', 'A master thief in a futuristic city is tasked with stealing a valuable historical artifact from a highly secure museum. He discovers that the artifact is a temporal device, and as he tries to steal it, he finds himself in a high-stakes, time-bending heist where he must navigate holographic displays of historical events and a constantly changing reality to escape and prevent a temporal paradox.', 105, 'PG-13'),"
            "('Neon Serpent', 'images/neon serpent.png', 'In a futuristic, cyberpunk city, a mysterious, glowing, neon-colored serpent appears in the city''s rain-soaked streets. As it slithers through the city''s alleyways, it leaves a trail of destruction in its wake. A lone detective must track down the serpent and uncover its origins to prevent it from destroying the city.', 105, 'PG-13'),"
            "('Oracle of the Dunes', 'images/oracle of the dunes.png', 'A lone wanderer braves a desolate, sand-swept world to find the legendary Oracle, said to reside within a colossal, ancient temple carved from the desert itself. As colossal sandstorms loom, they must uncover the secrets of the shifting landscape and face the trials of the desert to hear the Oracle''s prophecy, which promises to change their world forever.', 105, 'PG'),"
            "('Titan''s Fall', 'images/titans fall.png', 'In a world where colossal mechanical gods once roamed, an expedition team discovers a long-lost titan, now overgrown and dormant in a misty valley. As they explore its immense, silent form, they uncover the secrets of its catastrophic downfall, only to realize that their presence has awakened something far older and more dangerous than they could have imagined.', 105, 'PG-13'),"
            "('Aetherium Wars', 'images/aetherium wars.png', 'The sky-high conflict between two warring floating cities, one a kingdom of arcane magic and the other a bastion of industrial science, reaches a fever pitch. As their grand airships and fantastical weapons clash in the heavens, a hero from each side must race against time to expose a hidden conspiracy that threatens to bring both their civilizations crashing down.', 105, 'PG'),"
            "('Rogue Singularity', 'images/rogue singularity.png', 'When a maverick pilot on a reconnaissance mission stumbles upon a chaotic temporal anomaly, their advanced starship is drawn into a maelstrom of cosmic energy and shattered realities. They must fight against the impossible forces of the singularity to find a way back home, navigating a field of swirling debris and twisted spacetime.', 105, 'PG-13'),"
            "('Whispers of the Deep', 'images/whispers of the deep.png', 'A submarine crew on a deep-sea mining expedition makes a chilling discovery. When their sonar picks up an unknown signal, they descend into the ocean''s darkest abyss, only to find themselves stalked by a monstrous, ancient creature from beyond the light. Trapped in the crushing darkness, they must find a way to escape before the terrifying whispers of the deep claim their sanity.', 105, 'PG-13'),"
            "('The Gilded Compass', 'images/the glided compass.png', 'In a world where clockwork marvels and steampunk contraptions power civilization, a skilled adventurer comes into possession of a mystical, gilded compass that points not to true north, but to the location of a legendary hidden city. Pursued by a ruthless corporate guild, they must decipher the compass''s secrets and navigate a treacherous world of automatons, airships, and grand contraptions.', 105, 'PG'),"
            "('Zero Point Anomaly', 'images/zero point anomaly.png', 'In a high-security research facility, a team of brilliant but reckless scientists successfully creates a stable temporal anomaly. But their groundbreaking discovery quickly spirals out of control, revealing that the anomaly is not just a scientific breakthrough but a portal to another dimension. They must contain the anomaly and prevent its catastrophic effects from bleeding into their reality.', 105, 'PG-13'),"
            "('Siren''s Lament', 'images/sirens lament.png', 'On a dark and stormy night, a grieving sailor is lured to a treacherous shipwreck by the haunting song of a spectral siren. As a furious storm rages, he must resist her enchanting call, for the whispers promise to reunite him with his lost love. He must face the truth of her beautiful lie and escape the Siren''s embrace before he is lost to the sea forever.', 105, 'PG-13'),"
            "('Dragon''s Gambit', 'images/dragons gambit.png', 'The land is at the mercy of a fearsome dragon, and the last remaining kingdom makes a desperate final move. The most skilled knight is sent on a perilous quest to challenge the dragon and its colossal might, standing on a misty mountaintop as the sun sets, ready for the final, legendary battle that will determine the fate of their world.', 105, 'PG'),"
            "('The Starforged Blade', 'images/the starforged blade.png', 'As a cataclysmic war rages in the cosmos, a lonely warrior discovers a legendary sword on a distant, icy mountaintop. Forged from the very stars themselves, the blade grants them immense power, and with it, they are humanity''s last hope. They must wield the blade to face down an unstoppable enemy and bring an end to the celestial conflict.', 105, 'PG-13'),"
            "('Nomad of the Wastes', 'images/nomad of the wastes.png', 'In a sun-scorched, post-apocalyptic wasteland, a lone survivor travels across vast, sand-swept dunes with a battered vehicle. Hunted by a relentless storm of sand and scavengers, they must use their wits and combat skills to survive the unforgiving landscape, holding on to a secret that could change the fate of what''s left of humanity.', 105, 'PG-13'),"
            "('Crimson Peak Legacy', 'images/crimson peak legacy.png', 'In a desolate, mist-shrouded mountain, a young woman inherits a crumbling, gothic mansion from a long-lost relative. As she explores its labyrinthine halls, she discovers that the mansion is home to more than just dust and cobwebs—it is haunted by a malevolent, crimson-colored entity that is connected to her family''s past. She must unravel the chilling mystery before it''s too late.', 105, 'PG-13'),"
            "('The Ghost Fleet', 'images/the ghost flet.png', 'A seasoned sailor on a quiet night at sea stumbles upon a terrifying sight: a fleet of ghostly, transparent ships emerges from a thick fog, their presence chilling the water to a dead calm. He must use his skills to outmaneuver the spectral armada and break free from their eerie supernatural pull before his own ship joins the ranks of the ghost fleet.', 105, 'PG'),"
            "('Ironclad Heart', 'images/ironclad heart.png', 'In a dystopian future ruled by corporate war, a battle-worn mech warrior is the last line of defense for a rebellion. Powered by a mysterious, glowing heart, the mech must fight its way through a ruined metropolis, confronting the city''s ruthless robotic enforcers to deliver a message of hope to the surviving citizens.', 105, 'PG-13'),"
            "('Sands of Fury', 'images/sands of fury.png', 'In a world consumed by an endless desert, a lone warrior on a futuristic armored bike races against a colossal, ever-expanding sandstorm. Pursued by both the forces of nature and a gang of ruthless raiders, they must navigate the treacherous landscape and find a legendary oasis that holds the key to the survival of humanity.', 105, 'PG-13'),"
            "('The Celestial Map', 'images/the celestial map.png', 'An astronomer at a remote observatory discovers that the constellations are not just patterns of light but a cosmic map left behind by a celestial intelligence. As he deciphers its riddles, he realizes that the map is a guide to a new world. He must now protect the map from forces that want to exploit its power and embark on a journey of galactic proportions.', 105, 'PG'),"
            "('Vanguard''s Oath', 'images/vanguards oath.png', 'In a world where magic and machinery have merged, a legendary armored warrior known as the Vanguard faces the ultimate test. They stand as the sole guardian of humanity, confronting a formidable, corrupted dragon-like creature that has laid waste to the land. After a final, devastating battle, the Vanguard must uphold their solemn oath to protect what remains and rebuild their world.', 105, 'PG-13'),"
            "('Shadow of the Colossus', 'images/shadow of the colossus.png', 'In a world of colossal, slumbering giants, a lone swordsman embarks on a forbidden journey to revive his lost love. His quest requires him to awaken and defeat the ancient stone colossi that roam the land, but as he fights each one, he discovers a horrifying truth about their connection to the land and his own destiny.', 105, 'PG-13'),"
            "('The Emerald Tablet', 'images/the emarald tablet.png', 'A resourceful explorer ventures deep into a lost temple, where legend says the secrets of alchemy are hidden. After navigating treacherous traps and puzzles, she discovers the fabled Emerald Tablet, which glows with an arcane energy. But a shadowy syndicate is close behind her, and she must use her wits to escape with the tablet, its secrets promising to change the course of science and magic.', 105, 'PG'),"
            "('Rebel of the Red Planet', 'images/rebel of the red planet.png', 'In a Martian colony controlled by a tyrannical corporation, a lone rebel soldier uncovers a conspiracy that threatens to enslave all of humanity. He must fight his way across the desolate red planet, battling ruthless corporate forces while seeking a way to transmit his discovery to Earth. He is a symbol of hope for a future free from oppression.', 105, 'PG-13'),"
            "('The Sunken Kingdom', 'images/the sunken kingdom.png', 'In a future where the oceans have reclaimed the land, a deep-sea explorer follows a cryptic signal to the bottom of the ocean. He discovers the mythical, technologically advanced kingdom of Atlantis, still pulsing with a faint light. He must navigate the city''s treacherous ruins and decipher its secrets, discovering a dark truth about its fall and the forces that still haunt it.', 105, 'PG'),"
            "('Path of the Ronin', 'images/path of robin.png', 'A masterless samurai, disgraced by a past he cannot escape, wanders through a mystical land of spirits and treacherous forests. Haunted by his past failures, he seeks to atone for his sins, facing spiritual battles and internal demons along a misty path, with a single goal: to find a place of peace, or a worthy end.', 105, 'PG-13'),"
            "('The Clockwork Conspiracy', 'images/the clockwork conspiracy.png', 'In a sprawling, steampunk metropolis, a cynical private detective is hired to investigate a string of murders linked to an intricate clockwork device. The trail leads him into a hidden world of brilliant but dangerous inventors and a conspiracy to control the city''s power with a sinister clockwork contraption. He must solve the mystery before the city''s time runs out.', 105, 'PG-13'),"
            "('Legacy of the Void', 'images/legacy of the void.png', 'A lone pilot on a scavenging mission in deep space discovers a derelict, ancient space station. Upon boarding, he finds that the station holds the last remnants of a forgotten civilization and a powerful secret. As he uncovers the truth of their downfall, he realizes that the same fate awaits him if he can''t escape the station and its haunting legacy.', 105, 'PG'),"
            "('The Obsidian Mirror', 'images/the obsidian mirror.png', 'An antiquarian acquires an ornate, ancient obsidian mirror with a dark reputation. When he looks into its depths, he discovers that the mirror is a gateway to a terrifying other-dimensional realm. As a malevolent entity from the other side tries to break through, he must find a way to destroy the mirror before its dark reflection consumes his world.', 105, 'PG-13'),"
            "('Whispering Woods', 'images/whispering woods.png', 'A young traveler, seeking a missing sibling, enters an enchanted forest where the trees themselves seem to watch and whisper. As she ventures deeper, she discovers the woods are home to ancient spirits, some benevolent and some sinister. She must navigate the forest''s magical puzzles and face the truth of its hidden mysteries to find her missing sibling.', 105, 'PG'),"
            "('The Final Frontier', 'images/the final fromtier.png', 'After a devastating war, a lone, battle-worn pilot on a mission of exploration discovers a wormhole-like portal in the farthest reaches of space. He decides to enter the portal, embarking on a dangerous journey to an unknown destination, hoping to find a new world for humanity and leave behind the broken universe he knows.', 105, 'PG'),"
            "('Guardians of the Gate', 'images/guardians of the gate.png', 'A powerful, magical gate stands as the only barrier between two dimensions—one of light and one of darkness. Two elite guardians, bound by an ancient oath, must defend the gate against a malevolent force seeking to cross into their world. With their swords and magic, they are the last line of defense in a war between realms.', 105, 'PG-13'),"
            "('The Last Spell', 'images/the last spell.png', 'After a devastating battle, a lone sorcerer is all that stands between a magical kingdom and an invading army. With his allies defeated and his power nearly depleted, he must use the last of his strength to cast a final, forbidden spell that could either save his world or destroy it.', 105, 'PG-13'),"
            "('The Frozen Throne', 'images/the frozen throne.png', 'In a world covered in a perpetual blizzard, a young warrior embarks on a quest to defeat the tyrannical ruler who sits on the Frozen Throne. The warrior must brave the punishing snowstorms and treacherous icy lands to reach the throne room and challenge the figure who holds a dark secret about the world''s endless winter.', 105, 'PG-13'),"
            "('The Serpent''s Kiss', 'images/the serpants kiss.png', 'A young woman seeks a cure for her village''s mysterious illness and is told of a mystical serpent in a forgotten swamp. She travels to the swamp and finds the serpent, which promises to heal her people in exchange for her soul. She must choose between the well-being of her village and her own life.', 105, 'PG-13');";"""

VENUES_SQL = """
"INSERT INTO Venues  VALUES "
            "('Blocky Multiplex', 'Downtown Cubeville', 'venues/blocky multiplex.png', 12, 4.5),"
            "('The Redstone Cinema', 'Oak Valley', 'venues/the redstone cinema.png', 8, 5.0),"
            "('Pixel Perfect Theaters', 'Glass Pane City', 'venues/pixel perfect.png', 16, 4.0),"
            "('The Redstone Reel', 'Block City', 'venues/the redstone reel.png', 5, 4.6),"
            "('Creeper Cinemas', 'Creeperville', 'venues/creeper cinemas.png', 7, 4.4),"
            "('The Ender Screen', 'Endertown', 'venues/the ender screen.png', 6, 4.7),"
            "('NetherFlix Theatre', 'Nether District', 'venues/netherflix.png', 8, 4.5),"
            "('Diamond Screenplex', 'Minecart Central', 'venues/diamond screenplex.png', 10, 4.8),"
            "('Blockbuster Pavilion', 'Craftsville', 'venues/blockbuster pavilion.png', 4, 4.3);";
"""

# --- UTILITY FUNCTIONS ---
def parse_sql_insert(sql_string):
    # This now correctly skips the header line
    try:
        matches = re.findall(r"\(([^)]+)\)", sql_string[sql_string.find('('):])
        records = []
        for match in matches:
            values = re.split(r",(?=(?:[^']*'[^']*')*[^']*$)", match)
            cleaned_values = [val.strip().strip("'") for val in values]
            records.append(cleaned_values)
        return records
    except Exception:
        return []

def sql_escape(value):
    """Escapes single quotes for SQL insertion."""
    if isinstance(value, str):
        return value.replace("'", "''")
    return value
def cpp_string_escape(text):
    """Escapes a string for use inside a C++ double-quoted literal."""
    # 1. Escape backslashes first
    text = text.replace('\\', '\\\\')
    # 2. Escape double quotes
    text = text.replace('"', '\\"')
    # 3. Replace newlines with the \n character
    text = text.replace('\n', '\\n')
    return text
# --- GENERATION FUNCTIONS ---

def generate_users():
    print(f"Generating {CONFIG['NUM_USERS']} users...")
    fake = Faker()
    users = []
    for i in range(1, CONFIG['NUM_USERS'] + 1):
        users.append({
            "id": i,
            "username": fake.user_name(),
            "email": fake.email(),
            "password": "password123"
        })
    return users

def generate_auditorium_templates():
    print("Generating 5 auditorium templates...")
    return [
        {"id": 1, "desc": "Cozy Classic", "prem_rows": 1, "norm_rows": 7, "secs": [16, 0, 0]},
        {"id": 2, "desc": "Standard Twin", "prem_rows": 2, "norm_rows": 8, "secs": [8, 8, 0]},
        {"id": 3, "desc": "Grand Hall", "prem_rows": 2, "norm_rows": 10, "secs": [6, 12, 6]},
        {"id": 4, "desc": "Intimate Boutique", "prem_rows": 1, "norm_rows": 5, "secs": [12, 0, 0]},
        {"id": 5, "desc": "Mega Screen", "prem_rows": 3, "norm_rows": 12, "secs": [8, 16, 8]},
    ]

def generate_auditoriums(venues, templates):
    print("Generating auditoriums for each venue...")
    auditoriums = []
    audi_id_counter = 1
    for i, venue_data in enumerate(venues):
        venue_id = i + 1
        audi_count = int(venue_data[3]) # AuditoriumCount is the 4th value
        for j in range(1, audi_count + 1):
            template = random.choice(templates)
            auditoriums.append({
                "id": audi_id_counter,
                "venue_id": venue_id,
                "number": j,
                "template_id": template["id"],
                "normal_price": round(random.uniform(10.0, 14.0), 2),
                "premium_price": round(random.uniform(15.0, 22.0), 2)
            })
            audi_id_counter += 1
    return auditoriums

def generate_showtimes(movies, auditoriums):
    print("Generating realistic showtime schedule...")
    auditorium_timelines = {}
    screenings_to_schedule = []
    for i in range(len(movies)):
        movie_id = i + 1
        duration = int(movies[i][3])
        for _ in range(CONFIG["SHOWINGS_PER_MOVIE_TARGET"]):
            screenings_to_schedule.append({"id": movie_id, "duration": duration})
    
    random.shuffle(screenings_to_schedule)
    final_schedule = []
    showtime_id_counter = 1
    
    while screenings_to_schedule:
        movie = screenings_to_schedule.pop(0)
        scheduled = False
        for _ in range(50): # Try 50 random slots before giving up
            day = random.randint(0, CONFIG["DAYS_TO_SCHEDULE"] - 1)
            auditorium = random.choice(auditoriums)
            
            start_hour = random.randint(CONFIG["OPENING_TIME"].hour, CONFIG["LAST_SHOW_START"].hour)
            start_minute = random.choice([0, 15, 30, 45])
            start_time = time(start_hour, start_minute)
            
            start_dt = datetime.combine(datetime.today(), start_time)
            end_dt = start_dt + timedelta(minutes=movie["duration"] + CONFIG["CLEANING_BUFFER_MINUTES"])
            
            timeline_key = (day, auditorium["id"])
            if timeline_key not in auditorium_timelines:
                auditorium_timelines[timeline_key] = []

            is_collision = any(max(start_dt, busy_start) < min(end_dt, busy_end) for busy_start, busy_end in auditorium_timelines[timeline_key])
            
            if not is_collision:
                auditorium_timelines[timeline_key].append((start_dt, end_dt))
                final_schedule.append({
                    "id": showtime_id_counter,
                    "movie_id": movie["id"],
                    "venue_id": auditorium["venue_id"],
                    "auditorium_id": auditorium["id"],
                    "day_offset": day,
                    "time": start_time.strftime("%H:%M:%S")
                })
                showtime_id_counter += 1
                scheduled = True
                break
    
    print(f"Successfully generated {len(final_schedule)} showtimes.")
    return sorted(final_schedule, key=lambda x: (x['day_offset'], x['venue_id'], x['auditorium_id'], x['time']))

def generate_bookings(users, showtimes, auditoriums, templates):
    print("Generating bookings...")
    showtime_seat_maps = {}
    template_map = {t['id']: t for t in templates}
    auditorium_map = {a['id']: a for a in auditoriums}
    
    bookings = []
    total_bookings_to_make = int(CONFIG["NUM_USERS"] * CONFIG["AVG_BOOKINGS_PER_USER"])

    for _ in range(total_bookings_to_make):
        user_id = random.randint(1, len(users))
        showtime = random.choice(showtimes)
        showtime_id = showtime['id']
        
        if showtime_id not in showtime_seat_maps:
            showtime_seat_maps[showtime_id] = set()

        auditorium = auditorium_map[showtime['auditorium_id']]
        template = template_map[auditorium['template_id']]
        
        total_rows = template['prem_rows'] + template['norm_rows']
        
        available_seats = []
        total_seats_so_far = 0
        for sec_count in template['secs']:
            if sec_count == 0: continue
            for r in range(total_rows):
                row_letter = chr(65 + r)
                for c in range(1, sec_count + 1):
                    seat_num = c + total_seats_so_far
                    seat_id = f"{row_letter}{seat_num}"
                    if seat_id not in showtime_seat_maps[showtime_id]:
                        available_seats.append(seat_id)
            total_seats_so_far += sec_count

        if not available_seats:
            continue

        num_seats = max(1, int(random.gauss(CONFIG["AVG_SEATS_PER_BOOKING"], 1.5)))
        if num_seats > len(available_seats):
            continue

        start_index = random.randint(0, len(available_seats) - num_seats)
        seats_to_book = available_seats[start_index : start_index + num_seats]
        
        for seat in seats_to_book:
            bookings.append({"showtime_id": showtime_id, "user_id": user_id, "seat": seat})
            showtime_seat_maps[showtime_id].add(seat)

    print(f"Successfully generated {len(bookings)} individual seat bookings.")
    return bookings

# In generate_master_seed.py

def format_cpp_output(users, movies_sql, venues_sql, templates, auditoriums, showtimes, bookings):
    print("Formatting data for C++ header file with individual functions...")

    # Build individual INSERT statements
    users_sql_str = "INSERT INTO Users (UserID, Username, Email, Password) VALUES " + ", ".join([f"({u['id']}, '{sql_escape(u['username'])}', '{sql_escape(u['email'])}', '{u['password']}')" for u in users]) + ";"
    templates_sql_str = "INSERT INTO AuditoriumTemplates (TemplateID, Description, PremiumRows, NormalRows, Section1Seats, Section2Seats, Section3Seats) VALUES " + ", ".join([f"({t['id']}, '{t['desc']}', {t['prem_rows']}, {t['norm_rows']}, {t['secs'][0]}, {t['secs'][1]}, {t['secs'][2]})" for t in templates]) + ";"
    auditoriums_sql_str = "INSERT INTO Auditoriums (AuditoriumID, VenueID, AuditoriumNumber, TemplateID, NormalPrice, PremiumPrice) VALUES " + ", ".join([f"({a['id']}, {a['venue_id']}, {a['number']}, {a['template_id']}, {a['normal_price']:.2f}, {a['premium_price']:.2f})" for a in auditoriums]) + ";"
    bookings_sql_str = "INSERT INTO Bookings (ShowtimeID, UserID, SeatIdentifier) VALUES " + ", ".join([f"({b['showtime_id']}, {b['user_id']}, '{b['seat']}')" for b in bookings]) + ";"

    # Build the date-agnostic showtime vector
    showtime_structs = ",\\n".join([f"    {{{s['movie_id']}, {s['venue_id']}, {s['auditorium_id']}, {s['day_offset']}, \"{s['time']}\"}}" for s in showtimes])
    showtime_vector_string = f"static const std::vector<ShowtimeSeed> showtime_seeds = {{\n{showtime_structs}\n}};"

    # Assemble the final C++ file with separate functions
    cpp_code = f"""#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

// This file is auto-generated by the Python script. Do not edit manually.

namespace data {{

struct ShowtimeSeed {{ int movie_id; int venue_id; int auditorium_id; int day_offset; const char* time; }};
{showtime_vector_string}

// Each function returns the SQL for a single table.
static std::string getUsersSQL() {{ return R"({users_sql_str})"; }}
static std::string getMoviesSQL() {{ return R"({movies_sql.strip()})"; }}
static std::string getVenuesSQL() {{ return R"({venues_sql.strip()})"; }}
static std::string getAuditoriumTemplatesSQL() {{ return R"({templates_sql_str})"; }}
static std::string getAuditoriumsSQL() {{ return R"({auditoriums_sql_str})"; }}
static std::string getBookingsSQL() {{ return R"({bookings_sql_str})"; }}

static std::string generateShowtimesSQL() {{
    if (showtime_seeds.empty()) return "";
    const auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    std::stringstream sql;
    sql << "INSERT INTO Showtimes (MovieID, VenueID, AuditoriumID, ShowtimeDateTime) VALUES ";
    for (size_t i = 0; i < showtime_seeds.size(); ++i) {{
        const auto& seed = showtime_seeds[i];
        const auto target_day = today + std::chrono::days(seed.day_offset);
        const std::chrono::year_month_day ymd{{target_day}};
        sql << "(" << seed.movie_id << ", " << seed.venue_id << ", " << seed.auditorium_id << ", "
            << "'" << static_cast<int>(ymd.year()) << "-" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.month()) << "-" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.day()) << " " << seed.time << "')";
        if (i < showtime_seeds.size() - 1) sql << ",\\n";
    }}
    sql << ";";
    return sql.str();
}}

}} // namespace data
"""
    return cpp_code
# --- MAIN EXECUTION ---
if __name__ == "__main__":
    # 1. Parse input data
    movies_parsed = parse_sql_insert(MOVIES_SQL)
    venues_parsed = parse_sql_insert(VENUES_SQL)
    if not movies_parsed or not venues_parsed:
        print("ERROR: MOVIES_SQL or VENUES_SQL is empty. Please paste your data into the script.")
    else:
        # Skip the header row explicitly
        movies_parsed = movies_parsed[1:]

        # Debugging: Print parsed movies to verify header exclusion
        print("Parsed Movies:", movies_parsed[:5])  # Print the first 5 entries for verification

        # 2. Generate all dynamic data
        users = generate_users()
        templates = generate_auditorium_templates()
        auditoriums = generate_auditoriums(venues_parsed, templates)
        showtimes = generate_showtimes(movies_parsed, auditoriums)
        bookings = generate_bookings(users, showtimes, auditoriums, templates)

        # 3. Format the final C++ header file content
        cpp_file_content = format_cpp_output(users, MOVIES_SQL, VENUES_SQL, templates, auditoriums, showtimes, bookings)

        # 4. Write to file
        output_filename = "seed_data.hpp"
        with open(output_filename, "w") as f:
            f.write(cpp_file_content)
            
        print(f"\nSuccessfully created C++ seed file: {output_filename}")