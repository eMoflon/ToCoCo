<?php

namespace Runner;

use App\Analyzer\NeighborhoodAnalyzer;
use App\Analyzer\RoutingAnalyzer;
use App\Analyzer\RoutingGraphAnalyzer;
use App\Analyzer\Helper\IgnoredLinksTrait;
use App\Analyzer\Helper\MoteAddressTrait;
use App\Analyzer\Helper\PowercontrolTrait;
use App\Testbed\Flocklab;
use Alom\Graphviz\Digraph;
use Mexitek\PHPColors\Color;
use Fhaculty\Graph\Edge\EdgeUndirected;
use Fhaculty\Graph\Graph;

require_once 'vendor/autoload.php';

ini_set('memory_limit', '2048M');
set_time_limit(-1);
ini_set('display_errors', 1);



//thetaSeparation('/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/serial_24947.csv', 30, /* k=1.2 */ 49.2486367);


//$sentMin = 0; $sentMax = 1000;
//$minuteMin =  5; $minuteMax = 29;
//$minuteMin = 35; $minuteMax = 59;
//routinggraphAccWithTraffic('evaluate-thesis/traffic_on_edges/serial_lmst_24891.csv', $minuteMin, $minuteMax, $sentMin, $sentMax);

//routinggraphWithDroppedEdges('evaluate-thesis/traffic_on_edges/serial_aktc_24807.csv');


/*gnuplot_export(
	'noTC',
	'image_evaluation_hypothese2_metriken_datacollection_notc',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_merge.csv', null, [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23230.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23234.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23301.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23305.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23338.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23373.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23392.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23421.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23502.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_23505.csv',
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_merge.csv', null, [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23230.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23234.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23301.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23305.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23338.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23373.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23392.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23421.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23502.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_23505.csv',
	]
);
gnuplot_export(
	'a-kTC',
	'image_evaluation_hypothese2_metriken_datacollection_aktc',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_24672.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_24760.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_24810.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_24911.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_24947.csv',
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_24672.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_24760.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_24810.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_24911.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_24947.csv',
	]
);
gnuplot_export(
	'l-kTC',
	'image_evaluation_hypothese2_metriken_datacollection_lktc',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_24673.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_24761.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_24908.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_24935.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_24946.csv'
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_24673.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_24761.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_24908.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_24935.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_24946.csv'
	]
);
gnuplot_export(
	'LMST',
	'image_evaluation_hypothese2_metriken_datacollection_lmst',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_24550.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_24581.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_24759.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_24910.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_24933.csv'
	],	
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_24550.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_24581.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_24759.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_24910.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_24933.csv'
	]
);
gnuplot_export(
	'noTC',
	'image_evaluation_hypothese2_metriken_mesh_notc',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_merge.csv', null, [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_23917.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_23957.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_23993.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24037.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24041.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24095.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24124.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24151.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24170.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_24232.csv',
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_merge.csv', null, [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_23917.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_23957.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_23993.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24037.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24041.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24095.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24124.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24151.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24170.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_24232.csv',
	]
);
gnuplot_export(
	'a-kTC',
	'image_evaluation_hypothese2_metriken_mesh_aktc',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_24674.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_24758.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_24909.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_24936.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_24949.csv'
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_24674.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_24758.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_24909.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_24936.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_24949.csv'
	]
);
gnuplot_export(
	'LMST',
	'image_evaluation_hypothese2_metriken_mesh_lmst',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_24578.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_24648.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_24675.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_24934.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_24948.csv',
	],
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_merge.csv', [
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_24578.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_24648.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_24675.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_24934.csv',
		'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_24948.csv',
	]
);*/

/*gnuplot_finish(
	'image_evaluation_hypothese2_metriken_combined_datacollection',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_notc/energy_merge.csv',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lmst/energy_merge.csv',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_aktc/energy_merge.csv',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/datacollection_lktc/energy_merge.csv'
);

gnuplot_finish(
	'image_evaluation_hypothese2_metriken_combined_mesh',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_notc/energy_merge.csv',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_lmst/energy_merge.csv',
	'/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/prr_merge.csv', '/Users/tpetry/www/Misc/masterarbeit/evaluate-thesis/mesh_aktc/energy_merge.csv',
	null, null
);*/

function gnuplot_export($topologycontrol, $filename, $prrFile, $prrFileReference, $prrFilesMinMax, $energyFile, $energyFileReference, $energyFilesMinMax) {
	//
	// prr (over time and per mote)
	//
	$gnuplotPrrOverTime = tempnam(sys_get_temp_dir(), 'gnuplot_prrovertime_');
	$gnuplotPrrPerMote  = tempnam(sys_get_temp_dir(), 'gnuplot_prrpermote_');
	file_put_contents($gnuplotPrrOverTime, sprintf("%s\t%s\t%s\t%s\t%s\n", 'minute', 'avg', 'reference', 'min', 'max'));
	//file_put_contents($gnuplotPrrPerMote, sprintf("%s\t%s\n", 'mote', 'prr'));
	
	$prrReference = [];
	if($prrFileReference) {
		$handle = fopen($prrFileReference, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			
			$csv = array_combine($header, $line);	
			if($csv['moteid'] == -1 && $csv['runtime'] != -1)
				$prrReference[$csv['runtime']] = $csv;
		}
	}
	
	$prrMinMax = array_fill(0, 61, ['min' => 100, 'max' => 0]);
	foreach($prrFilesMinMax as $prrFileMinMax) {
		$handle = fopen($prrFileMinMax, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
		
			$csv = array_combine($header, $line);
			if($csv['runtime'] != -1 && $csv['moteid'] == -1) {
				if($prrMinMax[$csv['runtime']]['min'] > $csv['prr_avg'])
					$prrMinMax[$csv['runtime']]['min'] = $csv['prr_avg'];
				if($prrMinMax[$csv['runtime']]['max'] < $csv['prr_avg'])
					$prrMinMax[$csv['runtime']]['max'] = $csv['prr_avg'];
			}
		}
	}
	
	$handle = fopen($prrFile, 'r');
	$header = null;
	while($line = fgetcsv($handle)) {
		if($header === null) {
			$header = $line;
			continue;
		}
		$csv = array_combine($header, $line);
		
		if($csv['moteid'] == -1 && $csv['runtime'] != -1) {
			file_put_contents($gnuplotPrrOverTime, file_get_contents($gnuplotPrrOverTime) . sprintf("%s\t%s\t%s\t%s\t%s\n", $csv['runtime'], $csv['prr_avg'], floatval(@$prrReference[$csv['runtime']]['prr_avg']), $prrMinMax[$csv['runtime']]['min'], $prrMinMax[$csv['runtime']]['max']));
		}
		if($csv['moteid'] != -1 && $csv['runtime'] == -1) {
			file_put_contents($gnuplotPrrPerMote, file_get_contents($gnuplotPrrPerMote) . sprintf("%s\t%s\n", $csv['moteid'], $csv['prr_avg']));
		}
	}
	
	$gnuplotscriptPrrOverTime = tempnam(sys_get_temp_dir(), 'gnuplotscript_prrovertime_');
	file_put_contents($gnuplotscriptPrrOverTime, "
		set terminal png
		set output '{$filename}_prr_time.png'
		set xlabel 'Laufzeit'
		set ylabel 'PRR'
		set xrange [0:60]
		set yrange [0:1.0]
		set key invert right bottom
		plot '{$gnuplotPrrOverTime}' using 1:4:5 with filledcurves linecolor rgb '#DCDCDC' title '{$topologycontrol} Abweichung', '' using 1:2 with lines linecolor rgb '#1F77B4' linewidth 2 title '{$topologycontrol}'" . ($prrFileReference ? ", '' using 1:3 with lines linecolor '#FF7F0E' linewidth 2 title 'noTC'" : "") . "
	");
	exec('gnuplot ' . $gnuplotscriptPrrOverTime);
	
	$gnuplotscriptPrrPerMote = tempnam(sys_get_temp_dir(), 'gnuplotscript_prrpermote_');
	file_put_contents($gnuplotscriptPrrPerMote, "
		set terminal png
		set output '{$filename}_prr_mote.png'
		set style fill solid border -1
		set style histogram rowstacked
		set style data histograms
		set boxwidth 0.75
		set xlabel 'Sensorknoten'
		set ylabel 'PRR'
		set nokey
		set xtics font ',10'
		plot '{$gnuplotPrrPerMote}' using 2:xticlabels(1) linecolor rgb '#1F77B4' title 'PRR'
	");
	exec('gnuplot ' . $gnuplotscriptPrrPerMote);
	
	//
	// energy (over time and per mote)
	//
	$gnuplotEnergyOverTime = tempnam(sys_get_temp_dir(), 'gnuplot_energyovertime_');
	$gnuplotEnergyPerMote  = tempnam(sys_get_temp_dir(), 'gnuplot_energypermote_');
	file_put_contents($gnuplotEnergyOverTime, sprintf("%s\t%s\t%s\t%s\t%s\n", 'minute', 'avg', 'reference', 'min', 'max'));
	//file_put_contents($gnuplotEnergyPerMote, sprintf("%s\t%s\t%s\t%s\n", 'mote', 'cpu', 'transmit', 'listen'));
	
	$energyReference = [];
	if($energyFileReference) {
		$handle = fopen($energyFileReference, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			
			$csv = array_combine($header, $line);	
			if($csv['moteid'] == -1 && $csv['runtime'] != -1)
				$energyReference[$csv['runtime']] = $csv;
		}
	}
	
	$energyMinMax = array_fill(0, 61, ['min' => 100000000, 'max' => 0]);
	foreach($energyFilesMinMax as $energyFileMinMax) {
		$handle = fopen($energyFileMinMax, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
		
			$csv = array_combine($header, $line);
			if($csv['runtime'] != -1 && $csv['moteid'] == -1) {
				if($energyMinMax[$csv['runtime']]['min'] > $csv['complete_energy_avg'])
					$energyMinMax[$csv['runtime']]['min'] = $csv['complete_energy_avg'];
				if($energyMinMax[$csv['runtime']]['max'] < $csv['complete_energy_avg'])
					$energyMinMax[$csv['runtime']]['max'] = $csv['complete_energy_avg'];
			}
		}
	}
	
	$handle = fopen($energyFile, 'r');
	$header = null;
	while($line = fgetcsv($handle)) {
		if($header === null) {
			$header = $line;
			continue;
		}
		$csv = array_combine($header, $line);
	
		if($csv['moteid'] == -1 && $csv['runtime'] != -1) {
			file_put_contents($gnuplotEnergyOverTime, file_get_contents($gnuplotEnergyOverTime) . sprintf("%s\t%s\t%s\t%s\t%s\n", $csv['runtime'], $csv['complete_energy_avg'], floatval(@$energyReference[$csv['runtime']]['complete_energy_avg']), $energyMinMax[$csv['runtime']]['min'], $energyMinMax[$csv['runtime']]['max']));
			if($csv['runtime'] == 59)
				file_put_contents($gnuplotEnergyOverTime, file_get_contents($gnuplotEnergyOverTime) . sprintf("%s\t%s\t%s\t%s\t%s\n", 60, $csv['complete_energy_avg'], floatval(@$energyReference[59]['complete_energy_avg']), $energyMinMax[59]['min'], $energyMinMax[59]['max']));
		}
		if($csv['moteid'] != -1 && $csv['runtime'] == -1) {
			file_put_contents($gnuplotEnergyPerMote, file_get_contents($gnuplotEnergyPerMote) . sprintf("%s\t%s\t%s\t%s\n", $csv['moteid'], $csv['cpu_active_avg'] + $csv['cpu_lpm_avg'] + $csv['cpu_irq_avg'], $csv['radio_transmit_avg'], $csv['radio_listen_avg']));				
		}
	}
	
	$gnuplotscriptEnergyOverTime = tempnam(sys_get_temp_dir(), 'gnuplotscript_energyovertime_');
	file_put_contents($gnuplotscriptEnergyOverTime, "
		set terminal png
		set output '{$filename}_energy_time.png'
		set xlabel 'Laufzeit'
		set ylabel 'Energieverbrauch (mAh)'
		set xrange [0:60]
		set yrange [0:160]
		set key invert right bottom
		plot '{$gnuplotEnergyOverTime}' using 1:4:5 with filledcurves linecolor rgb '#DCDCDC' title '{$topologycontrol} Abweichung', '' using 1:2 with lines linecolor rgb '#1F77B4' linewidth 2 title '{$topologycontrol}' " . ($prrFileReference ? ", '' using 1:3 with lines linecolor rgb '#FF7F0E' linewidth 2 title 'noTC'" : "") . "
	");
	exec('gnuplot ' . $gnuplotscriptEnergyOverTime);
	
	echo file_get_contents($gnuplotEnergyPerMote);
	
	$gnuplotscriptEnergyPerMote = tempnam(sys_get_temp_dir(), 'gnuplotscript_energypermote_');
	file_put_contents($gnuplotscriptEnergyPerMote, "
		set terminal png
		set output '{$filename}_energy_mote.png'
		set style fill solid border -1
		set style histogram rowstacked
		set style data histograms
		set boxwidth 0.75
		set xlabel 'Sensorknoten'
		set ylabel 'Energieverbrauch (mAh)'
		set yrange [0:12000]
		set xtics font ',10'
		plot '{$gnuplotEnergyPerMote}' using 2:xticlabels(1) linecolor rgb '#2CA02C' title 'CPU', '' using 3 linecolor rgb '#FF7F0E' title 'Senden', '' using 4 linecolor rgb '#1F77B4' title 'Empfangen' 
	");
	exec('gnuplot ' . $gnuplotscriptEnergyPerMote);
}

function gnuplot_finish($filename, $notcPrrFile, $notcEnergyFile, $lmstPrrFile, $lmstEnergyFile, $aktcPrrFile, $aktcEnergyFile, $lktcPrrFile, $lktcEnergyFile) {
	//
	// prr
	//
	$resultsPrr = [];
	foreach(['notc' => $notcPrrFile, 'lmst' => $lmstPrrFile, 'aktc' => $aktcPrrFile, 'lktc' => $lktcPrrFile] as $tc => $file) {
		if(!$file)
			continue;
		
		$handle = fopen($file, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			$csv = array_combine($header, $line);
			
			if($csv['runtime'] != -1 && $csv['moteid'] == -1) {
				if(!isset($resultsPrr[$csv['runtime']]))
					$resultsPrr[$csv['runtime']] = [];
				
				$resultsPrr[$csv['runtime']][$tc] = $csv['prr_avg'];
			}
		}
	}
	
	$gnuplotPrr  = tempnam(sys_get_temp_dir(), 'gnuplot_prr_');
	$gnuplotscriptPrr = tempnam(sys_get_temp_dir(), 'gnuplotscript_prr_');
	foreach($resultsPrr as $runtime => $results) {
		file_put_contents($gnuplotPrr, file_get_contents($gnuplotPrr) . sprintf("%s\t%s\t%s\t%s\t%s\n", $runtime, floatval(@$results['notc']), floatval(@$results['lmst']), floatval(@$results['aktc']), floatval(@$results['lktc'])));
	}
	
	file_put_contents($gnuplotscriptPrr, "
		set terminal png
		set output '{$filename}_prr.png'
		set xlabel 'Laufzeit'
		set ylabel 'PRR'
		set xrange [0:60]
		set key right bottom
		plot '{$gnuplotPrr}' using 1:2 with lines linecolor rgb '#FF7F0E' linewidth 2 title 'noTC', '' using 1:3 with lines linecolor rgb '#1F77B4' linewidth 2 title 'LMST', '' using 1:4 with lines linecolor rgb '#2CA02C' linewidth 2 title 'a-kTC'" . ($lktcPrrFile ? ", '' using 1:5 with lines linecolor rgb '#D62728' linewidth 2 title 'l-kTC'" : "") . "
	");
	exec('gnuplot ' . $gnuplotscriptPrr);
	
	//
	// energy
	//
	$resultsEnergy = [];
	foreach(['notc' => $notcEnergyFile, 'lmst' => $lmstEnergyFile, 'aktc' => $aktcEnergyFile, 'lktc' => $lktcEnergyFile] as $tc => $file) {
		if(!$file)
			continue;
		
		$handle = fopen($file, 'r');
		$header = null;
		while($line = fgetcsv($handle)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			$csv = array_combine($header, $line);
			
			if($csv['runtime'] != -1 && $csv['moteid'] == -1) {
				if(!isset($resultsEnergy[$csv['runtime']]))
					$resultsEnergy[$csv['runtime']] = [];
				
				$resultsEnergy[$csv['runtime']][$tc] = $csv['complete_energy_avg'];
				if($csv['runtime'] == 59)
					$resultsEnergy[60][$tc] = $csv['complete_energy_avg'];
			}
		}
	}
	
	$gnuplotEnergy = tempnam(sys_get_temp_dir(), 'gnuplot_energy_');
	$gnuplotscriptEnergy = tempnam(sys_get_temp_dir(), 'gnuplotscript_energy_');
	foreach($resultsEnergy as $runtime => $results) {
		file_put_contents($gnuplotEnergy, file_get_contents($gnuplotEnergy) . sprintf("%s\t%s\t%s\t%s\t%s\n", $runtime, floatval(@$results['notc']), floatval(@$results['lmst']), floatval(@$results['aktc']), floatval(@$results['lktc'])));
	}
	
	file_put_contents($gnuplotscriptEnergy, "
		set terminal png
		set output '{$filename}_energy.png'
		set xlabel 'Laufzeit'
		set ylabel 'Energieverbrauch (mAh)'
		set xrange [0:60]
		set key right bottom
		plot '{$gnuplotEnergy}' using 1:2 with lines linecolor rgb '#FF7F0E' linewidth 2 title 'noTC', '' using 1:3 with lines linecolor rgb '#1F77B4' linewidth 2 title 'LMST', '' using 1:4 with lines linecolor rgb '#2CA02C' linewidth 2 title 'a-kTC'" . ($lktcPrrFile ? ", '' using 1:5 with lines linecolor rgb '#D62728' linewidth 2 title 'l-kTC'" : "") . "
	");
	exec('gnuplot ' . $gnuplotscriptEnergy);
	
}


class RunnerHelper {
	use IgnoredLinksTrait;
	use MoteAddressTrait;
	use PowercontrolTrait;

	public function collectIgnoredLinks(array $messages) {
		return $this->_collectIgnoredLinks($messages, array_flip($this->_collectMoteAddresses($messages)));
	}

	public function collectPowercontrol(array $messages) {
		return $this->_collectPowercontrol($messages, array_flip($this->_collectMoteAddresses($messages)));
	}
}

function thetaSeparation($filename, $minute, $minimumDegree) {
	$messages = (new Flocklab())->getLogReader()->parseMessages($filename);
	$graph = (new NeighborhoodAnalyzer())->analyze($messages)[$minute];
	$motes = array_keys($graph->getVertices()->getMap());
	sort($motes, SORT_NUMERIC);
	$positions = (new Flocklab())->getMotePositions();
	
	$edgesCorrect = $edgesFailed = 0;
	
	foreach($motes as $mote1) {
		foreach($motes as $mote2) {
			foreach($motes as $mote3) {
				if(in_array($mote1, [$mote2, $mote3]) || in_array($mote2, [$mote1, $mote3]) || in_array($mote3, [$mote1, $mote2]))
					continue;
				if(!($mote2 < $mote3))
					continue;
				
				$vertex1 = $graph->getVertex($mote1);
				$vertex2 = $graph->getVertex($mote2);
				$vertex3 = $graph->getVertex($mote3);
				if(!$vertex1->hasEdgeFrom($vertex2) && !$vertex1->hasEdgeTo($vertex2))
					continue;
				if(!$vertex1->hasEdgeFrom($vertex3) && !$vertex1->hasEdgeTo($vertex3))
					continue;
				
				$distance_1_2 = sqrt(pow($positions[$mote2]["x"] - $positions[$mote1]["x"], 2) + pow($positions[$mote2]["y"] - $positions[$mote1]["y"], 2));
				$distance_1_3 = sqrt(pow($positions[$mote3]["x"] - $positions[$mote1]["x"], 2) + pow($positions[$mote3]["y"] - $positions[$mote1]["y"], 2));
				$distance_2_3 = sqrt(pow($positions[$mote3]["x"] - $positions[$mote2]["x"], 2) + pow($positions[$mote3]["y"] - $positions[$mote2]["y"], 2));
				$cosAlpha = (pow($distance_2_3, 2) - pow($distance_1_2, 2) - pow($distance_1_3, 2)) / (-2 * $distance_1_2 * $distance_1_3);
				$alpha = rad2deg(acos($cosAlpha));
				
				if($alpha < $minimumDegree)
					$edgesFailed++;
				else
					$edgesCorrect++;
				
				var_dump(sprintf("theta-of(%d-%d, %d-%d) = %f (correct=%d, failed=%d)", $mote1, $mote2, $mote1, $mote3, $alpha, $edgesCorrect, $edgesFailed));
			}
		}
	}
}

function routinggraphAccWithTraffic($filename, $minuteStart, $minuteEnd) {
	$flocklab = new Flocklab();
	$messages = $flocklab->getLogReader()->parseMessages($filename);
	$routing = (new RoutingAnalyzer())->analyze($messages);
	$powercontrol = (new RunnerHelper())->collectPowercontrol($messages);
	
	$graph = new Graph();
	foreach($routing[0]->getVertices() as $vertex)
		$graph->createVertex($vertex->getId());
	
	for($i = $minuteStart; $i <= $minuteEnd; $i++) {
		foreach($routing[$i]->getEdges() as $edge) {
			if(!$edge->getAttribute("active"))
				continue;
			
			$node1 = $graph->getVertex($edge->getVerticesStart()->getVertexFirst()->getId());
			$node2 = $graph->getVertex($edge->getVerticesTarget()->getVertexFirst()->getId());
			$node1_to_node2 = $node1->getEdgesTo($node2);
			$node2_to_node1 = $node1->getEdgesTo($node1);
			
			// possibility 1: bidirectional edge exists
			if(count($node1_to_node2) == 1 && count($node2_to_node1) == 1 && $node1_to_node2->getEdgeFirst() == $node2_to_node1->getEdgeFirst()) {
				$node1_to_node2->getEdgeFirst()->setAttribute("sent", $node1_to_node2->getEdgeFirst()->getAttribute("sent", 0) + $edge->getAttribute("sentmessage_minute", 0));
			}
			// possibility 2: unidirectional edge in contrary direction exists
			else if(count($node1_to_node2) == 0 && count($node2_to_node1) == 1) {
				$node1->createEdge($node2)->setAttribute("sent", $node2_to_node1->getEdgeFirst()->getAttribute("sent", 0) + $edge->getAttribute("sentmessage_minute", 0));
				$node2_to_node1->getEdgeFirst()->destroy();
			}
			// possibility 3: no edge between nodes exist
			else if(count($node1_to_node2) == 0 && count($node2_to_node1) == 0) {
				$node1->createEdgeTo($node2)->setAttribute("sent", $edge->getAttribute("sentmessage_minute", 0));
			}
			// possiblity 4: edge exists and has to be incremented
			else if(count($node1_to_node2) == 1 && count($node2_to_node1) == 0) {
				$node1_to_node2->getEdgeFirst()->setAttribute("sent", $node1_to_node2->getEdgeFirst()->getAttribute("sent", 0) + $edge->getAttribute("sentmessage_minute", 0));
			}
			else {
				throw new \Exception("this should be impossible");
			}
		}
	}
	
	$graphviz = new Digraph("G");
	foreach($graph->getVertices() as $vertex) {
		$graphviz->node($vertex->getId(), [
			"shape" => "circle",
			"width" => "0.4",
			"fixedsize"=> true,
			"pos" => sprintf("%d,-%d!", $flocklab->getMotePositions()[$vertex->getId()]["x"], $flocklab->getMotePositions()[$vertex->getId()]["y"])
		]);
	}
	$min = PHP_INT_MAX; $max = 0; $sent = [];
	foreach($graph->getEdges() as $edge) {
		$min = min($min, $edge->getAttribute("sent"));
		$max = max($max, $edge->getAttribute("sent"));
		$sent[] = $edge->getAttribute("sent");
	}
	sort($sent, SORT_NUMERIC);
	//$max = $sent[floor(1.0 * count($sent) - 1)];
	var_dump(["min" => $min, "max" => $max]);
	
	foreach($graph->getEdges() as $edge) {
		$node1 = $edge->getVerticesStart()->getVertexFirst()->getId();
		$node2 = $edge->getVerticesTarget()->getVertexFirst()->getId();
		
		$powerlevel = $powerlevel_cnt = 0;
		for($i = $minuteStart; $i <= $minuteEnd; $i++) {
			if(!isset($powercontrol[$i][$node1][$node2]))
				continue;
			
			$powerlevel += ($powercontrol[$i][$node1][$node2] == -1 ? 31 : $powercontrol[$i][$node1][$node2]);
			$powerlevel_cnt++;
		}
		if($powerlevel_cnt == 0) {
			$powerlevel = 31;
			$powerlevel_cnt = 1;
		}
		$powerlevel = round($powerlevel / $powerlevel_cnt);
		
		$percentPowerlevel = min(1.0, max(0.0, ($powerlevel - 16) / 16));
		$percentSent = min(1.0, max(0.0, ($edge->getAttribute("sent") - $min) / ($max - $min)));
		$attributes = ["color" => colorGradient(0xC0C0C0, 0x000000, $percentSent), "penwidth" => strval(round(1.0 + 3.5 * $percentSent, 1))];
		if($edge instanceof EdgeUndirected)
			$attributes = array_merge($attributes, ["arrowhead" => "normal", "arrowtail" => "normal", "dir" => "both"]);
		$graphviz->edge([$node1, $node2], $attributes);
	}
	
	$filename = sprintf("%s/%s.png", sys_get_temp_dir(), uniqid());
	$command = sprintf("echo %s | dot -Tpng -Kneato -n -o%s", escapeshellarg(str_replace("\n", " ",$graphviz->render())), escapeshellarg($filename));
	system($command, $ret);
	exec("open " . escapeshellarg($filename) . " > /dev/null 2>&1 &");
	
	exit();
}


function routinggraphWithDroppedEdges($filename) {
	$flocklab = new Flocklab();
	$messages = $flocklab->getLogReader()->parseMessages($filename);	
	$graph = (new RoutingGraphAnalyzer(10, $flocklab->getMotePositions()))->analyze($messages);
	$ignored = (new RunnerHelper())->collectIgnoredLinks($messages)[12];
	
	$graphviz = new Digraph("G");
	foreach($graph->getVertices() as $vertex) {
		$graphviz->node($vertex->getId(), [
			"shape" => "circle",
			"width" => "0.4",
			"fixedsize"=> true,
			"pos" => sprintf("%d,-%d!", $flocklab->getMotePositions()[$vertex->getId()]["x"], $flocklab->getMotePositions()[$vertex->getId()]["y"])
		]);
	}
	$edgesPlotted = [];
	foreach($graph->getEdges() as $edge) {
		$node1 = $edge->getVerticesStart()->getVertexFirst();
		$node2 = $edge->getVerticesTarget()->getVertexFirst();
		
		if(isset($edgesPlotted[min($node1->getId(), $node2->getId()) . "-" . max($node1->getId(), $node2->getId())]))
			continue;
		$edgesPlotted[min($node1->getId(), $node2->getId()) . "-" . max($node1->getId(), $node2->getId())] = true;
		
		$attributes = [];
		if(in_array($node2->getId(), $ignored[$node1->getId()]) || in_array($node1->getId(), $ignored[$node2->getId()]))
			$attributes["style"] = "dashed";
		if($node2->hasEdgeTo($node1))
			$attributes = array_merge($attributes, ["arrowhead" => "normal", "arrowtail" => "normal", "dir" => "both"]);
		
		$graphviz->edge([$node1->getId(), $node2->getId()], $attributes);
	}
	
	$filename = sprintf("%s/%s.png", sys_get_temp_dir(), uniqid());
	$command = sprintf("echo %s | dot -Tpng -Kneato -n -o%s", escapeshellarg(str_replace("\n", " ",$graphviz->render())), escapeshellarg($filename));
	system($command, $ret);
	exec("open " . escapeshellarg($filename) . " > /dev/null 2>&1 &");
}

/**
 * @see http://www.herethere.net/~samson/php/color_gradient/color_gradient_generator.php.txt
 */
function interpolate($pBegin, $pEnd, $pStep, $pMax) {
	if ($pBegin < $pEnd) {
		return (($pEnd - $pBegin) * ($pStep / $pMax)) + $pBegin;
	} else {
		return (($pBegin - $pEnd) * (1 - ($pStep / $pMax))) + $pEnd;
	}
}
function colorGradient($theColorBegin, $theColorEnd, $percent) {
	$theR0 = ($theColorBegin & 0xff0000) >> 16;
	$theG0 = ($theColorBegin & 0x00ff00) >> 8;
	$theB0 = ($theColorBegin & 0x0000ff) >> 0;
	
	$theR1 = ($theColorEnd & 0xff0000) >> 16;
	$theG1 = ($theColorEnd & 0x00ff00) >> 8;
	$theB1 = ($theColorEnd & 0x0000ff) >> 0;
	
	$theR = interpolate($theR0, $theR1, round($percent * 100 / 6.25), 16);
	$theG = interpolate($theG0, $theG1, round($percent * 100 / 6.25), 16);
	$theB = interpolate($theB0, $theB1, round($percent * 100 / 6.25), 16);
	
	return sprintf("#%06X", ((($theR << 8) | $theG) << 8) | $theB);
}