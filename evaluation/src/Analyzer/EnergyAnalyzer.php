<?php

namespace App\Analyzer;

use App\Analyzer\Helper\EnergyTrait;
use App\Analyzer\Helper\SerializeTrait;
use App\Analyzer\Helper\StatisticsTrait;

/**
 * Energy Evaluation
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class EnergyAnalyzer implements AnalyzerInterface {
	use EnergyTrait;
	use SerializeTrait { serialize as protected _serializeFromTrait; }
	use StatisticsTrait;
	
	/**
	 * energy should provided as energest ticks
	 */
	const ENERGY_TICKS = "ticks";
	
	/**
	 * energy should provided as mAh
	 */
	const ENERGY_MILLIAMPEREHOUR = "mAh";
	
	/**
	 * @var string
	 */
	protected $_energyMode;
	
	/**
	 * constructs new energy analyzer
	 * 
	 * @param string $energyMode
	 */
	public function __construct($energyMode) {
		$this->_energyMode = $energyMode;
	}
	
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[] $messages
	 * @param string $energyMode
	 * @return array
	 */
	public function analyze(array $messages) {
		$moteEnergyAtMinute = $this->_collectMoteEnergy($messages, $this->_energyMode === self::ENERGY_MILLIAMPEREHOUR);
		
		return [
			"motes" => $moteEnergyAtMinute,
			"network" => $this->_calculateEnergyOfNetwork($moteEnergyAtMinute)
		];
	}
	
	/**
	 * serializes the analyze results to a csv file
	 * 
	 * @param string $filename
	 * @param array $data
	 */
	public function serialize($filename, $data) {
		$serializedata = [];
		
		// add the system energy information
		foreach($data["network"] as $minute => $minuteData) {
			$serializedata[] = array_merge(["moteid" => "-1", "runtime" => $minute], $minuteData);
		}
		
		// add the mote energy information
		foreach($data["motes"] as $moteid => $moteData) {
			foreach($moteData as $minute => $minuteData) {
				$data = ["moteid" => $moteid, "runtime" => $minute];
				foreach($minuteData as $energysource => $energyvalue) {
					if(!is_array($energyvalue)) {
						$data[$energysource] = $this->_calculateStatistics([$energyvalue], $this->PERCENTILE_SORT_ASC);
					} else {
						$data[$energysource] = [];
						foreach($energyvalue as $subenergysource => $subenergyvalue) {
							$data[$energysource][$subenergysource] = $this->_calculateStatistics([$subenergyvalue], $this->PERCENTILE_SORT_ASC);
						}
					}
				}
				
				$serializedata[] = $data;
			}
		}
		
		// sort data by moteid ASC, runtime ASC
		usort($serializedata, function ($a, $b) {
			if($a["moteid"] != $b["moteid"])
				return $a["moteid"] - $b["moteid"];
			
			return $a["runtime"] - $b["runtime"];
		});
		
		$this->_serializeFromTrait($filename, $serializedata);
	}
	
	/**
	 * merges several serialized evaluations
	 *
	 * @param string $filename
	 * @param array $serializationFiles
	 */
	public function merge($filename, array $serializationFiles) {
		$datasMinute = [];
		$datasComplete = [];
		foreach($serializationFiles as $file) {
			$complete = [];
			
			foreach($this->_unserialize($file, true) as $values) {
				if(!isset($datasMinute[$values["moteid"]]))
					$datasMinute[$values["moteid"]] = [];
				if(!isset($datasMinute[$values["moteid"]][$values["runtime"]]))
					$datasMinute[$values["moteid"]][$values["runtime"]] = [];
				if(!isset($complete[$values["moteid"]]))
					$complete[$values["moteid"]] = [];
				
				foreach($values as $valueName => $valueValue) {
					$properties = ['cpu_active_sum', 'cpu_lpm_sum', 'cpu_irq_sum', 'radio_listen_sum', 'radio_transmit_sum', 'radio_transmit_powerlevels_.*_sum', 'complete_energy_sum'];
					foreach($properties as $property) {
						if(!preg_match("/^" . $property . "$/", $valueName))
							continue;
						
						if(!isset($datasMinute[$values["moteid"]][$values["runtime"]][$valueName]))
							$datasMinute[$values["moteid"]][$values["runtime"]][$valueName] = [];
						
						$datasMinute[$values["moteid"]][$values["runtime"]][$valueName][] = $valueValue;
						$complete[$values["moteid"]][$valueName] = @$complete[$values["moteid"]][$valueName] + $valueValue;
					}
				}
			}
			
			foreach($complete as $moteid => $datas) {
				if(!isset($datasComplete[$moteid]))
					$datasComplete[$moteid] = [];
				
				foreach($datas as $valueName => $valueValue) {
					if(!isset($datasComplete[$moteid][$valueName]))
						$datasComplete[$moteid][$valueName] = [];
					
					$datasComplete[$moteid][$valueName][] = $valueValue;
				}
			}
		}
		
		$serializedata = [];
		
		// csv entries for moteid=X, runtime=Y:
		foreach($datasMinute as $moteid => $moteidMinutes) {
			foreach($moteidMinutes as $runtime => $values) {
				$csvLine = [
					"moteid" => $moteid,
					"runtime" => $runtime
				];
				foreach($values as $valueName => $valueValue)
					$csvLine[substr($valueName, 0, -4)] = $this->_calculateStatistics($valueValue, $this->PERCENTILE_SORT_ASC);
				
				$serializedata[] = $csvLine;
			}
		}
		// csv entries for moteid=X, runtime=-1:
		foreach($datasComplete as $moteid => $values) {
			$csvLine = [
				"moteid" => $moteid,
				"runtime" => -1
			];
			foreach($values as $valueName => $valueValue)
				$csvLine[substr($valueName, 0, -4)] = $this->_calculateStatistics($valueValue, $this->PERCENTILE_SORT_ASC);
			
			$serializedata[] = $csvLine;
		}
		// csv entries for moteid=-1, runtime=Y:
		$runtimes = array_unique(call_user_func_array("array_merge", array_map("array_keys", $datasMinute)));
		foreach($runtimes as $runtime) {
			$csvLine = [
				"moteid" => -1,
				"runtime" => $runtime
			];
			
			foreach(array_slice($datasMinute, 0, 1)[0][$runtime] as $valueName => $_) {
				$values = [];
				foreach($datasMinute as $moteid => $moteidMinutes)
					$values = array_merge($values, $moteidMinutes[$runtime][$valueName]);
				
				$csvLine[substr($valueName, 0, -4)] = $this->_calculateStatistics($values, $this->PERCENTILE_SORT_ASC);
			}
			
			$serializedata[] = $csvLine;
		}
		// csv entries for moteid=-1, runtime=-1:
		{
			$csvLine = [
				"moteid" => -1,
				"runtime" => -1
			];
			
			foreach(array_keys(array_slice($datasComplete, 0, 1)[0]) as $valueName) {
				$values = [];
				foreach($datasComplete as $moteid => $moteidValues)
					$values = array_merge($values, $moteidValues[$valueName]);
				
				$csvLine[substr($valueName, 0, -4)] = $this->_calculateStatistics($values, $this->PERCENTILE_SORT_ASC);
			}
			
			$serializedata[] = $csvLine;
		}
		
		// sort data by moteid ASC, runtime ASC
		usort($serializedata, function ($a, $b) {
			if($a["moteid"] != $b["moteid"])
				return $a["moteid"] - $b["moteid"];
			
			return $a["runtime"] - $b["runtime"];
		});
		
		$this->_serializeFromTrait($filename, $serializedata);
	}
	
	/**
	 * calculates energy of system
	 * 
	 * @param array $moteEnergyAtMinute
	 * @return array
	 */
	protected function _calculateEnergyOfNetwork(array $moteEnergyAtMinute) {
		$energyAtMinute = [];
		
		$randomMoteid = array_keys($moteEnergyAtMinute)[0]; // access to energy profile of a random mote
		foreach(array_keys($moteEnergyAtMinute[$randomMoteid]) as $minute) {
			// check for existing energy profile (maybe the node has been started later, did fail...)
			foreach($moteEnergyAtMinute as $moteid => $moteEnergies) {
				if(!isset($moteEnergies[$minute])) {
					trigger_error("mote $moteid has no energy profile for minute $minute", E_USER_WARNING);
				}
			}
			
			$energyAtMinute[$minute] = [];
			foreach(array_keys($moteEnergies[0]) as $energysource) {
				if(!is_array($moteEnergyAtMinute[$randomMoteid][0][$energysource])) {
					$values = [];
					foreach($moteEnergyAtMinute as $moteid => $moteEnergies) {
						if(!isset($moteEnergies[$minute]))
							continue;
						
						$values[$moteid] = $moteEnergies[$minute][$energysource];
					}
					
					$energyAtMinute[$minute][$energysource] = $this->_calculateStatistics($values, $this->PERCENTILE_SORT_ASC);
				} else {
					$energyAtMinute[$minute][$energysource] = [];
					
					foreach(array_keys($moteEnergyAtMinute[$randomMoteid][0][$energysource]) as $subenergysource) {
						$values = [];
						foreach($moteEnergyAtMinute as $moteid => $moteEnergies) {
							if(!isset($moteEnergies[$minute]))
								continue;
						
							$values[$moteid] = $moteEnergies[$minute][$energysource][$subenergysource];
						}
							
						$energyAtMinute[$minute][$energysource][$subenergysource] = $this->_calculateStatistics($values, $this->PERCENTILE_SORT_ASC);
					}
				}
			}
		}
		
		return $energyAtMinute;
	}
}