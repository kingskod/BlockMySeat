document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get data from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const movieTitle = decodeURIComponent(urlParams.get('movie'));
    const showtimeId = urlParams.get('showtime_id');
    const auditoriumId = urlParams.get('auditorium_id');
    const time = urlParams.get('time');

    // --- DOM Elements ---
    const theaterContainer = document.getElementById('theater-container');
    const guestPromptOverlay = document.getElementById('guest-prompt-overlay');
    const proceedButton = document.getElementById('proceed-btn');
    const goBackButton = document.getElementById('go-back-btn');
    const guestCountContainer = document.getElementById('guest-count-container');
    const checkoutBtn = document.getElementById('checkout-btn');

    let numberOfGuests = 1;
    let selectedSeats = [];

    // --- Dark Mode Logic ---
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

    // Check for saved theme in localStorage
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
        applyTheme(savedTheme);
    }

    // Add event listener to the theme toggle
    if (themeToggle) {
        themeToggle.addEventListener('click', () => {
            const currentTheme = body.classList.contains('night-mode') ? 'night' : 'day';
            const newTheme = currentTheme === 'night' ? 'day' : 'night';
            localStorage.setItem('theme', newTheme);
            applyTheme(newTheme);
        });
    }

    // --- Main Initialization ---
    const initializePage = () => {
        if (!movieTitle || !showtimeId || !auditoriumId || !time) {
            document.body.innerHTML = "<h1>Error: Missing details in URL.</h1>";
            return;
        }
        document.getElementById('movie-title-header').textContent = movieTitle;
        document.getElementById('selection-info-header').textContent = `Time: ${time}`;
        for (let i = 1; i <= 8; i++) {
            const button = document.createElement('button');
            button.classList.add('guest-count-btn');
            button.dataset.guests = i;
            button.textContent = i;
            if (i === 1) button.classList.add('selected');
            guestCountContainer.appendChild(button);
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

    // === MODIFIED: LAYOUT GENERATION LOGIC ===
    const generateLayout = async () => {
        try {
            // 1. Fetch all necessary data from the backend
            const [layoutDetails, occupiedSeats] = await Promise.all([
                fetchAuditoriumLayout(),
                fetchOccupiedSeats()
            ]);

            if (!layoutDetails) {
                theaterContainer.innerHTML = "<p>Could not load auditorium layout.</p>";
                return;
            }

            theaterContainer.innerHTML = ''; // Clear loading message
            const totalRows = layoutDetails.premium_rows + layoutDetails.normal_rows;

            // 2. Create the row containers with letters
            const rowElements = [];
            for (let r = 0; r < totalRows; r++) {
                const rowDiv = document.createElement('div');
                rowDiv.classList.add('seat-row');
                const letterDiv = document.createElement('div');
                letterDiv.classList.add('row-letter');
                letterDiv.textContent = String.fromCharCode(65 + r); // A, B, C...
                rowDiv.appendChild(letterDiv);
                rowElements.push(rowDiv);
            }

            // 3. Populate rows with seats, section by section
            let totalSeatsSoFar = 0;
            layoutDetails.sections.forEach((sectionSeatCount, sectionIndex) => {
                const seatBlock = document.createElement('div');
                seatBlock.classList.add('seat-block');

                for (let r = 0; r < totalRows; r++) {
                    const rowLetter = String.fromCharCode(65 + r);
                    const rowSegment = document.createElement('div');
                    rowSegment.classList.add('seat-row-segment');

                    for (let c = 1; c <= sectionSeatCount; c++) {
                        const seatNumber = c + totalSeatsSoFar;
                        const seatId = `${rowLetter}${seatNumber}`;
                        const seatDiv = document.createElement('div');
                        seatDiv.classList.add('seat');
                        seatDiv.classList.add(r < layoutDetails.premium_rows ? 'premium' : 'normal');
                        seatDiv.dataset.seatId = seatId;
                        seatDiv.dataset.row = rowLetter;
                        seatDiv.dataset.col = seatNumber;

                        if (occupiedSeats.includes(seatId)) {
                            seatDiv.classList.add('occupied');
                        }
                        rowSegment.appendChild(seatDiv);
                    }
                    seatBlock.appendChild(rowSegment);
                }
                theaterContainer.appendChild(seatBlock);

                // Add passage if not the last section
                if (sectionIndex < layoutDetails.sections.length - 1) {
                    const passageDiv = document.createElement('div');
                    passageDiv.classList.add('passage');
                    theaterContainer.appendChild(passageDiv);
                }
                totalSeatsSoFar += sectionSeatCount;
            });

        } catch (error) {
            console.error("Failed to generate layout:", error);
            theaterContainer.innerHTML = "<p>Error generating seat layout.</p>";
        }
    };

    const fetchAuditoriumLayout = async () => {
        try {
            const response = await fetch(`${serverUrl}/auditorium-details/${auditoriumId}`);
            if (!response.ok) return null;
            const data = await response.json();
            return data.layout; // Return just the layout object
        } catch (error) {
            console.error("Could not fetch auditorium layout:", error);
            return null;
        }
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

    // --- Seat Selection Logic  ---
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