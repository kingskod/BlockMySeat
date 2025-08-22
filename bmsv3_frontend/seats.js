document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get data from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const movieTitle = decodeURIComponent(urlParams.get('movie'));
    const showtimeId = urlParams.get('showtime_id');
    const auditoriumId = urlParams.get('auditorium_id');
    const date = urlParams.get('date');
    const time = urlParams.get('time');

    // --- DOM Elements ---
    const theaterContainer = document.getElementById('theater-container');
    const guestPromptOverlay = document.getElementById('guest-prompt-overlay');
    const proceedButton = document.getElementById('proceed-btn');
    const goBackButton = document.getElementById('go-back-btn');
    const guestCountContainer = document.getElementById('guest-count-container');
    const checkoutBtn = document.getElementById('checkout-btn');
    const movieTitleHeader = document.getElementById('movie-title-header');
    const selectionInfoHeader = document.getElementById('selection-info-header');

    let numberOfGuests = 1;
    let selectedSeats = [];

    // --- Dark Mode Toggle Logic ---
    const themeToggle = document.getElementById('theme-toggle');
    const body = document.body;

    // Function to apply the theme
    const applyTheme = (theme) => {
        if (theme === 'night') {
            body.classList.remove('day-mode');
            body.classList.add('night-mode');
        } else {
            body.classList.remove('night-mode');
            body.classList.add('day-mode');
        }
    };

    // 1. Check for a saved theme in localStorage when the page loads
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
        applyTheme(savedTheme);
    } else {
        applyTheme('day'); // Default to day mode if nothing is saved
    }

    // 2. Add the event listener for the new toggle switch
    if (themeToggle) {
        themeToggle.addEventListener('click', () => {
            // Check which theme is currently active and switch to the other
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
    const initializePage = () => {
        if (!movieTitle || !showtimeId || !auditoriumId || !time || !date) {
            document.body.innerHTML = "<h1>Error: Missing details in URL. Please go back and select a showtime.</h1>";
            return;
        }

        // Format the date for display (e.g., "August 22")
        const displayDate = new Date(date + 'T00:00:00').toLocaleDateString('en-US', {
            month: 'long',
            day: 'numeric'
        });

        movieTitleHeader.textContent = movieTitle;
        selectionInfoHeader.textContent = `Date: ${displayDate} | Time: ${time}`;

        for (let i = 1; i <= 8; i++) {
            const button = document.createElement('button');
            button.classList.add('guest-count-btn');
            button.dataset.guests = i;
            button.textContent = i;
            if (i === 1) button.classList.add('selected');
            guestCountContainer.appendChild(button);
        }

        // Remove the link from the logo and the check mark for the theme switch
        const headerLogo = document.querySelector('.header-logo .logo-text');
        if (headerLogo) {
            headerLogo.removeAttribute('href');
        }

        const themeSwitchLabel = document.querySelector('.theme-switch-wrapper label');
        if (themeSwitchLabel) {
            themeSwitchLabel.style.display = 'none';
        }
    };

    // --- Event Listeners ---
    proceedButton.addEventListener('click', () => {
        guestPromptOverlay.classList.add('hidden');
        generateLayout();
    });
    goBackButton.addEventListener('click', () => window.history.back());
    guestCountContainer.addEventListener('click', (e) => {
        if (e.target.classList.contains('guest-count-btn')) {
            guestCountContainer.querySelector('.selected').classList.remove('selected');
            e.target.classList.add('selected');
            numberOfGuests = parseInt(e.target.dataset.guests);
        }
    });

    // === REWRITTEN LAYOUT GENERATION LOGIC ===
    const generateLayout = async () => {
        const layouts = {
            1: { sections: [10, 10], premium_rows: 2, total_rows: 8 },
            2: { sections: [8, 12, 8], premium_rows: 1, total_rows: 10 },
            3: { sections: [20], premium_rows: 1, total_rows: 9 }
        };
        const auditoriumLayout = layouts[auditoriumId] || layouts[1];
        const occupiedSeats = await fetchOccupiedSeats();
        theaterContainer.innerHTML = '';

        // Create all row containers first
        const rowElements = [];
        for (let r = 0; r < auditoriumLayout.total_rows; r++) {
            const rowDiv = document.createElement('div');
            rowDiv.classList.add('seat-row');
            rowElements.push(rowDiv);
        }

        // Populate each row with seats from all sections
        let totalSeatsSoFar = 0;
        auditoriumLayout.sections.forEach((sectionSeatCount, sectionIndex) => {
            const seatBlock = document.createElement('div');
            seatBlock.classList.add('seat-block');

            for (let r = 0; r < auditoriumLayout.total_rows; r++) {
                const rowLetter = String.fromCharCode(65 + r);
                const rowDiv = document.createElement('div'); // A temporary container for this section's seats
                rowDiv.classList.add('seat-row-segment');

                // Add row letter ONLY to the first block
                if (sectionIndex === 0) {
                    const letterDiv = document.createElement('div');
                    letterDiv.classList.add('row-letter');
                    letterDiv.textContent = rowLetter;
                    rowElements[r].appendChild(letterDiv);
                }

                for (let c = 1; c <= sectionSeatCount; c++) {
                    const seatNumber = c + totalSeatsSoFar;
                    const seatId = `${rowLetter}${seatNumber}`;
                    const seatDiv = document.createElement('div');
                    seatDiv.classList.add('seat');
                    seatDiv.classList.add(r < auditoriumLayout.premium_rows ? 'premium' : 'normal');
                    seatDiv.dataset.seatId = seatId;
                    seatDiv.dataset.row = rowLetter;
                    seatDiv.dataset.col = seatNumber;

                    if (occupiedSeats.includes(seatId)) {
                        seatDiv.classList.add('occupied');
                    }
                    rowDiv.appendChild(seatDiv);
                }
                seatBlock.appendChild(rowDiv);
            }
            theaterContainer.appendChild(seatBlock);

            // Add passage if not the last section
            if (sectionIndex < auditoriumLayout.sections.length - 1) {
                const passageDiv = document.createElement('div');
                passageDiv.classList.add('passage');
                theaterContainer.appendChild(passageDiv);
            }
            totalSeatsSoFar += sectionSeatCount;
        });
    };

    const fetchOccupiedSeats = async () => {
        try {
            const response = await fetch(`${serverUrl}/occupied-seats?showtime_id=${showtimeId}`);
            if (!response.ok) return [];
            return await response.json();
        } catch (error) {
            console.error("Could not fetch occupied seats:", error);
            return [];
        }
    };

    // --- RE-INTEGRATED: Advanced Seat Selection Logic ---
    theaterContainer.addEventListener('click', (e) => {
        const clickedSeat = e.target;
        if (!clickedSeat.classList.contains('seat') || clickedSeat.classList.contains('occupied')) {
            return;
        }

        document.querySelectorAll('.seat.selected').forEach(s => s.classList.remove('selected'));
        let finalSelection = [clickedSeat];
        
        // Stage 1: Search Right
        let current = clickedSeat;
        while (finalSelection.length < numberOfGuests) {
            const nextEl = current.nextElementSibling;
            if (!nextEl || nextEl.classList.contains('occupied') || nextEl.classList.contains('passage')) break;
            finalSelection.push(nextEl);
            current = nextEl;
        }

        // Stage 2: Continue Search Left
        if (finalSelection.length < numberOfGuests) {
            current = clickedSeat;
            while (finalSelection.length < numberOfGuests) {
                const prevEl = current.previousElementSibling;
                if (!prevEl || prevEl.classList.contains('occupied') || prevEl.classList.contains('passage')) break;
                finalSelection.unshift(prevEl);
                current = prevEl;
            }
        }

        // Final Check
        if (finalSelection.length === numberOfGuests) {
            selectedSeats = finalSelection;
            selectedSeats.forEach(seat => seat.classList.add('selected'));
            checkoutBtn.classList.add('visible');
        } else {
            selectedSeats = [];
            // Only select the single clicked seat if a group can't be found
            clickedSeat.classList.add('selected');
            if (numberOfGuests > 1) {
                alert('Not enough adjacent seats available from this point.');
                checkoutBtn.classList.remove('visible');
            } else {
                checkoutBtn.classList.add('visible');
            }
        }
    });

    initializePage();
});