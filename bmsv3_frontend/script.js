document.addEventListener('DOMContentLoaded', () => {
    // DOM Elements
    const body = document.body;
    const form = document.getElementById('auth-form');
    const submitButton = document.getElementById('submit-button');
    const errorMessage = document.getElementById('error-message');

    // Toggle Buttons
    const signInToggle = document.getElementById('signin-toggle');
    const signUpToggle = document.getElementById('signup-toggle');

    // Form Field Groups
    const emailGroup = document.getElementById('email-group');
    const confirmPasswordGroup = document.getElementById('confirm-password-group');
    const rememberMeGroup = document.getElementById('remember-me-group');
    
    // Input Fields
    const emailInput = document.getElementById('email');
    const passwordInput = document.getElementById('password');
    const confirmPasswordInput = document.getElementById('confirm-password');

    // --- Mode Detection and Setup ---
    const urlParams = new URLSearchParams(window.location.search);
    const mode = urlParams.get('mode') === 'signup' ? 'signup' : 'signin';

    const setupMode = (currentMode) => {
        errorMessage.textContent = ''; 

        if (currentMode === 'signin') {
            body.className = 'signin-mode';
            signInToggle.classList.add('active');
            signUpToggle.classList.remove('active');
            submitButton.textContent = 'Sign In';
            emailGroup.style.display = 'none';
            confirmPasswordGroup.style.display = 'none';
            rememberMeGroup.style.display = 'flex';
        } else { 
            body.className = 'signup-mode';
            signUpToggle.classList.add('active');
            signInToggle.classList.remove('active');
            submitButton.textContent = 'Create Account';
            emailGroup.style.display = 'block';
            confirmPasswordGroup.style.display = 'block';
            rememberMeGroup.style.display = 'none';
        }
    };

    // --- Event Listeners ---
    signInToggle.addEventListener('click', () => {
        if (mode !== 'signin') {
            window.location.href = 'index.html';
        }
    });

    signUpToggle.addEventListener('click', () => {
        if (mode !== 'signup') {
            window.location.href = 'index.html?mode=signup';
        }
    });

    // === NEW: GUEST LOGIN LOGIC ===
    const guestMoviesLink = document.getElementById('guest-movies-link');
    if (guestMoviesLink) {
        guestMoviesLink.addEventListener('click', (event) => {
            event.preventDefault(); // Stop the link from just adding '#' to the URL
            // A "guest" login is just a direct redirect to the movies page.
            window.location.href = 'movies.html';
        });
    }
    // === NEW: GUEST VENUES LOGIC=== 
    const guestVenuesLink = document.getElementById('guest-venues-link');
    if (guestVenuesLink) {
        guestVenuesLink.addEventListener('click', (event) => {
            event.preventDefault();
            // Redirect directly to the venues page.
            window.location.href = 'venues.html';
        });
    }
    // === END OF NEW LOGIC ===

    form.addEventListener('submit', (event) => {
        event.preventDefault(); 
        errorMessage.textContent = ''; 
        const serverUrl = 'http://127.0.0.1:18080';

        if (mode === 'signup') {
            const username = document.getElementById('username').value;
            const email = emailInput.value;
            const password = passwordInput.value;
            const confirmPassword = confirmPasswordInput.value;

            if (!username || !email || !password || !confirmPassword) {
                errorMessage.textContent = 'Please fill in all fields.';
                return;
            }
            if (!validateEmail(email)) {
                errorMessage.textContent = 'Please enter a valid email address.';
                return;
            }
            if (password.length < 8) {
                errorMessage.textContent = 'Password must be at least 8 characters long.';
                return;
            }
            if (password !== confirmPassword) {
                errorMessage.textContent = 'Passwords do not match.';
                return;
            }

            fetch(serverUrl + '/signup', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, email, password })
            })
            .then(response => response.json().then(data => ({ ok: response.ok, data })))
            .then(({ ok, data }) => {
                if (ok) {
                    alert('Account created! Please sign in.');
                    window.location.href = 'index.html'; 
                } else {
                    errorMessage.textContent = data.message;
                }
            })
            .catch(error => {
                console.error('Error:', error);
                errorMessage.textContent = 'Could not connect to the server.';
            });
            
        } else { // Sign-In Logic
            const username = document.getElementById('username').value;
            const password = passwordInput.value;

            if (!username || !password) {
                errorMessage.textContent = 'Please enter a username and password.';
                return;
            }

            fetch(serverUrl + '/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            })
            .then(response => response.json().then(data => ({ ok: response.ok, data })))
            .then(({ ok, data }) => {
                if (ok) {
                    // === MODIFICATION: REDIRECT ON SUCCESSFUL LOGIN ===
                    window.location.href = 'movies.html'; 
                } else {
                    errorMessage.textContent = data.message;
                }
            })
            .catch(error => {
                console.error('Error:', error);
                errorMessage.textContent = 'Could not connect to the server.';
            });
        }
    });
    
    // --- Utility Functions ---
    const validateEmail = (email) => {
        const re = /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
        return re.test(String(email).toLowerCase());
    };

    // --- Initial Setup ---
    setupMode(mode);
});