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
            const response = await fetch('http://localhost:18080/movies'); // Updated endpoint
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
                newCard.addEventListener('click', () => {
                    window.location.href = `movie-details.html?id=${movie.id}`;
                });
                movieGrid.appendChild(newCard);
            });

        } catch (error) {
            console.error('Failed to fetch movies:', error);
            movieGrid.innerHTML = '<p style="color: var(--text-primary);">Could not load movies. Is the C++ server running?</p>';
        }
    };

    fetchMovies();
    const body = document.body;
    const themeToggle = document.getElementById('sidebar-theme-toggle'); // Correct ID for the sidebar toggle

    // --- Dark Mode Logic (Complete and Correct) ---
    const applyTheme = (theme) => {
        if (theme === 'night') {
            body.classList.remove('day-mode');
            body.classList.add('night-mode');
        } else {
            body.classList.remove('night-mode');
            body.classList.add('day-mode');
        }
    };

    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
        applyTheme(savedTheme);
    } else {
        applyTheme('day'); // Default to day mode
    }

    if (themeToggle) {
        themeToggle.addEventListener('click', () => {
            if (body.classList.contains('day-mode')) {
                localStorage.setItem('theme', 'night');
                applyTheme('night');
            } else {
                localStorage.setItem('theme', 'day');
                applyTheme('day');
            }
        });
    }

    // --- Logout Prompt Logic ---
    const logoutButton = document.getElementById('logout-button');
    const logoutPrompt = document.getElementById('logout-prompt-overlay');
    const logoutConfirmBtn = document.getElementById('logout-confirm-btn');
    const logoutCancelBtn = document.getElementById('logout-cancel-btn');
    const logoutCloseBtn = document.getElementById('logout-close-btn');

    if (logoutButton && logoutPrompt) {
        logoutButton.addEventListener('click', (e) => {
            e.preventDefault(); // Prevent default link behavior
            logoutPrompt.classList.remove('hidden');
        });

        const closeLogoutPrompt = () => {
            logoutPrompt.classList.add('hidden');
        };

        const continueAsGuest = () => {
            sessionStorage.clear();
            closeLogoutPrompt();
            // Optional: You might want to refresh the page to update the UI
            // window.location.reload();
        };

        logoutCancelBtn.addEventListener('click', continueAsGuest);
        logoutCloseBtn.addEventListener('click', closeLogoutPrompt);

        logoutConfirmBtn.addEventListener('click', () => {
            sessionStorage.clear();
            window.location.href = 'index.html';
        });
    }
});