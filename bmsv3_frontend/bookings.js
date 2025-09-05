document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- DOM Elements ---
    const upcomingList = document.getElementById('upcoming-bookings-list');
    const previousList = document.getElementById('previous-bookings-list');
    const cardTemplate = document.getElementById('booking-card-template');
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

    // --- Main Initialization ---
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
            const bookings = await response.json();

            populateList(upcomingList, bookings.upcoming);
            populateList(previousList, bookings.previous);

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
});