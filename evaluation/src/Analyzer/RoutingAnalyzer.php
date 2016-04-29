<?php

namespace App\Analyzer;

use App\Analyzer\Helper\MoteAddressTrait;
use App\Analyzer\Helper\PrrTrait;
use App\Analyzer\Helper\RoutingTrait;
use App\Analyzer\Helper\SendingTargetTrait;
use Fhaculty\Graph\Graph as Graph;
use Fhaculty\Graph\Edge\Base as Edge;

/**
 * Routing Evaluation
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class RoutingAnalyzer implements AnalyzerInterface {
	use MoteAddressTrait;
	use PrrTrait;
	use RoutingTrait;
	use SendingTargetTrait;
	
	/**
	 * analyzes the network graph
	 *
	 * @param App\LogReader\Message[]
	 * @return array
	 */
	public function analyze(array $messages) {
		$starttime = $messages[0]->getTimestamp();
		$address2moteid = array_flip($this->_collectMoteAddresses($messages));
		$sendingTargets = $this->_collectMoteSendingTargets($messages, $address2moteid);
		$sentPackets = $this->_collectSentPackets($messages, $address2moteid);
		$moteRoutingAtMinute = $this->_collectMoteRouting($messages, $address2moteid);
		
		// init graph with mote vertices
		$lastgraph = new Graph();
		foreach($address2moteid as $moteid) {
			$lastgraph->createVertex($moteid);
		}
		
		$graphAtMinute = [];
		foreach(array_keys($moteRoutingAtMinute[$messages[0]->getMoteid()]) as $minute) {
			// clone last graph to keep accumulated message flow on edges
			$graphAtMinute[$minute] = $lastgraph = $lastgraph->createGraphClone();
			
			// clean message flow of minute & active status of edges
			foreach($graphAtMinute[$minute]->getEdges() as $edge) {
				$edge->setAttribute("sentmessage_minute", 0);
				$edge->setAttribute("active", false);
			}
			
			// save active edges to nexthops for all sending targets
			foreach($address2moteid as $moteid) {
				if(!isset($sendingTargets[$moteid]))
					continue;
				
				foreach($sendingTargets[$moteid] as $destination) {
					$this->_executeOnRoutingEdges($graphAtMinute[$minute], $moteRoutingAtMinute, $minute, $moteid, $destination, function(Edge $edge) {
						$edge->setAttribute("active", true);
					});
				}
			}
			
			// save message flow for minute
			foreach($sentPackets as $packet) {
				$packetMinute = (int) ceil(($packet["time"] - $starttime) / 60);
				if($packetMinute == $minute && $packet["received"]) {
					$this->_executeOnRoutingEdges($graphAtMinute[$minute], $moteRoutingAtMinute, $minute, $packet["source"], $packet["destination"], function(Edge $edge) {
						$edge->setAttribute("sentmessage_minute", $edge->getAttribute("sentmessage_minute", 0) + 1);
						$edge->setAttribute("sentmessage_accumulated", $edge->getAttribute("sentmessage_accumulated", 0) + 1);
					});
				}
			}
		}
		
		return $graphAtMinute;
	}
	
	/**
	 * serializes the analyze results to a csv file
	 *
	 * @param string $filename
	 * @param mixed $data
	 */
	public function serialize($filename, $data) {
		throw new Exception("TODO: graphml output");
	}
	
	public function serializationType() {
		return "graphml";
	}
	
	/**
	 * merges several serialized evaluations
	 *
	 * @param string $filename
	 * @param array $serializationFiles
	 */
	public function merge($filename, array $serializationFiles) {
		throw new Exception("merge operation makes no sense for this analyzer");
	}
	
	/**
	 * execute a callback on all edges of the routing path
	 * 
	 * @param Graph $graph
	 * @param array $moteRoutingAtMinute
	 * @param int $minute
	 * @param int $source
	 * @param int $destination
	 * @param callable $function
	 */
	private function _executeOnRoutingEdges(Graph $graph, array $moteRoutingAtMinute, $minute, $source, $destination, callable $function) {
		// find routing path
		$routingPath = [$source];
		while($routingPath[count($routingPath) - 1] != $destination) {
			$routingTableBeforeSending = $routingTableAfterSending = [];
			if(isset($moteRoutingAtMinute[$routingPath[count($routingPath) - 1]][$minute - 1])) {
				$routingTableBeforeSending = $moteRoutingAtMinute[$routingPath[count($routingPath) - 1]][$minute - 1];
			}
			if(isset($moteRoutingAtMinute[$routingPath[count($routingPath) - 1]][$minute])) {
				$routingTableAfterSending = $moteRoutingAtMinute[$routingPath[count($routingPath) - 1]][$minute];
			}
				
			// find routing path or exit search if path is broken
			$found = false;
			foreach(array_merge($routingTableBeforeSending, $routingTableAfterSending) as $entry) {
				if(!$found && $entry["destination"] == $destination) {
					$found = true;
					$routingPath[] = $entry["nexthop"];
				}
			}
			if(!$found) {
				return; // it's unclear how the message flowed in the system
			}
			
			
			// there may be loops in the routing table (possibly because we only have snapshots at some points in time)
			if(count($routingPath) != count(array_unique($routingPath))) {
				// ROUTING LOOP DETECTION DIDN'T WORK VERY WELL
				//trigger_error("possible (unconfirmed) routing loop found around minute $minute for mote $source to $destination: " . implode("~>", $routingPath), E_USER_WARNING);
				return; 
			}
		}
		
		// execute function on routing edges
		foreach($routingPath as $i => $mote) {
			if($mote != $destination) {
				$from = $graph->getVertex($mote);
				$to = $graph->getVertex($routingPath[$i + 1]);
				
				$function($from->getEdgesTo($to)->count() ? $from->getEdgesTo($to)->getEdgeFirst() : $from->createEdgeTo($to));
			}
		}
	}
}