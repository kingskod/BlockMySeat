document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- DOM Elements ---
    const upcomingList = document.getElementById('upcoming-bookings-list');
    const previousList = document.getElementById('previous-bookings-list');
    const cardTemplate = document.getElementById('booking-card-template');
    const body = document.body;
    const themeToggle = document.getElementById('sidebar-theme-toggle'); // Correct ID for the sidebar toggle
    const homeIcon = document.getElementById('home-icon');

    // --- Dark Mode Logic (Complete and Correct) ---
    const applyTheme = (theme) => {
        if (theme === 'night') {
            body.classList.remove('day-mode');
            body.classList.add('night-mode');
            if (homeIcon) homeIcon.src = '../icons/Lanter_night.webp';
        } else {
            body.classList.remove('night-mode');
            body.classList.add('day-mode');
            if (homeIcon) homeIcon.src = '../icons/Lanter_day.webp';
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

    // --- Main Initialization ---
    const searchBar = document.querySelector('.search-bar input');
    let allBookings = { upcoming: [], previous: [] };

    const initializePage = async () => {
        const userId = sessionStorage.getItem('userId');
        /*if (!userId) {
            alert('You must be logged in to view your bookings.');
            window.location.href = 'index.html';
            return;
        }*/

        try {
            const response = await fetch(`${serverUrl}/my-bookings/${userId}`);
            if (!response.ok) throw new Error('Could not fetch bookings');
            allBookings = await response.json();

            populateList(upcomingList, allBookings.upcoming);
            populateList(previousList, allBookings.previous);

        } catch (error) {
            console.error('Failed to load bookings:', error);
            if (upcomingList) {
                upcomingList.innerHTML = "<p>Could not load your bookings.</p>";
            }
        }
    };

    const populateList = (listElement, bookings) => {
        if (!listElement) return; // Safety check

        if (!bookings || bookings.length === 0) {
            listElement.innerHTML = "<p>No bookings found.</p>";
            return;
        }

        listElement.innerHTML = ''; // Clear the list

        bookings.forEach(booking => {
            const newCard = cardTemplate.cloneNode(true);
            newCard.removeAttribute('id');
            newCard.style.display = 'flex';

            newCard.querySelector('.booking-poster').src = booking.poster_url;
            newCard.querySelector('.booking-movie-title').textContent = booking.movie_title;
            newCard.querySelector('.booking-venue-name').textContent = booking.venue_name;
            
            const displayDate = new Date(booking.show_date + 'T00:00:00').toLocaleDateString('en-US', {
                weekday: 'short', month: 'short', day: 'numeric'
            });
            newCard.querySelector('.booking-date').textContent = displayDate;
            
            newCard.querySelector('.booking-seat-count').textContent = `${booking.seat_count} Seat(s)`;

            listElement.appendChild(newCard);
        });
    };

    initializePage();

    searchBar.addEventListener('input', (e) => {
        const searchTerm = e.target.value.toLowerCase();

        const filteredUpcoming = allBookings.upcoming.filter(booking =>
            booking.movie_title.toLowerCase().includes(searchTerm) ||
            booking.venue_name.toLowerCase().includes(searchTerm)
        );
        populateList(upcomingList, filteredUpcoming);

        const filteredPrevious = allBookings.previous.filter(booking =>
            booking.movie_title.toLowerCase().includes(searchTerm) ||
            booking.venue_name.toLowerCase().includes(searchTerm)
        );
        populateList(previousList, filteredPrevious);
    });

    // --- Logout Prompt Logic ---
    
});