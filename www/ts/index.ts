import $ from 'jquery';

import { init, use } from 'echarts/core';
import { BarChart } from 'echarts/charts';
import { TitleComponent, TooltipComponent, GridComponent, DatasetComponent } from 'echarts/components';
import { CanvasRenderer } from 'echarts/renderers';

use([TitleComponent, TooltipComponent, GridComponent, DatasetComponent, BarChart, CanvasRenderer]);

$(() => {
	const chart = init(document.getElementById('main') as HTMLDivElement);

	chart.setOption({
		title: {
			text: "ECharts entry example",
		},
		tooltip: {},
		xAxis: {
			data: ["shirt", "cardign", "chiffon shirt", "pants", "heels", "socks"],
		},
		yAxis: {},
		series: [
			{
				name: "Sales",
				type: "bar",
				data: [5, 20, 36, 10, 10, 20],
			},
		],
	});
})


function formatDate(date: number): string {
	const today: Date = new Date();
	const givenDate: Date = new Date(date);
	const day: number = givenDate.getDate();
	const month: number = givenDate.getMonth();
	const year: number = givenDate.getFullYear();
	const hour: number = givenDate.getHours();
	const minute: number = givenDate.getMinutes();

	let dateString: string = ``;

	if (year !== today.getFullYear()) dateString = `${day}/${month + 1}/${year}`;

	else dateString = `${hour}:${minute}`;

	return ``;
}