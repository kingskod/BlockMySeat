document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get data from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const movieTitle = decodeURIComponent(urlParams.get('movie'));
    const venueName = decodeURIComponent(urlParams.get('venue'));
    const posterUrl = decodeURIComponent(urlParams.get('poster'));
    const showtimeId = urlParams.get('showtime_id');
    const auditoriumId = urlParams.get('auditorium_id');
    const date = urlParams.get('date');
    const time = urlParams.get('time');
    const seats = urlParams.get('seats').split(',');

    // --- DOM Elements ---
    const confirmBtn = document.getElementById('confirm-booking-btn');
    const guestBlocker = document.getElementById('guest-blocker');

    // --- Dark Mode & Header Logic ---
    const themeToggle = document.getElementById('theme-toggle');
    const body = document.body;
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
        applyTheme('day');
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
        // 1. Populate static info
        document.getElementById('movie-poster').src = posterUrl;
        document.getElementById('movie-title').textContent = movieTitle;
        document.getElementById('venue-name').textContent = venueName;
        const displayDate = new Date(date + 'T00:00:00').toLocaleDateString('en-US', { month: 'long', day: 'numeric' });
        document.getElementById('show-date-time').textContent = `${displayDate} at ${time}`;
        document.getElementById('seat-count').textContent = seats.length;
        document.getElementById('seat-numbers').textContent = seats.join(', ');

        // 2. Fetch auditorium details to get prices
        try {
            const response = await fetch(`${serverUrl}/auditorium-details/${auditoriumId}`);
            const audiDetails = await response.json();
            
            // 3. Calculate Price
            let totalPrice = 0;
            let priceBreakdown = [];
            seats.forEach(seatId => {
                const rowLetter = seatId.charAt(0);
                const rowIndex = rowLetter.charCodeAt(0) - 65;
                
                if (rowIndex < audiDetails.layout.premium_rows) {
                    totalPrice += audiDetails.premium_price;
                    priceBreakdown.push(`$${audiDetails.premium_price.toFixed(2)}`);
                } else {
                    totalPrice += audiDetails.normal_price;
                    priceBreakdown.push(`$${audiDetails.normal_price.toFixed(2)}`);
                }
            });
            document.getElementById('price-breakdown').textContent = priceBreakdown.join(' + ');
            document.getElementById('total-price').textContent = `$${totalPrice.toFixed(2)}`;

        } catch (error) {
            console.error("Could not fetch auditorium details:", error);
            document.querySelector('.booking-details').innerHTML = "<p>Error calculating price.</p>";
        }

        // 4. Check if user is a guest
        const userToken = sessionStorage.getItem('userToken');
        if (!userToken) {
            confirmBtn.disabled = true;
            confirmBtn.style.display = 'none';
            guestBlocker.classList.remove('hidden');
        }
    };

    // --- Event Listeners ---
    confirmBtn.addEventListener('click', async () => {
        const userId = sessionStorage.getItem('userId');
        
        try {
            const response = await fetch(`${serverUrl}/book-tickets`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    showtime_id: parseInt(showtimeId),
                    user_id: parseInt(userId),
                    seats: seats
                })
            });
            const result = await response.json();
            if (result.status === 'success') {
                alert('Booking Successful!');
                window.location.href = 'movies.html';
            } else {
                alert('Booking failed. Please try again.');
            }
        } catch (error) {
            console.error("Booking failed:", error);
            alert('Booking failed due to a network error.');
        }
    });

    // --- THIS IS THE CRUCIAL PART THAT WAS MISSING ---
    // Start the page initialization process
    initializePage();
});