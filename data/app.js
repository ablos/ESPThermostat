// Temperature settings
let MIN_TEMP = 10;
let MAX_TEMP = 35;
let currentTargetTemp = 22;
let currentEcoTemp = 16;
let currentTemp = null;
let currentMode = 'eco'; // 'off', 'eco', 'on'

// DOM elements
const dialHandle = document.getElementById('dialHandle');
const dialProgress = document.getElementById('dialProgress');
const targetTempMainDisplay = document.getElementById('targetTempMain');
const targetTempDecimalDisplay = document.getElementById('targetTempDecimal');
const currentTempCenterDisplay = document.getElementById('currentTempCenter');
const modeButtons = document.querySelectorAll('.mode-button');
const humidityPointer = document.getElementById('humidityPointer');
const humidityValue = document.getElementById('humidityValue');
const flameIcon = document.getElementById('flameIcon');

// Dial geometry
const dialContainer = document.querySelector('.dial-container');
const centerX = 150;
const centerY = 150;
const radius = 130;
const circumference = 2 * Math.PI * radius * (270 / 360); // 270 degrees of the full circle

let isDragging = false;
let lastUserInteraction = 0;

// Initialize
updateDialForMode();
updateModeButtons();
loadStatus();

// Load status from API
async function loadStatus()
{
    try
    {
        const response = await fetch('/api/status');
        const data = await response.json();

        if (data.status === 'ok')
        {
            // Skip updating mode/temp if user just interacted (within 2 seconds)
            const timeSinceInteraction = Date.now() - lastUserInteraction;
            const skipModeUpdate = timeSinceInteraction < 2000;

            if (!skipModeUpdate)
            {
                currentTargetTemp = data.targetTemp || 22;
                currentEcoTemp = data.ecoTemp || 16;
                currentMode = data.mode || 'eco';
            }

            MIN_TEMP = data.minTemp || 10;
            MAX_TEMP = data.maxTemp || 35;

            if (data.currentTemp)
            {
                currentTemp = data.currentTemp;
                const currentTempValue = data.currentTemp.toFixed(1);
                currentTempCenterDisplay.textContent = currentTempValue + "°C";
            }
            else
            {
                currentTemp = null;
                currentTempCenterDisplay.textContent = "--";
            }

            if (data.humidity)
                updateHumidity(data.humidity);

            // Update heater status indicator (only show in eco/on mode)
            if (data.heaterActive && (currentMode === 'eco' || currentMode === 'on'))
                flameIcon.classList.add('active');
            else
                flameIcon.classList.remove('active');

            if (!skipModeUpdate)
            {
                updateDialForMode();
                updateModeButtons();
            }
        }
    }
    catch (error)
    {
        console.error('Failed to load status:', error);
    }
}

// Update temperature on server
async function updateTemperature(temp) 
{
    try 
    {
        const response = await fetch('/api/target/set', 
        {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ targetTemp: temp })
        });
        
        const data = await response.json();
        
        if (data.status === 'ok')
            console.log('Temperature updated successfully');
    }
    catch (error) 
    {
        console.error('Failed to update temperature:', error);
    }
}

// Update dial visuals based on current mode
function updateDialForMode()
{
    if (currentMode === 'off')
    {
        // In off mode: hide slider, show current temp in target temp position, hide center temp
        dialHandle.style.display = 'none';
        dialProgress.style.display = 'none';
        currentTempCenterDisplay.style.display = 'none';

        if (currentTemp !== null)
        {
            const mainTemp = Math.floor(currentTemp);
            const decimal = (currentTemp - mainTemp).toFixed(1).substring(1);
            targetTempMainDisplay.textContent = mainTemp;
            targetTempDecimalDisplay.textContent = decimal;
        }
        else
        {
            targetTempMainDisplay.textContent = '--';
            targetTempDecimalDisplay.textContent = '.-';
        }
    }
    else if (currentMode === 'eco')
    {
        // In eco mode: hide slider, show eco temp
        dialHandle.style.display = 'none';
        dialProgress.style.display = 'none';
        currentTempCenterDisplay.style.display = 'block';
        updateDial(currentEcoTemp);
    }
    else
    {
        // In on mode: show slider, show target temp
        dialHandle.style.display = 'block';
        dialProgress.style.display = 'block';
        currentTempCenterDisplay.style.display = 'block';
        updateDial(currentTargetTemp);
    }
}

// Update dial visuals
function updateDial(temp)
{
    const mainTemp = Math.floor(temp);
    const decimal = (temp - mainTemp).toFixed(1).substring(1); // Get ".X" format

    targetTempMainDisplay.textContent = mainTemp;
    targetTempDecimalDisplay.textContent = decimal;

    // Calculate progress (0-100%)
    const tempRange = MAX_TEMP - MIN_TEMP;
    const progress = (temp - MIN_TEMP) / tempRange;

    // Update progress arc
    const dashOffset = circumference - (progress * circumference);
    dialProgress.style.strokeDashoffset = dashOffset;

    // Update handle position
    // Use 270 degrees total range, starting at 135 degrees (bottom-left)
    const angle = (progress * 270) + 135;
    const angleRad = (angle * Math.PI) / 180;

    // Calculate handle position relative to container center
    const handleX = centerX + radius * Math.cos(angleRad) - 20; // -20 to center the 40px handle
    const handleY = centerY + radius * Math.sin(angleRad) - 20; // -20 to center the 40px handle

    dialHandle.style.left = handleX + 'px';
    dialHandle.style.top = handleY + 'px';
}

// Handle dragging
function startDrag(e)
{
    // Don't allow dragging in eco mode
    if (currentMode === 'eco')
        return;

    isDragging = true;
    e.preventDefault();
}

function drag(e)
{
    if (!isDragging) return;
    e.preventDefault();

    const rect = dialContainer.getBoundingClientRect();
    const touch = e.touches ? e.touches[0] : e;
    const x = touch.clientX - rect.left - centerX;
    const y = touch.clientY - rect.top - centerY;

    // Calculate angle from center
    let angle = Math.atan2(y, x) * (180 / Math.PI);

    // Convert to our coordinate system (0 degrees = bottom-left starting at 135°)
    angle = angle - 135;

    // Handle angle wrapping
    if (angle < 0) angle += 360;
    if (angle > 360) angle -= 360;

    // The valid range is 0-270 degrees in our coordinate system
    // The gap is from 270-360 degrees (bottom area)
    if (angle > 270)
    {
        // If in the gap area, snap to nearest valid end
        if (angle <= 315)
            angle = 270; // Snap to right end (bottom-right)
        else
            angle = 0; // Snap to left end (bottom-left)
    }

    // Ensure we stay within 0-270 range
    angle = Math.max(0, Math.min(270, angle));

    // Convert angle to temperature
    const progress = angle / 270;
    const temp = MIN_TEMP + (progress * (MAX_TEMP - MIN_TEMP));

    // Round to nearest 0.5 degrees
    const roundedTemp = Math.round(temp * 2) / 2;

    currentTargetTemp = roundedTemp;
    updateDial(roundedTemp);
}

function stopDrag()
{
    if (isDragging)
    {
        isDragging = false;
        lastUserInteraction = Date.now();
        updateTemperature(currentTargetTemp);
    }
}

// Update mode on server
async function updateMode(mode)
{
    try
    {
        const response = await fetch('/api/mode/set',
        {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode: mode })
        });

        const data = await response.json();
        
        if (data.status === 'ok')
            console.log('Mode updated successfully');
    }
    catch (error)
    {
        console.error('Failed to update mode:', error);
    }
}

// Mode selection
function selectMode(mode)
{
    currentMode = mode;
    lastUserInteraction = Date.now();
    updateModeButtons();
    updateDialForMode();
    updateMode(mode);
}

function updateModeButtons()
{
    modeButtons.forEach(button =>
    {
        button.classList.remove('active');
        
        if (button.dataset.mode === currentMode)
            button.classList.add('active');
    });
}

// Event listeners
dialHandle.addEventListener('mousedown', startDrag);
dialHandle.addEventListener('touchstart', startDrag);

document.addEventListener('mousemove', drag);
document.addEventListener('touchmove', drag);

document.addEventListener('mouseup', stopDrag);
document.addEventListener('touchend', stopDrag);

modeButtons.forEach(button =>
{
    button.addEventListener('click', () => { selectMode(button.dataset.mode); });
});

// Update humidity indicator
function updateHumidity(humidity)
{
    // Clamp humidity between 0 and 100
    humidity = Math.max(0, Math.min(100, humidity));

    // Update value display
    humidityValue.textContent = humidity.toFixed(0) + '%';

    // Update pointer position (0% = left, 100% = right)
    const pointerPosition = humidity;
    humidityPointer.style.left = `calc(${pointerPosition}% - 2px)`;
}

// Auto-refresh status every 1 second for responsive heater status updates
setInterval(loadStatus, 1000);