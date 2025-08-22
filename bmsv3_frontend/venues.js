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

    // === THIS IS THE CORRECTED DARK MODE LOGIC ===
    const themeSwitch = document.getElementById('theme-checkbox');
    const body = document.body;

    // Function to apply the theme and save the preference
    const applyTheme = (theme) => {
        if (theme === 'night') {
            body.classList.remove('day-mode');
            body.classList.add('night-mode');
            if (themeSwitch) themeSwitch.checked = true;
        } else {
            body.classList.remove('night-mode');
            body.classList.add('day-mode');
            if (themeSwitch) themeSwitch.checked = false;
        }
    };

    // 1. Check for a saved theme in localStorage when the page loads
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
        applyTheme(savedTheme);
    }

    // 2. Add the event listener for the toggle switch
    if (themeSwitch) {
        themeSwitch.addEventListener('change', () => {
            if (themeSwitch.checked) {
                localStorage.setItem('theme', 'night');
                applyTheme('night');
            } else {
                localStorage.setItem('theme', 'day');
                applyTheme('day');
            }
        });
    }
    // === END OF CORRECTED LOGIC ===

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
            venueListContainer.appendChild(venueTemplate);

            venues.forEach(venue => {
                const newItem = venueTemplate.cloneNode(true);
                newItem.removeAttribute('id');
                newItem.style.display = 'grid';

                newItem.querySelector('.venue-image').src = venue.image_url;
                newItem.querySelector('.venue-name').textContent = venue.name;
                newItem.querySelector('.venue-location').innerHTML = `<i class="fa-solid fa-map-marker-alt"></i> ${venue.location}`;
                newItem.querySelector('.venue-auditoriums').innerHTML = `<i class="fa-solid fa-film"></i> ${venue.auditorium_count} Auditoriums`;
                newItem.querySelector('.venue-rating').innerHTML = createStarRating(venue.rating);

                venueListContainer.appendChild(newItem);
            });
        } catch (error) {
            console.error('Failed to fetch venues:', error);
            venueListContainer.innerHTML = '<p style="color: var(--text-primary);">Could not load venues. Is the C++ server running?</p>';
        }
    };

    fetchVenues();
});