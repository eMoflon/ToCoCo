<?php

namespace App\Analyzer;

use Alom\Graphviz\Digraph;

/**
 * Routing Graph Analyzer
 * 
 * Creates a graph image for a specific minute
 *
 * REMARK: this is not directly an analyzer but the same implemented interface makes
 * the development of the evaluation cli-script much easier
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class RoutingGraphAnalyzer implements AnalyzerInterface {
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
		$routing =  (new RoutingAnalyzer())->analyze($messages);
		if(!isset($routing[$this->_graphMinute])) {
			throw new Exception("no graph for minute {$this->_graphMinute} found");
		}
		
		/** @var \Fhaculty\Graph\Graph $graph */
		$graph = $routing[$this->_graphMinute];
		foreach($graph->getEdges() as $e) {
			if($e->getAttribute("active") === false) {
				$graph->removeEdge($e);
			}
			
			// TODO remove all attributes
		}
		
		return $graph;
	}
	
	/**
	 * serializes the analyzed results to a png file
	 *
	 * @param string $filename
	 * @param \Fhaculty\Graph\Graph $data
	 */
	public function serialize($filename, $data) {
		$graphviz = new Digraph("G");
		
		foreach($data->getVertices() as $vertex) {
			$moteAttributes = ["shape" => "circle", "width" => "0.4", "fixedsize"=> true];
			if(!isset($this->_motePositions[$vertex->getId()])) {
				trigger_error("unknown mote position for mote {$vertex->getId()}", E_USER_WARNING);
			} else {
				$moteAttributes["pos"] = sprintf("%d,-%d!", $this->_motePositions[$vertex->getId()]["x"], $this->_motePositions[$vertex->getId()]["y"]);
			}
			
			$graphviz->node($vertex->getId(), $moteAttributes);
		}
		
		$edgesPlotted = [];
		foreach($data->getEdges() as $edge) {
			$node1 = $edge->getVerticesStart()->getVertexFirst();
			$node2 = $edge->getVerticesTarget()->getVertexFirst();
			
			if(isset($edgesPlotted[min($node1->getId(), $node2->getId()) . "-" . max($node1->getId(), $node2->getId())]))
				continue;
			$edgesPlotted[min($node1->getId(), $node2->getId()) . "-" . max($node1->getId(), $node2->getId())] = true;
			
			if($node2->hasEdgeTo($node1)) {
				$graphviz->edge([$node1->getId(), $node2->getId()], ["arrowhead" => "normal", "arrowtail" => "normal", "dir" => "both"]);
			} else {
				$graphviz->edge([$node1->getId(), $node2->getId()]);
			}
		}
		
		$command = sprintf("echo %s | dot -Tpng -Kneato -n -o%s", escapeshellarg(str_replace("\n", " ",$graphviz->render())), escapeshellarg($filename));
		system($command, $ret);
		if($ret !== 0) {
			throw new Exception("failed running graphviz executable (returncode $ret)");
		}
	}
	
	/**
	 * gets serialization filetype
	 *
	 * @return string filetype
	 */
	public function serializationType() {
		return "png";
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