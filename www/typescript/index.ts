// Import ECharts core and required modules
import { init, use } from 'echarts/core';
import { LineChart } from 'echarts/charts';
import { TitleComponent, TooltipComponent, GridComponent, DatasetComponent } from 'echarts/components';
import { CanvasRenderer } from 'echarts/renderers';

// Register ECharts components
use([TitleComponent, TooltipComponent, GridComponent, DatasetComponent, LineChart, CanvasRenderer]);

// Interface for LED API response
interface LedResponse {
	success: boolean;
	message?: string;
}

/**
 * Sends a request to set the LED state and updates the UI accordingly.
 * @param state - Desired LED state ('on' or 'off')
 * @param onButton - Reference to the ON button
 * @param offButton - Reference to the OFF button
 * @param statusDiv - Reference to the status display div
 */
function setLedState(state: 'on' | 'off', onButton: HTMLButtonElement, offButton: HTMLButtonElement, statusDiv: HTMLDivElement) {
	onButton.disabled = true;
	offButton.disabled = true;
	statusDiv.textContent = `Turning LED ${state.toUpperCase()}...`;

	fetch('/api/led', {
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify({ state })
	})
		.then(response => response.json())
		.then((response: LedResponse) => {
			// Update status based on API response
			statusDiv.textContent = response.success
				? `LED is now ${state.toUpperCase()}.` && (statusDiv.style.color = 'green')
				: `Failed to turn LED ${state.toUpperCase()}: ${response.message || 'Unknown error'}` && (statusDiv.style.color = 'red');
		})
		.catch(error => {
			// Handle network or server errors
			statusDiv.style.color = 'red';
			statusDiv.textContent = `Error communicating with server: ${error.message}`;
		})
		.finally(() => {
			onButton.disabled = false;
			offButton.disabled = false;
		});
}

// DOMContentLoaded event handler to initialize chart and button events
document.addEventListener('DOMContentLoaded', () => {
	// Initialize ECharts instance on the chart container
	const chart = init(document.getElementById('main') as HTMLDivElement);

	// Get references to LED control buttons
	const onButton = document.getElementById('onButton') as HTMLButtonElement;
	const offButton = document.getElementById('offButton') as HTMLButtonElement;

	// Get reference to LED status display
	let statusDiv = document.getElementById('ledStatus') as HTMLDivElement;
	if (!statusDiv) {
		// Fallback: create status div if not found
		statusDiv = document.createElement('div');
		statusDiv.id = 'ledStatus';
		document.parentElement?.appendChild(statusDiv);
	}

	// Attach event listeners to LED control buttons
	onButton.addEventListener('click', () => setLedState('on', onButton, offButton, statusDiv));
	offButton.addEventListener('click', () => setLedState('off', onButton, offButton, statusDiv));

	// Generate simulated hourly temperature data
	const temps = Array.from({ length: 24 }, () => Math.floor(Math.random() * 15) + 15); // Simulated hourly temps

	// Configure and render the ECharts line chart
	chart.setOption({
		title: { text: "Temperature Monitoring (Sample Data)" },
		xAxis: {
			type: "category",
			data: Array.from({ length: 24 }, (_, i) => `${i}:00`)
		},
		yAxis: {
			type: "value",
			name: "Temperature (°C)"
		},
		tooltip: {
			trigger: "axis"
		},
		series: [{
			name: "Temperature",
			type: "line",
			data: temps,
			smooth: true,
			lineStyle: { width: 2 },
			itemStyle: { color: "#5470C6" }
		}]
	});
});
