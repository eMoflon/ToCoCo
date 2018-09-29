<?php

namespace App\Analyzer;

use App\Analyzer\Helper\MoteAddressTrait;
use App\Analyzer\Helper\NeighborhoodTrait;
use App\Analyzer\Helper\IgnoredLinksTrait;
use Fhaculty\Graph\Graph as Graph;
use Fhaculty\Graph\Edge\Base as Edge;

/**
 * Neighborhood Graph Analyzer
 * 
 * Creates a graph image for a specific minute
 *
 * REMARK: this is not directly an analyzer but the same implemented interface makes
 * the development of the evaluation cli-script much easier
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class NeighborhoodAnalyzer implements AnalyzerInterface {
	use MoteAddressTrait;
	use NeighborhoodTrait;
	use IgnoredLinksTrait;
	
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[]
	 * @return \Fhaculty\Graph\Graph
	 */
	public function analyze(array $messages) {
		$starttime = $messages[0]->getTimestamp();
		$moteid2address = $this->_collectMoteAddresses($messages);
		$address2moteid = array_flip($moteid2address);
		$neighborhood = $this->_collectNeighborhood($messages, $address2moteid);
		$ignoredlinks = $this->_collectIgnoredLinks($messages, $address2moteid);
		
		// init graph with mote vertices
		$initgraph = new Graph();
		foreach($address2moteid as $moteid) {
			$initgraph->createVertex($moteid);
		}
		
		$graphAtMinute = [];
		foreach($neighborhood as $minute => $motes) {
			$graphAtMinute[$minute] = $nowgraph = $initgraph->createGraphClone();
			
			foreach($motes as $moteid => $moteNeighborhoods) {
				foreach($moteNeighborhoods as $moteNeighborhood) {
					//
					// edge from node2 to node1 (because 1 received from 2)
					//
					
					if($moteNeighborhood["node1"] != $moteid)
						continue;
					if(in_array($moteNeighborhood["node2"] , $ignoredlinks[$minute][$moteNeighborhood["node1"]]))
						continue;
					
					$node1 = $nowgraph->getVertex($moteNeighborhood["node1"]);
					$node2 = $nowgraph->getVertex($moteNeighborhood["node2"]);
					if(!$node2->hasEdgeFrom($node1)) {
						$edge = $node2->createEdgeTo($node1);
						$edge->setAttribute("weight", $moteNeighborhood["weight"]["node2_to_node1"]);
						$weight=$moteNeighborhood["weight"]["node2_to_node1"];
					} else {
						$oldEdge = $node2->getEdgesFrom($node1)->getEdgeFirst();
						$oldEdge->destroy();
						
						$edge = $node1->createEdge($node2);
						$weight=array_sum($moteNeighborhood["weight"]) / 2;
						$edge->setAttribute("weight", array_sum($moteNeighborhood["weight"]) / 2);
					}
				}
			}
		}
		
		return $graphAtMinute;
	}
	
	/**
	 * serializes the analyzed results to a png file
	 *
	 * @param string $filename
	 * @param \Fhaculty\Graph\Graph $data
	 */
	public function serialize($filename, $data) {
		throw new Exception("TODO: graphml output");
	}
	
	/**
	 * gets serialization filetype
	 *
	 * @return string filetype
	 */
	public function serializationType() {
		return 'graphml';
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
}