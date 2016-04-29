<?php

namespace App\Analyzer;

use App\Analyzer\Helper\MoteAddressTrait;
use App\Analyzer\Helper\SerializeTrait;
use App\Analyzer\Helper\StatisticsTrait;
use App\Analyzer\Helper\PrrTrait;

/**
 * Packet Reception Rate Evaluation
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class PrrAnalyzer implements AnalyzerInterface {
	use MoteAddressTrait;
	use SerializeTrait { serialize as protected _serializeFromTrait; }
	use StatisticsTrait;
	use PrrTrait;
	
	/**
	 * analyzes the packet reception rate
	 *
	 * @param App\LogReader\Message[]
	 * @return array
	 */
	public function analyze(array $messages) {
		$moteid2address = $this->_collectMoteAddresses($messages);
		$address2moteid = array_flip($moteid2address);
		$motePrrAtMinute = $this->_collectMotePrr($messages, $address2moteid);
		
		return [
			"motes" => $motePrrAtMinute,
			"network" => $this->_calculatePrrOfNetwork($motePrrAtMinute)
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
		
		// add the mote prr information
		foreach($data["motes"] as $moteid => $moteData) {
			foreach($moteData as $minute => $minuteData) {
				$serializedata[] = [
					"moteid" => $moteid,
					"runtime" => $minute,
					"sent" => $this->_calculateStatistics([$minuteData["sent"]], $this->PERCENTILE_SORT_ASC),
					"received" => $this->_calculateStatistics([$minuteData["received"]], $this->PERCENTILE_SORT_ASC),
					"prr" => $this->_calculateStatistics([$minuteData["prr"]], $this->PERCENTILE_SORT_ASC),
				];
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
		$datasNetwork = ["sent" => [], "received" => [], "prr" => []];
		foreach($serializationFiles as $file) {
			$complete = [];
			
			foreach($this->_unserialize($file, true) as $values) {
				if(!isset($datasMinute[$values["moteid"]]))
					$datasMinute[$values["moteid"]] = [];
				if(!isset($datasMinute[$values["moteid"]][$values["runtime"]]))
					$datasMinute[$values["moteid"]][$values["runtime"]] = ["sent" => [], "received" => [], "prr" => []];
				if(!isset($datasMinute[$values["moteid"]][$values["runtime"]]["received"]))
					$datasMinute[$values["moteid"]][$values["runtime"]]["sent"] = [];
				if(!isset($complete[$values["moteid"]]))
					$complete[$values["moteid"]] = ["sent" => 0, "received" => 0];
				
				$datasMinute[$values["moteid"]][$values["runtime"]]["sent"][] = $values["sent_sum"];
				$datasMinute[$values["moteid"]][$values["runtime"]]["received"][] = $values["received_sum"];
				$datasMinute[$values["moteid"]][$values["runtime"]]["prr"][] = @($values["received_sum"] / $values["sent_sum"]);
				$complete[$values["moteid"]]["sent"] += $values["sent_sum"];
				$complete[$values["moteid"]]["received"] += $values["received_sum"];
			}
			
			$sent = $received = 0;
			foreach($complete as $moteid => $datas) {
				if(!isset($datasComplete[$moteid]))
					$datasComplete[$moteid] = ["sent" => [], "received" => [], "prr" => []];
				
				$datasComplete[$moteid]["sent"][] = $datas["sent"];
				$datasComplete[$moteid]["received"][] = $datas["received"];
				$datasComplete[$moteid]["prr"][] = @($datas["received"] / $datas["sent"]);
				
				$sent += $datas["sent"];
				$received += $datas["received"];
			}
			
			$datasNetwork["sent"][] = $sent;
			$datasNetwork["received"][] = $received;
			$datasNetwork["prr"][] = @($received / $sent);
		}
		
		$serializedata = [];
		
		// csv entries for moteid=X, runtime=Y:
		foreach($datasMinute as $moteid => $moteidMinutes) {
			foreach($moteidMinutes as $runtime => $values) {
				$serializedata[] = [
					"moteid" => $moteid,
					"runtime" => $runtime,
					"sent" => $this->_calculateStatistics($values["sent"], $this->PERCENTILE_SORT_ASC),
					"received" => $this->_calculateStatistics($values["received"], $this->PERCENTILE_SORT_ASC),
					"prr" => $this->_calculateStatistics($values["prr"], $this->PERCENTILE_SORT_ASC)
				];
			}
			
		}
		// csv entries for moteid=X, runtime=-1:
		foreach($datasComplete as $moteid => $values) {
			$serializedata[] = [
				"moteid" => $moteid,
				"runtime" => -1,
				"sent" => $this->_calculateStatistics($values["sent"], $this->PERCENTILE_SORT_ASC),
				"received" => $this->_calculateStatistics($values["received"], $this->PERCENTILE_SORT_ASC),
				"prr" => $this->_calculateStatistics($values["prr"], $this->PERCENTILE_SORT_ASC)
			];
		}
		// csv entries for moteid=-1, runtime=Y:
		$runtimes = array_unique(call_user_func_array("array_merge", array_map("array_keys", $datasMinute)));
		foreach($runtimes as $runtime) {
			$values = ["sent" => [], "received" => [], "prr" => []];
			foreach($datasMinute as $moteidMinutes) {
				if(isset($moteidMinutes[$runtime])) {
					$values["sent"] = array_merge($values["sent"], $moteidMinutes[$runtime]["sent"]);
					$values["received"] = array_merge($values["received"], $moteidMinutes[$runtime]["received"]);
					$values["prr"] = array_merge($values["prr"], $moteidMinutes[$runtime]["prr"]);
				}
			}
			
			$serializedata[] = [
				"moteid" => -1,
				"runtime" => $runtime,
				"sent" => $this->_calculateStatistics($values["sent"], $this->PERCENTILE_SORT_ASC),
				"received" => $this->_calculateStatistics($values["received"], $this->PERCENTILE_SORT_ASC),
				"prr" => $this->_calculateStatistics($values["prr"], $this->PERCENTILE_SORT_ASC)
			];
		}
		// csv entries for moteid=-1, runtime=-1:
		{
			
			$serializedata[] = [
				"moteid" => -1,
				"runtime" => -1,
				"sent" => $this->_calculateStatistics($datasNetwork["sent"], $this->PERCENTILE_SORT_ASC),
				"received" => $this->_calculateStatistics($datasNetwork["received"], $this->PERCENTILE_SORT_ASC),
				"prr" => $this->_calculateStatistics($datasNetwork["prr"], $this->PERCENTILE_SORT_ASC)
			];
		}
		
		// sort data by moteid ASC, runtime ASC
		usort($serializedata, function ($a, $b) {
			if($a["moteid"] != $b["moteid"])
				return $a["moteid"] - $b["moteid"];
			
			return $a["runtime"] - $b["runtime"];
		});
		
		$this->_serializeFromTrait($filename, $serializedata);
	}
	
	protected function _calculatePrrOfNetwork(array $motepPrrAtMinute) {
		$prrAtMinute = [];
		
		$randomMoteid = array_keys($motepPrrAtMinute)[0]; // access to energy profile of a random mote
		foreach(array_keys($motepPrrAtMinute[$randomMoteid]) as $minute) {
			$values = ["sent" => [], "received" => [], "prr" => []];
			foreach($motepPrrAtMinute as $moteid => $motePrr) {
				if(!isset($motePrr[$minute])) {
					trigger_error("mote $moteid has no prr information for minute $minute", E_USER_WARNING);
					continue;
				}
				
				foreach($values as $valueName => $_) {
					$values[$valueName][$moteid] = $motePrr[$minute][$valueName];
				}
			}
			
			$prrAtMinute[$minute] = [
				"sent" => $this->_calculateStatistics($values["sent"], $this->PERCENTILE_SORT_ASC),
				"received" => $this->_calculateStatistics($values["received"], $this->PERCENTILE_SORT_ASC),
				"prr" => $this->_calculateStatistics($values["prr"], $this->PERCENTILE_SORT_ASC)
			];
		}
		
		return $prrAtMinute;
	}
}