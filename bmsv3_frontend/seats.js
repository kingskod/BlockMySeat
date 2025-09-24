document.addEventListener('DOMContentLoaded', () => {
    const serverUrl = 'http://127.0.0.1:18080';

    // --- Get data from URL ---
    const urlParams = new URLSearchParams(window.location.search);
    const movieTitle = decodeURIComponent(urlParams.get('movie'));
    const showtimeId = urlParams.get('showtime_id');
    const auditoriumId = urlParams.get('auditorium_id');
    const time = urlParams.get('time');
    const date = urlParams.get('date'); // Ensure date is retrieved for confirmation page
    const venueName = decodeURIComponent(urlParams.get('venue') || 'Unknown Venue'); // Retrieve venue name
    const posterUrl = decodeURIComponent(urlParams.get('poster') || ''); // Retrieve poster URL

    // --- DOM Elements ---
    const theaterContainer = document.getElementById('theater-container');
    const guestPromptOverlay = document.getElementById('guest-prompt-overlay');
    const proceedButton = document.getElementById('proceed-btn');
    const goBackButton = document.getElementById('go-back-btn');
    const guestCountContainer = document.getElementById('guest-count-container');
    const checkoutBtn = document.getElementById('checkout-btn'); // Your button in question

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
        if (!movieTitle || !showtimeId || !auditoriumId || !time || !date || !venueName || !posterUrl) {
            document.body.innerHTML = "<h1>Error: Missing details in URL. Please go back and select a showtime.</h1>";
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
        // Make sure checkout button is hidden initially
        checkoutBtn.classList.remove('visible');
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
            // Re-evaluate seat selection if number of guests changes
            // This clears any previous selection and hides checkout button
            document.querySelectorAll('.seat.selected').forEach(s => s.classList.remove('selected'));
            selectedSeats = [];
            checkoutBtn.classList.remove('visible');
        }
    });

    checkoutBtn.addEventListener('click', () => {
        if (selectedSeats.length === 0) {
            alert('Please select your seats.');
            return;
        }

        const seatIds = selectedSeats.map(seat => seat.dataset.seatId).join(',');
        
        const params = new URLSearchParams({
            movie: movieTitle,
            venue: venueName, // Pass venue name
            poster: posterUrl, // Pass movie poster URL
            showtime_id: showtimeId,
            auditorium_id: auditoriumId,
            date: date, // Pass the date parameter
            time: time,
            seats: seatIds
        });
        window.location.href = `confirmation.html?${params.toString()}`;
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

            // 2. Create the row containers with letters and actual seat rows
            const theaterBlocks = []; // To hold seat-block elements

            let totalSeatsSoFar = 0;
            layoutDetails.sections.forEach((sectionSeatCount, sectionIndex) => {
                const seatBlock = document.createElement('div');
                seatBlock.classList.add('seat-block');

                // For each row within this section
                for (let r = 0; r < totalRows; r++) {
                    const rowLetter = String.fromCharCode(65 + r);
                    const seatRowDiv = document.createElement('div'); // This represents a full row, including letter and segments
                    seatRowDiv.classList.add('seat-row');

                    const letterDiv = document.createElement('div');
                    letterDiv.classList.add('row-letter');
                    letterDiv.textContent = rowLetter;
                    seatRowDiv.appendChild(letterDiv);

                    const rowSegment = document.createElement('div'); // This holds the actual seats for this section in the current row
                    rowSegment.classList.add('seat-row-segment');
                    rowSegment.dataset.rowLetter = rowLetter; // Store row letter on the segment for easier seat finding

                    for (let c = 1; c <= sectionSeatCount; c++) {
                        // Global seat number across all sections in that row
                        const seatNumberInFullRow = totalSeatsSoFar + c; 
                        const seatId = `${rowLetter}${seatNumberInFullRow}`;

                        const seatDiv = document.createElement('div');
                        seatDiv.classList.add('seat');
                        seatDiv.classList.add(r < layoutDetails.premium_rows ? 'premium' : 'normal');
                        seatDiv.dataset.seatId = seatId;
                        seatDiv.dataset.row = rowLetter;
                        seatDiv.dataset.col = seatNumberInFullRow; // Use global seat number

                        if (occupiedSeats.includes(seatId)) {
                            seatDiv.classList.add('occupied');
                        }
                        rowSegment.appendChild(seatDiv);
                    }
                    seatRowDiv.appendChild(rowSegment); // Add the segment to the full row div
                    seatBlock.appendChild(seatRowDiv); // Add the full row to the seat block
                }
                theaterBlocks.push(seatBlock);

                // Add passage if not the last section
                if (sectionIndex < layoutDetails.sections.length - 1) {
                    const passageDiv = document.createElement('div');
                    passageDiv.classList.add('passage');
                    theaterBlocks.push(passageDiv); // Add passage to the list of blocks
                }
                totalSeatsSoFar += sectionSeatCount; // Increment for the next section's starting seat number
            });

            // Append all constructed blocks to the theater container
            theaterBlocks.forEach(block => theaterContainer.appendChild(block));

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

        // Clear existing selections before making new ones
        document.querySelectorAll('.seat.selected').forEach(s => s.classList.remove('selected'));
        selectedSeats = []; // Reset selected seats array
        checkoutBtn.classList.remove('visible'); // Hide button until a valid selection is made

        let finalSelection = [];
        const clickedRowLetter = clickedSeat.dataset.row;
        const clickedCol = parseInt(clickedSeat.dataset.col);

        // Find all available seats in the clicked seat's row within its segment
        const seatsInClickedSegment = Array.from(clickedSeat.closest('.seat-row-segment').children)
                                         .filter(seat => !seat.classList.contains('occupied'));
        
        // Convert to an array of seat elements for easier indexing
        const availableSeatsInRowSegment = seatsInClickedSegment;
        const clickedSeatIndexInSegment = availableSeatsInRowSegment.indexOf(clickedSeat);

        if (clickedSeatIndexInSegment === -1) { // Should not happen if clickedSeat is valid and not occupied
            return;
        }

        // Attempt to find `numberOfGuests` adjacent seats starting from the clicked seat
        for (let i = 0; i < availableSeatsInRowSegment.length - numberOfGuests + 1; i++) {
            let potentialGroup = [];
            let isAdjacent = true;
            for (let j = 0; j < numberOfGuests; j++) {
                const candidate = availableSeatsInRowSegment[i + j];
                if (!candidate) { // Not enough seats in the row segment
                    isAdjacent = false;
                    break;
                }
                potentialGroup.push(candidate);
                // Check if the current candidate is truly adjacent to the previous one in the group (by column number)
                if (j > 0 && parseInt(candidate.dataset.col) !== parseInt(potentialGroup[j-1].dataset.col) + 1) {
                     isAdjacent = false;
                     break;
                }
            }
            if (isAdjacent && potentialGroup.length === numberOfGuests) {
                // If the clicked seat is part of this potential group, select this group
                if (potentialGroup.includes(clickedSeat)) {
                    finalSelection = potentialGroup;
                    break; 
                }
            }
        }


        // If a valid contiguous group was found including the clicked seat
        if (finalSelection.length === numberOfGuests) {
            selectedSeats = finalSelection;
            selectedSeats.forEach(seat => seat.classList.add('selected'));
            checkoutBtn.classList.add('visible');
        } else {
            selectedSeats = []; 
            // If only one guest, allow single seat selection even if a group isn't found
            if (numberOfGuests === 1) {
                clickedSeat.classList.add('selected');
                selectedSeats.push(clickedSeat);
                checkoutBtn.classList.add('visible');
            } else {
                alert(`Could not find ${numberOfGuests} adjacent seats from your selection. Please try another seat or reduce the number of guests.`);
                checkoutBtn.classList.remove('visible');
            }
        }
    });

    initializePage();
});