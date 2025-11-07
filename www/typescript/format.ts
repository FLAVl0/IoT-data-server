/**
 * Formats a date into a human-readable string. (Unused function, was meant for chart tooltip)
 * @param date The date to format (timestamp in milliseconds).
 * @returns The formatted date string.
 */
function formatDate(date: number): string {
	const today = new Date();
	const givenDate = new Date(date);

	if (givenDate.getFullYear() !== today.getFullYear()) {
		return `${givenDate.getDate()}/${givenDate.getMonth() + 1}/${givenDate.getFullYear()}`;
	}

	const hour = String(givenDate.getHours()).padStart(2, '0');
	const minute = String(givenDate.getMinutes()).padStart(2, '0');
	return `${hour}:${minute}`;
}
