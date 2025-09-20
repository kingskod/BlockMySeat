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

// --- Logout Confirmation Logic ---
    const logoutButton = document.getElementById('logout-button');
    const logoutPromptOverlay = document.getElementById('logout-prompt-overlay');
    const confirmLogoutBtn = document.getElementById('confirm-logout-btn');
    const cancelLogoutBtn = document.getElementById('cancel-logout-btn');

    // Ensure all elements exist before adding listeners
    if (logoutButton && logoutPromptOverlay && confirmLogoutBtn && cancelLogoutBtn) {
        
        // 1. When the user clicks the "Log out" link in the sidebar
        logoutButton.addEventListener('click', (event) => {
            event.preventDefault(); // Stop the link from navigating anywhere
            // Show the prompt by removing the 'hidden' class
            logoutPromptOverlay.classList.remove('hidden');
        });

        // 2. When the user clicks the "Cancel" button inside the prompt
        cancelLogoutBtn.addEventListener('click', () => {
            // Hide the prompt by adding the 'hidden' class back
            logoutPromptOverlay.classList.add('hidden');
        });

        // 3. When the user clicks the final "Log Out" button
        confirmLogoutBtn.addEventListener('click', () => {
            // Clear the user's session from the browser
            sessionStorage.removeItem('userToken');
            sessionStorage.removeItem('userId');
            
            // Redirect to the login page
            window.location.href = 'index.html';
        });
    }
    // fetch movies type shi
    const searchBar = document.querySelector('.search-bar input');
    let allMovies = [];

    const renderMovies = (movies) => {
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
    };

    const fetchMovies = async () => {
        try {
            const response = await fetch('http://localhost:18080/movies'); // Updated endpoint
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            allMovies = await response.json();
            renderMovies(allMovies);

        } catch (error) {
            console.error('Failed to fetch movies:', error);
            movieGrid.innerHTML = '<p style="color: var(--text-primary);">Could not load movies. Is the C++ server running?</p>';
        }
    };

    fetchMovies();

    searchBar.addEventListener('input', (e) => {
        const searchTerm = e.target.value.toLowerCase();
        const filteredMovies = allMovies.filter(movie => movie.title.toLowerCase().includes(searchTerm));
        renderMovies(filteredMovies);
    });
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
    
});