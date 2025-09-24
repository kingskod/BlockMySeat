document.addEventListener('DOMContentLoaded', () => 
{
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get Venue ID from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const venueId = urlParams.get('id');

    if (!venueId) 
    {
        document.body.innerHTML = '<h1>Venue ID not found in URL. Please select a venue from the venues page.</h1>';
        return;
    }

    // --- DOM Elements ---
    const venuePoster = document.getElementById('venue-poster'); // Changed ID
    const venueName = document.getElementById('venue-name');     // Changed ID
    const venueLocation = document.getElementById('venue-location'); // New
    const venueAuditoriums = document.getElementById('venue-auditoriums'); // New
    const venueRating = document.getElementById('venue-rating'); // Changed ID, will hold stars
    const datesBar = document.querySelector('.dates-bar');
    const movieList = document.getElementById('movie-list');       // Changed ID
    const movieTemplate = document.getElementById('movie-item-template'); // Changed ID
    const themeToggle = document.getElementById('theme-toggle');
    const body = document.body;

    // Function to apply the theme
    const applyTheme = (theme) => 
    {
        if (theme === 'night') 
        {
            body.classList.remove('day-mode');
            body.classList.add('night-mode');
        } 
        else 
        {
            body.classList.remove('night-mode');
            body.classList.add('day-mode');
        }
    };

    // 1. Check for a saved theme in localStorage when the page loads
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) 
    {
        applyTheme(savedTheme);
    } 
    else 
    {
        applyTheme('day'); // Default to day mode if nothing is saved
    }

    // 2. Add the event listener for the new toggle switch
    if (themeToggle) 
    {
        themeToggle.addEventListener('click', () => 
        {
            // Check which theme is currently active and switch to the other
            if (body.classList.contains('day-mode')) 
            {
                localStorage.setItem('theme', 'night');
                applyTheme('night');
            } 
            else
            {
                localStorage.setItem('theme', 'day');
                applyTheme('day');
            }
        });
    }

    // --- Helper Functions ---
    const createStarRating = (rating) => 
    {
        let starsHTML = '';
        const fullStars = Math.floor(rating);
        const halfStar = rating % 1 >= 0.5;
        const emptyStars = 5 - fullStars - (halfStar ? 1 : 0);

        for (let i = 0; i < fullStars; i++) starsHTML += '<i class="fa-solid fa-star"></i>';
        if (halfStar) starsHTML += '<i class="fa-solid fa-star-half-alt"></i>';
        for (let i = 0; i < emptyStars; i++) starsHTML += '<i class="fa-regular fa-star"></i>'; // Corrected to regular star for empty
        return starsHTML;
    };

    // --- Main Data Fetching Functions ---

    // 1. Fetch and display the main venue details
    const fetchVenueDetails = async () => 
    {
        try 
        {
            const response = await fetch(`${serverUrl}/venues/${venueId}`); // NEW ENDPOINT
            if (!response.ok) throw new Error('Venue not found');
            const venue = await response.json();

            document.title = `BlockMySeat - ${venue.name}`; // Update page title
            venuePoster.src = venue.image_url;
            venueName.textContent = venue.name;
            venueLocation.textContent = venue.location;
            venueAuditoriums.textContent = `${venue.auditorium_count} Auditoriums`;
            venueRating.innerHTML = createStarRating(venue.rating); // Populate with stars
        } 
        catch (error) 
        {
            console.error('Failed to fetch venue details:', error);
            document.querySelector('.venue-info-panel-fixed').innerHTML = '<h1>Could not load venue details.</h1>';
        }
    };

    // 2. Generate the dates bar (same as movie-details)
    const generateDates = () => 
    {
        const today = new Date();
        for (let i = 0; i < 7; i++) 
        { // Generate for the next 7 days
            const date = new Date(today);
            date.setDate(today.getDate() + i);

            const dateItem = document.createElement('div');
            dateItem.className = 'date-item';
            // Store the date in YYYY-MM-DD format for the API call
            dateItem.dataset.date = date.toISOString().split('T')[0]; 
            
            dateItem.innerHTML = `
                <span class="date-day">${date.toLocaleDateString('en-US', { weekday: 'short' })}</span>
                <span class="date-number">${date.getDate()}</span>
            `;

            if (i === 0) 
            {
                dateItem.classList.add('active'); // Make today the default active date
            }

            dateItem.addEventListener('click', () =>
            {
                // Handle date selection
                document.querySelector('.date-item.active').classList.remove('active');
                dateItem.classList.add('active');
                fetchMoviesAndShowtimes(dateItem.dataset.date); // Re-fetch movies and showtimes for the new date
            });

            datesBar.appendChild(dateItem);
        }
    };

    // 3. Fetch and display movies and showtimes for a given date at this venue
    const fetchMoviesAndShowtimes = async (date) => 
    {
        movieList.innerHTML = '<p>Loading movies and showtimes...</p>'; // Show loading message
        try 
        {
            const response = await fetch(`${serverUrl}/venue-showtimes?venue_id=${venueId}&date=${date}`); // NEW ENDPOINT
            if (!response.ok) throw new Error('Could not fetch movies and showtimes');
            const movies = await response.json();

            movieList.innerHTML = ''; // Clear loading message

            if (movies.length === 0) 
            {
                movieList.innerHTML = '<p>No movies playing for this date at this venue.</p>';
                return;
            }

            movies.forEach(movie => 
            {
                const newItem = movieTemplate.cloneNode(true);
                newItem.removeAttribute('id');
                newItem.style.display = 'flex'; // Ensure it's displayed

                newItem.querySelector('.movie-image').src = movie.poster_url;
                newItem.querySelector('.movie-name').textContent = movie.movie_title;
                newItem.querySelector('.movie-duration').textContent = `${movie.duration_minutes} min`;
                newItem.querySelector('.movie-rating').textContent = movie.rating; // Display rating as text, will style with CSS later
                newItem.querySelector('.movie-synopsis').textContent = movie.synopsis;

                const timingsContainer = newItem.querySelector('.movie-timings');
                timingsContainer.innerHTML = ''; // Clear any template buttons
                movie.showtimes.forEach(showtime => 
                {
                    const timeButton = document.createElement('button');
                    const selectedDate = document.querySelector('.date-item.active').dataset.date;
                    timeButton.className = 'time-btn';
                    timeButton.textContent = showtime.time;

                    // Add event listener to redirect to seats.html with parameters
                    timeButton.addEventListener('click', () => {
                        // Pass venueName and venuePoster.src from the *current page's* fixed panel
                        const currentVenueName = venueName.textContent || 'Unknown Venue';
                        const currentVenuePosterUrl = venuePoster.src || ''; // This is the venue image

                        const params = new URLSearchParams(
                        {
                            movie: movie.movie_title,
                            venue: currentVenueName,         // This venue's name
                            poster: movie.poster_url,        // The specific movie's poster
                            showtime_id: showtime.showtime_id,
                            auditorium_id: showtime.auditorium_id,
                            date: selectedDate,
                            time: showtime.time
                        });
                        window.location.href = `seats.html?${params.toString()}`;
                    });

                    timingsContainer.appendChild(timeButton);
                });

                movieList.appendChild(newItem);
            });

        } 
        catch (error) 
        {
            console.error('Failed to fetch movies and showtimes:', error);
            movieList.innerHTML = '<p>Could not load movies for this venue and date. Please ensure the server is running and the data is available.</p>';
        }
    };

    // --- Initial Page Load ---
    fetchVenueDetails();
    generateDates();
    // Fetch movies and showtimes for today's date by default
    fetchMoviesAndShowtimes(new Date().toISOString().split('T')[0]);
});