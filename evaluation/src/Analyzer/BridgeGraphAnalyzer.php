<?php

namespace App\Analyzer;

use Alom\Graphviz\Digraph;
use Fhaculty\Graph\Edge\Directed as DirectedEdge;

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
class BridgeGraphAnalyzer implements AnalyzerInterface {
	/**
	 * @var int
	 */
	protected $_graphMinute;
	
	/**
	 * @var array
	 */
	protected $_motePositions;
	
	/**
	 * constructs a new graph analyzer to return graph at a specific minute
	 * 
	 * @param unknown $graphMinute
	 */
	public function __construct($graphMinute, array $motePositions) {
		$this->_graphMinute = $graphMinute;
		$this->_motePositions = $motePositions;
	}
	
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[]
	 * @return \Fhaculty\Graph\Graph
	 */
	public function analyze(array $messages) {
		$neighborhood =  (new NeighborhoodAnalyzer())->analyze($messages);
		if(!isset($neighborhood[$this->_graphMinute])) {
			throw new Exception("no graph for minute {$this->_graphMinute} found");
		}
		
		return $neighborhood[$this->_graphMinute];
	}
	
	/**
	 * serializes the analyzed results to a png file
	 *
	 * @param string $filename
	 * @param \Fhaculty\Graph\Graph $data
	 */
	public function serialize($filename, $data) {
		$content="";
		foreach($data->getVertices() as $vertex) {
			if(!isset($this->_motePositions[$vertex->getId()])) {
				trigger_error("unknown mote position for mote {$vertex->getId()}", E_USER_WARNING);
			} else {
				$content.=sprintf("NODE %d %d %d\n",$vertex->getId(), $this->_motePositions[$vertex->getId()]["x"], $this->_motePositions[$vertex->getId()]["y"]);
			}
		}
		
		foreach($data->getEdges() as $edge) {
			if($edge instanceof DirectedEdge)
					$content.=sprintf("EDGE %d %d %d %d\n",$edge->getVerticesStart()->getVertexFirst()->getId(),$edge->getVerticesTarget()->getVertexFirst()->getId(),$edge->getAttribute("weight"),NAN);
				else
					$content.=sprintf("EDGE %d %d %d %d\n",$edge->getVerticesStart()->getVertexFirst()->getId(),$edge->getVerticesTarget()->getVertexFirst()->getId(),$edge->getAttribute("weight"),$edge->getAttribute("weight"));
			}
		file_put_contents($filename,$content);
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
		throw new Exception("merge operation makes no sense for this analyzer");
	}
}