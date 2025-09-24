document.addEventListener('DOMContentLoaded', () => {
    
    // --- Carousel Logic ---
    const track = document.querySelector('.carousel-track');
    if (track) {
        const slides = Array.from(track.children);
        const nextButton = document.querySelector('.carousel-button.next');
        const prevButton = document.querySelector('.carousel-button.prev');
        let currentSlideIndex = 0;

        const moveToSlide = (targetIndex) => {
            if (slides.length === 0) return;
            if (targetIndex < 0) targetIndex = slides.length - 1;
            else if (targetIndex >= slides.length) targetIndex = 0;

            slides[currentSlideIndex].classList.remove('active');
            slides[targetIndex].classList.add('active');
            currentSlideIndex = targetIndex;
        };

        if (nextButton && prevButton) {
            nextButton.addEventListener('click', () => moveToSlide(currentSlideIndex + 1));
            prevButton.addEventListener('click', () => moveToSlide(currentSlideIndex - 1));
        }
    }

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
    // --- Fetch and Display Venue Data ---
    const venueListContainer = document.getElementById('venue-list-container');
    const venueTemplate = document.getElementById('venue-item-template');

    const createStarRating = (rating) => {
        let starsHTML = '';
        const fullStars = Math.floor(rating);
        const halfStar = rating % 1 >= 0.5;
        const emptyStars = 5 - fullStars - (halfStar ? 1 : 0);

        for (let i = 0; i < fullStars; i++) starsHTML += '<i class="fa-solid fa-star"></i>';
        if (halfStar) starsHTML += '<i class="fa-solid fa-star-half-alt"></i>';
        for (let i = 0; i < emptyStars; i++) starsHTML += '<i class="fa-regular fa-star"></i>';
        
        return starsHTML;
    };

    const fetchVenues = async () => {
    if (!venueListContainer || !venueTemplate) return;
    try {
        const response = await fetch('http://127.0.0.1:18080/venues');
        if (!response.ok) throw new Error('Network response was not ok');
        const venues = await response.json();

        venueListContainer.innerHTML = '';
        venueListContainer.appendChild(venueTemplate); // Keep the template hidden

        venues.forEach(venue => {
            const newItem = venueTemplate.cloneNode(true);
            newItem.removeAttribute('id');
            newItem.style.display = 'grid';

            newItem.querySelector('.venue-image').src = venue.image_url;
            newItem.querySelector('.venue-name').textContent = venue.name;
            newItem.querySelector('.venue-location').innerHTML = `<i class="fa-solid fa-map-marker-alt"></i> ${venue.location}`;
            newItem.querySelector('.venue-auditoriums').innerHTML = `<i class="fa-solid fa-film"></i> ${venue.auditorium_count} Auditoriums`;
            newItem.querySelector('.venue-rating').innerHTML = createStarRating(venue.rating);

            // --- NEW CODE STARTS HERE ---
            // Make the entire card clickable
            newItem.addEventListener('click', () => {
                window.location.href = `venue-details.html?id=${venue.id}`;
            });

            // Get the "See Showtimes" button and set its href directly
            // This ensures the button itself is also a link, even if the card is clicked.
            const seeShowtimesBtn = newItem.querySelector('.btn-primary');
            if (seeShowtimesBtn) {
                seeShowtimesBtn.href = `venue-details.html?id=${venue.id}`;
                // Optional: Prevent immediate redirection if the button is clicked,
                // letting the card's click handler manage it, or vice versa.
                // For now, both will point to the same URL.
            }
            // --- NEW CODE ENDS HERE ---

            venueListContainer.appendChild(newItem);
        });
    } catch (error) {
        console.error('Failed to fetch venues:', error);
        venueListContainer.innerHTML = '<p style="color: var(--text-primary);">Could not load venues. Is the C++ server running?</p>';
    }
};
    fetchVenues();
});