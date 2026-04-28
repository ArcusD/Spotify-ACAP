document.addEventListener('DOMContentLoaded', () => {
    // UI Elements
    const form = document.getElementById('settingsForm');
    const deviceNameInput = document.getElementById('DeviceName');
    const bitrateSelect = document.getElementById('Bitrate');
    const saveBtn = document.getElementById('saveBtn');
    const saveMessage = document.getElementById('saveMessage');
    const btnText = saveBtn.querySelector('.btn-text');
    const loader = saveBtn.querySelector('.loader');
    
    // Player UI Elements
    const serviceStatus = document.getElementById('serviceStatus');
    const playbackState = document.getElementById('playbackState');
    const trackDetails = document.getElementById('trackDetails');
    const albumArt = document.getElementById('albumArt');
    const visualizer = document.getElementById('visualizer');

    // Create visualizer bars
    for(let i = 0; i < 40; i++) {
        const bar = document.createElement('div');
        bar.className = 'bar';
        bar.style.animationDuration = `${0.5 + Math.random()}s`;
        bar.style.animationDelay = `${Math.random()}s`;
        visualizer.appendChild(bar);
    }

    // --- Configuration (VAPIX) ---

    // Fetch current settings
    async function fetchSettings() {
        try {
            const response = await fetch('/axis-cgi/param.cgi?action=list&group=spotifyconnect');
            const text = await response.text();
            
            // Parse VAPIX plain text response: spotifyconnect.DeviceName=Speaker
            const lines = text.split('\n');
            lines.forEach(line => {
                if(line.includes('=')) {
                    const parts = line.split('=');
                    const key = parts[0].split('.')[1]; // get 'DeviceName' from 'root.spotifyconnect.DeviceName'
                    const value = parts[1].trim();
                    
                    if(key === 'DeviceName') deviceNameInput.value = value;
                    if(key === 'Bitrate') bitrateSelect.value = value;
                }
            });
            
            setServiceStatus('active', 'Service Active');
        } catch (err) {
            console.error('Failed to load settings:', err);
            setServiceStatus('error', 'Camera Offline');
        }
    }

    // Save settings
    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        // Show loading state
        btnText.classList.add('hidden');
        loader.classList.remove('hidden');
        saveBtn.disabled = true;
        saveMessage.className = 'save-message';
        saveMessage.textContent = '';

        const name = encodeURIComponent(deviceNameInput.value);
        const bitrate = encodeURIComponent(bitrateSelect.value);

        try {
            // Update via VAPIX
            const url = `/axis-cgi/param.cgi?action=update&spotifyconnect.DeviceName=${name}&spotifyconnect.Bitrate=${bitrate}`;
            const response = await fetch(url);
            
            if(response.ok) {
                showMessage('Settings saved! Restarting app...', 'success');
                // Restarting the app to apply changes (optional: usually Axis requires an app restart for param changes to take effect on the daemon)
                await fetch('/axis-cgi/applications/control.cgi?action=restart&package=spotifyconnect');
            } else {
                throw new Error('Save failed');
            }
        } catch (err) {
            console.error('Failed to save settings:', err);
            showMessage('Failed to save settings', 'error');
        } finally {
            // Reset button
            btnText.classList.remove('hidden');
            loader.classList.add('hidden');
            saveBtn.disabled = false;
        }
    });

    function showMessage(msg, type) {
        saveMessage.textContent = msg;
        saveMessage.className = `save-message ${type}`;
        setTimeout(() => {
            saveMessage.style.opacity = '0';
        }, 5000);
        setTimeout(() => {
            saveMessage.className = 'save-message';
            saveMessage.style.opacity = '1';
            saveMessage.textContent = '';
        }, 5300);
    }

    function setServiceStatus(type, text) {
        serviceStatus.className = `status-badge ${type}`;
        serviceStatus.querySelector('.text').textContent = text;
    }

    // --- Playback State Polling ---

    let currentEvent = 'stopped';

    async function pollState() {
        try {
            const response = await fetch('/local/spotifyconnect/state.cgi?t=' + Date.now());
            if(response.ok) {
                const data = await response.json();
                updatePlayerUI(data);
            }
        } catch (err) {
            // Silently fail, it might be restarting or the file doesn't exist yet
        }
    }

    function updatePlayerUI(data) {
        if(data.event === currentEvent && !data.track_id) return;
        currentEvent = data.event;

        if(currentEvent === 'playing' || currentEvent === 'track_changed') {
            albumArt.classList.add('playing');
            visualizer.classList.add('active');
            playbackState.textContent = 'Now Playing';
            trackDetails.textContent = 'Streaming audio from Spotify';
        } 
        else if (currentEvent === 'paused') {
            albumArt.classList.add('playing');
            visualizer.classList.remove('active');
            playbackState.textContent = 'Paused';
            trackDetails.textContent = 'Ready to resume';
        }
        else {
            albumArt.classList.remove('playing');
            visualizer.classList.remove('active');
            playbackState.textContent = 'Ready to play';
            trackDetails.textContent = 'Select this camera in your Spotify app';
        }
    }

    // Initialize
    fetchSettings();
    setInterval(pollState, 2000); // Poll state every 2 seconds
});
