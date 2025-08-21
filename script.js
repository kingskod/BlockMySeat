document.addEventListener('DOMContentLoaded', () => {
    // DOM Elements - The 'form' constant is now correctly defined here.
    const body = document.body;
    const form = document.getElementById('auth-form'); // This was the crucial line.
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
        errorMessage.textContent = ''; // Clear errors on mode switch

        if (currentMode === 'signin') {
            body.className = 'signin-mode';
            
            // Configure UI
            signInToggle.classList.add('active');
            signUpToggle.classList.remove('active');
            submitButton.textContent = 'Sign In';

            // Show/Hide Fields
            emailGroup.style.display = 'none';
            confirmPasswordGroup.style.display = 'none';
            rememberMeGroup.style.display = 'flex';
            
        } else { // signup mode
            body.className = 'signup-mode';

            // Configure UI
            signUpToggle.classList.add('active');
            signInToggle.classList.remove('active');
            submitButton.textContent = 'Create Account';

            // Show/Hide Fields
            emailGroup.style.display = 'block';
            confirmPasswordGroup.style.display = 'block';
            rememberMeGroup.style.display = 'none';
        }
    };

    // --- Event Listeners ---
    signInToggle.addEventListener('click', () => {
        window.location.href = 'index.html';
    });

    signUpToggle.addEventListener('click', () => {
        window.location.href = 'index.html?mode=signup';
    });

    form.addEventListener('submit', (event) => {
        event.preventDefault(); // Prevent actual form submission
        errorMessage.textContent = ''; // Clear previous errors

        if (mode === 'signup') {
            // --- Sign-Up Validation ---
            const email = emailInput.value;
            const password = passwordInput.value;
            const confirmPassword = confirmPasswordInput.value;

            if (!email || !password || !confirmPassword) {
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

            // If validation passes
            alert('Account created successfully! (This is a demo)');
            
        } else {
            // --- Sign-In Logic (Placeholder) ---
            alert('Sign-in attempt! (This is a demo)');
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