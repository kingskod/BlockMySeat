document.addEventListener('DOMContentLoaded', () => {
    
    // --- Carousel Logic ---
    const track = document.querySelector('.carousel-track');
    const slides = Array.from(track.children);
    const nextButton = document.querySelector('.carousel-button.next');
    const prevButton = document.querySelector('.carousel-button.prev');

    let currentSlideIndex = 0;

    const moveToSlide = (targetIndex) => {
        // Make sure the target index is within the bounds of the slides array
        if (targetIndex < 0) {
            targetIndex = slides.length - 1;
        } else if (targetIndex >= slides.length) {
            targetIndex = 0;
        }

        // Remove 'active' class from the current slide
        slides[currentSlideIndex].classList.remove('active');
        // Add 'active' class to the new target slide
        slides[targetIndex].classList.add('active');
        
        // Update the current slide index
        currentSlideIndex = targetIndex;
    };

    // When I click the right button, move to the next slide
    nextButton.addEventListener('click', e => {
        moveToSlide(currentSlideIndex + 1);
    });

    // When I click the left button, move to the previous slide
    prevButton.addEventListener('click', e => {
        moveToSlide(currentSlideIndex - 1);
    });

    // Optional: Auto-play the carousel
    setInterval(() => {
        moveToSlide(currentSlideIndex + 1);
    }, 5000); // Change slide every 5 seconds
    const movieGrid = document.querySelector('.grid-content');
    const cardTemplate = document.getElementById('movie-card-template');


    // fetch movies type shi
    
    const fetchMovies = async () => {
        try {
            const response = await fetch('http://12.0.0.1:18080/movies');
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            const movies = await response.json();
            
            // Clear any static placeholder cards
            const placeholders = movieGrid.querySelectorAll('.movie-card:not(#movie-card-template)');
            placeholders.forEach(card => card.remove());

            movies.forEach(movie => {
                const newCard = cardTemplate.cloneNode(true);
                newCard.removeAttribute('id');
                newCard.style.display = 'block';

                newCard.querySelector('.card-poster').src = movie.poster_url;
                newCard.querySelector('.card-poster').alt = movie.title;
                newCard.querySelector('.card-title').textContent = movie.title;
                newCard.querySelector('.card-rating').textContent = `⭐ ${movie.rating}`;
                newCard.querySelector('.card-genre').textContent = `${movie.duration_minutes} min`;

                movieGrid.appendChild(newCard);
            });

        } catch (error) {
            console.error('Failed to fetch movies:', error);
            movieGrid.innerHTML = '<p style="color: var(--text-primary);">Could not load movies. Is the C++ server running?</p>';
        }
    };

    fetchMovies();
});