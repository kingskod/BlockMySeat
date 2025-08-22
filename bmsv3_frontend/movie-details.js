document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get Movie ID from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const movieId = urlParams.get('id');

    if (!movieId) {
        document.body.innerHTML = '<h1>Movie ID not found in URL.</h1>';
        return;
    }

    // --- DOM Elements ---
    const moviePoster = document.getElementById('movie-poster');
    const movieTitle = document.getElementById('movie-title');
    const movieDuration = document.getElementById('movie-duration');
    const movieRating = document.getElementById('movie-rating');
    const movieSynopsis = document.getElementById('movie-synopsis');
    const datesBar = document.querySelector('.dates-bar');
    const venueList = document.getElementById('venue-list');
    const venueTemplate = document.getElementById('venue-item-template');

    // --- Helper Functions ---
    const createStarRating = (rating) => {
        let starsHTML = '';
        const fullStars = Math.floor(rating);
        const halfStar = rating % 1 >= 0.5;
        for (let i = 0; i < fullStars; i++) starsHTML += '<i class="fa-solid fa-star"></i>';
        if (halfStar) starsHTML += '<i class="fa-solid fa-star-half-alt"></i>';
        for (let i = 0; i < 5 - fullStars - (halfStar ? 1 : 0); i++) starsHTML += '<i class="fa-regular fa-star"></i>';
        return starsHTML;
    };

    // --- Main Data Fetching Functions ---

    // 1. Fetch and display the main movie details
    const fetchMovieDetails = async () => {
        try {
            const response = await fetch(`${serverUrl}/movies/${movieId}`);
            if (!response.ok) throw new Error('Movie not found');
            const movie = await response.json();

            document.title = `BlockMySeat - ${movie.title}`; // Update page title
            moviePoster.src = movie.poster_url;
            movieTitle.textContent = movie.title;
            movieDuration.textContent = `${movie.duration_minutes} min`;
            movieRating.textContent = movie.rating;
            movieSynopsis.textContent = movie.synopsis;
        } catch (error) {
            console.error('Failed to fetch movie details:', error);
            document.querySelector('.details-container').innerHTML = '<h1>Could not load movie details.</h1>';
        }
    };

    // 2. Generate the dates bar
    const generateDates = () => {
        const today = new Date();
        for (let i = 0; i < 7; i++) { // Generate for the next 7 days
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

            if (i === 0) {
                dateItem.classList.add('active'); // Make today the default active date
            }

            dateItem.addEventListener('click', () => {
                // Handle date selection
                document.querySelector('.date-item.active').classList.remove('active');
                dateItem.classList.add('active');
                fetchShowtimes(dateItem.dataset.date); // Re-fetch showtimes for the new date
            });

            datesBar.appendChild(dateItem);
        }
    };

    // 3. Fetch and display showtimes for a given date
    const fetchShowtimes = async (date) => {
        venueList.innerHTML = '<p>Loading showtimes...</p>'; // Show loading message
        try {
            const response = await fetch(`${serverUrl}/showtimes?movie_id=${movieId}&date=${date}`);
            if (!response.ok) throw new Error('Could not fetch showtimes');
            const venues = await response.json();

            venueList.innerHTML = ''; // Clear loading message

            if (venues.length === 0) {
                venueList.innerHTML = '<p>No showtimes available for this date.</p>';
                return;
            }

            venues.forEach(venue => {
                const newItem = venueTemplate.cloneNode(true);
                newItem.removeAttribute('id');
                newItem.style.display = 'grid';

                newItem.querySelector('.venue-image').src = venue.venue_image_url;
                newItem.querySelector('.venue-name').textContent = venue.venue_name;
                newItem.querySelector('.venue-rating').innerHTML = createStarRating(venue.venue_rating);

                const timingsContainer = newItem.querySelector('.venue-timings');
                timingsContainer.innerHTML = ''; // Clear any template buttons
                venue.showtimes.forEach(time => {
                    const timeButton = document.createElement('button');
                    timeButton.className = 'time-btn';
                    timeButton.textContent = time;
                    timingsContainer.appendChild(timeButton);
                });

                venueList.appendChild(newItem);
            });

        } catch (error) {
            console.error('Failed to fetch showtimes:', error);
            venueList.innerHTML = '<p>Could not load showtimes.</p>';
        }
    };

    // --- Initial Page Load ---
    fetchMovieDetails();
    generateDates();
    // Fetch showtimes for today's date by default
    fetchShowtimes(new Date().toISOString().split('T')[0]);
});