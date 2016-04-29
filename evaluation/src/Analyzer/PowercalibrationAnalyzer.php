<?php

namespace App\Analyzer;

use IpUtils\Factory as IpFactory;

/**
 * Powercalibration Analyzer
 * 
 * Creates a matrice with optimum txpower
 *
 * REMARK: this is not directly an analyzer but the same implemented interface makes
 * the development of the evaluation cli-script much easier
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class PowercalibrationAnalyzer implements AnalyzerInterface {
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[]
	 * @return array
	 */
	public function analyze(array $messages) {
		$powerMin = $powerMax = -1;
		$optimizations = [];
		
		foreach($messages as $message) {
			if(preg_match("/^\[application-powercalibration\] testing power range (?P<min>\d+)-(?P<max>\d+)$/", $message->getMessage(), $matches)) {
				$powerMin = $matches["min"];
				$powerMax = $matches["max"];
			}
			if(preg_match("/^\[application-powercalibration] optimum txpower=(?P<power>\d+) for (?P<source>.+)->(?P<destination>.+)$/", $message->getMessage(), $matches)) {
				if(!isset($optimizations[$matches["source"]]))
					$optimizations[$matches["source"]] = [];
				
				$power = ($matches["power"] != $powerMax) ? (int) $matches["power"] : -1;
				$optimizations[$matches["source"]][$matches["destination"]] = $power;
			}
		}
		
		if($powerMin == -1 && $powerMax == -1)
			throw new Exception("no testing power range information found");
		
		if(count($optimizations) == 0)
			throw new Exception("no optimizations found");
		
		return $optimizations;
	}
	
	/**
	 * serializes the analyzed results to a png file
	 *
	 * @param string $filename
	 * @param array $data
	 */
	public function serialize($filename, $data) {
		$motes = [];
		foreach($data as $source => $powers)
			$motes = array_unique(array_merge($motes, [$source], array_keys($powers)));
		sort($motes, SORT_NATURAL);
		
		$matrix = [];
		foreach($motes as $row) {
			foreach($motes as $column) {
				if(!isset($matrix[$row]))
					$matrix[$row] = [];				
				
				$matrix[$row][$column] = -1;
				if(isset($data[$row][$column]))
					$matrix[$row][$column] = $data[$row][$column];
			}
		}
		
		$powercalibration = implode("\n", [
			"// for best results you should average some power calibrations by yourself",
			"#define COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_SIZE {{num}}",
			"#define COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_MOTES {{header}}",
			"#define COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_POWERS {{values}}"
		]);
		$powercalibration = str_replace("{{num}}", count($motes), $powercalibration);
		$powercalibration = str_replace("{{header}}", $this->_getMatrixHeader($motes), $powercalibration);
		$powercalibration = str_replace("{{values}}", $this->_getMatrixValues($matrix), $powercalibration);
		
		file_put_contents($filename, $powercalibration);
	}
	
	/**
	 * gets serialization filetype
	 *
	 * @return string filetype
	 */
	public function serializationType() {
		return "txt";
	}
	
	/**
	 * merges several serialized evaluations
	 *
	 * @param string $filename
	 * @param array $serializationFiles
	 */
	public function merge($filename, array $serializationFiles) {
		throw new Exception("not implemented yet");
	}
	
	/**
	 * formats matrix header
	 *
	 * @param array $motes
	 * @return string
	 */
	protected function _getMatrixHeader(array $motes) {
		$addresses = [];
		
		foreach($motes as $mote) {
			if(strpos($mote, ".") !== false) {
				// rime
				$addresses[] = "{{" . implode(",", explode(".", $mote)) . "}}";
			} else {
				// ipv6
				$expanded = IpFactory::getAddress($mote)->getExpanded();
				$bytes = array_map(function(array $nibbles) {
					return "0x" . $nibbles[0] . $nibbles[1];
				},array_chunk(str_split(str_replace(":", "", $expanded)), 2));
				$addresses[] = "{{" . implode(",", $bytes) . "}}";
			}
		}
		
		return "{" . implode(", ", $addresses) . "}";
	}
	
	/**
	 * formats matrix values
	 * 
	 * @param array $matrix
	 * @return string
	 */
	protected function _getMatrixValues(array $matrix) {
		$powerProfileMatrixRows = array_map(function($powerlevels) {
			return "{" . implode(", ", $powerlevels) ."}";
		}, $matrix);
		
		return "{" . implode(", ", $powerProfileMatrixRows) . "}";
	}
}