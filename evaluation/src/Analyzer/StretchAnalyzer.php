<?php

namespace App\Analyzer;

use App\Analyzer\Helper\SerializeTrait;
use App\Analyzer\Helper\StatisticsTrait;
use App\Analyzer\NeighborhoodAnalyzer;
use Fhaculty\Graph\Graph;
use Graphp\Algorithms\ShortestPath\BreadthFirst;

/**
 * Energy Evaluation
 *
 * @author Michael Stein
 */
class StretchAnalyzer implements AnalyzerInterface {
	//use EnergyTrait;
	use SerializeTrait { serialize as protected _serializeFromTrait; }
	use StatisticsTrait;

	
	/**
	 * constructs new stretch analyzer
	 * 
	 * @param 
	 */
	public function __construct() {
	//
	}
	
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[] $messages
	 * @param string $energyMode
	 * @return array
	 */
	public function analyze(array $messages) {
		
/*
		$moteEnergyAtMinute = $this->_collectMoteEnergy($messages, $this->_energyMode === self::ENERGY_MILLIAMPEREHOUR);
		
		return [
			"motes" => $moteEnergyAtMinute,
			"network" => $this->_calculateEnergyOfNetwork($moteEnergyAtMinute)
		];
*/

	$_minuteInput = 5;
	$_minuteOutput = 20;
	$targetMote = '1';


	$graphInput = (new NeighborhoodAnalyzer())->analyze($messages)[$_minuteInput];
	$graphOutput = (new NeighborhoodAnalyzer())->analyze($messages)[$_minuteOutput];


	$motes = array_keys($graphInput->getVertices()->getMap());


	$stretchOfMotes = [];

	foreach($motes as $sourceMote) {

		// node 1 always has stretch 1
		if($sourceMote == '1') {
			$stretchOfMotes[$sourceMote] = 1;

			continue;
		}

//		var_dump(sprintf("Handling input mote: %d", $sourceMote));
		
		// the source vertex in the input graph
		$vertexSourceInput = $graphInput->getVertex($sourceMote);

		// the source vertex in the output graph
		$vertexSourceOutput = $graphOutput->getVertex($sourceMote);

		// the target vertex in the input graph
		$vertexTargetInput = $graphInput->getVertex($targetMote);

		// the target vertex in the output graph
		$vertexTargetOutput = $graphOutput->getVertex($targetMote);

//		var_dump(sprintf("Target mote: %s", $vertexTargetInput->getId()));


		// compute and compare hop distance in both graphs
		//$hopCountInput = computeHopCount($graphInput, $vertexSourceInput, $vertexTargetInput);
		$hopCountInput = (new BreadthFirst($vertexSourceInput))->getDistance($vertexTargetInput);
//		var_dump(sprintf("Distance in input graph: %d", $hopCountInput));
		
		// compute hop count output topology
		$hopCountOutput = (new BreadthFirst($vertexSourceOutput))->getDistance($vertexTargetOutput);
//		var_dump(sprintf("Distance in output graph: %d", $hopCountOutput));

		// compute stretch factor
		$stretch = number_format($hopCountOutput,5)/number_format($hopCountInput,5);
//		var_dump(sprintf("Stretch: %f", $stretch));	


		// store result somewhere for later processing...
		$stretchOfMotes[$sourceMote] = $stretch;
	}

		return [
			"motes" => $stretchOfMotes
		];
}


	
	/**
	 * serializes the analyze results to a csv file
	 * 
	 * @param string $filename
	 * @param array $data
	 */
	public function serialize($filename, $data) {

//		var_dump($data);
		
		$serializedata = []; //$data["motes"];

		// add the mote stretch information
		foreach($data["motes"] as $moteid => $moteStretch) {
			$data = ["moteid" => $moteid, "moteStretch" => $moteStretch];
//			$data["moteStretch"] = $this->_calculateStatistics([$moteStretch], $this->PERCENTILE_SORT_ASC);

			$serializedata[] = $data;
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
	// TODO MSt implement

	
		$datasComplete = [];
		foreach($serializationFiles as $file) {
			foreach($this->_unserializeNoRuntime($file, true) as $values) {
				if(!isset($complete["moteid"]))
					$complete["moteid"] = [];
				foreach($values as $valueName => $valueValue) {
					if(preg_match("/^" . "moteid" . "$/", $valueName)){
						if(!in_array($valueValue,$complete["moteid"]))
							$complete["moteid"][]=$valueValue;
					}
					else{
						if(!isset($complete["moteStretch"][$values["moteid"]]))
							$complete["moteStretch"][$values["moteid"]]=[];
						$complete["moteStretch"][$values["moteid"]][] = $valueValue;
					}
				}
			}
		}
		foreach($complete["moteid"] as $moteid) {
			if(!isset($datasComplete["moteid"]))
				$datasComplete["moteid"]=[];
			if(!isset($datasComplete["averageStretch"]))
				$datasComplete["averageStretch"]=[];
			$datasComplete["moteid"][]=$moteid;
			$datasComplete["averageStretch"][$moteid]= array_sum($complete["moteStretch"][$moteid])/sizeof($complete["moteStretch"][$moteid]);
		}
		$serializedata = [];
		foreach($datasComplete["moteid"] as $moteid){
			$serializedata[]=[
				"moteid" => $moteid,
				"avgStretch" =>$datasComplete["averageStretch"][$moteid]
			];
		}
		$this->_serializeFromTrait($filename, $serializedata);
	}
	
	
}
