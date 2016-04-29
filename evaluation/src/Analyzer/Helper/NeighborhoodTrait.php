<?php

namespace App\Analyzer\Helper;

use App\Analyzer\Exception;

/**
 * Neighborhood Trait
 *
 * Parses neighborhoud information from neighbor discovery output
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait NeighborhoodTrait {
	/**
	 * collects and preprocesses neighborhood information
	 * 
	 * ignored links may still be in neighborhood information
	 * until the neighbor discovery detects the ignored link !!!
	 * 
	 * The information will be returned in the format:
	 * array(
	 *     :minute-0 => :information,
	 *     :minute-1 => :information,
	 *     ...,
	 *     :minute-n => :information,
	 * )
	 * and :information has the following structure:
	 * array(
	 *     :mote-x => :neighborhoods,
	 *     :mote-y => :neighborhoods,
	 *     :mote-z => :neighborhoods
	 * )
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	protected function _collectNeighborhood(array $messages, array $address2moteid) {
		$starttime = $messages[0]->getTimestamp();
		
		$neighborlinks = [];
		foreach($messages as $message) {
			if(preg_match("/^\[evaluation\] neighborhood: (?P<neighborlinks>.*)$/i", $message->getMessage(), $matches)) {
				$minute = (int) round(($message->getTimestamp() - $starttime) / 60);
				if(!isset($neighborlinks[$minute]))
					$neighborlinks[$minute] = [];
				if(!isset($neighborlinks[$minute][$message->getMoteid()]))
					$neighborlinks[$minute][$message->getMoteid()] = [];

				if(!$matches["neighborlinks"])
					continue;
				
				foreach(explode(", ", trim($matches["neighborlinks"])) as $link) {
					if(!preg_match("/(?P<address_node1>.+)->(?P<address_node2>.+)(\[(?P<weight_node1_to_node2>-?\d+),(?P<weight_node2_to_node1>-?\d+)\])?$/U", trim($link), $matches2))
						throw new Exception("neighborlink '" . trim($link) . "' has wrong format of logid " . $message->getLogid());
					
					
					if(!isset($address2moteid[$matches2['address_node1']])) 
						throw new Exception("unknown mote address '" . $matches2['address_node1'] . "' of logid " . $message->getLogid());
					if(!isset($address2moteid[$matches2['address_node2']]))
						throw new Exception("unknown mote address '" . $matches2['address_node2'] . "' of logid " . $message->getLogid());
					
					$neighborlinks[$minute][$message->getMoteid()][] = [
						'node1' => $address2moteid[$matches2['address_node1']],
						'node2' => $address2moteid[$matches2['address_node2']],
						'weight' => [
							'node1_to_node2' => (isset($matches2['weight_node1_to_node2']) && $matches2['weight_node1_to_node2'] !== "-1") ? (int) $matches2['weight_node1_to_node2'] : false,
							'node2_to_node1' => (isset($matches2['weight_node2_to_node1']) && $matches2['weight_node2_to_node1'] !== "-1") ? (int) $matches2['weight_node2_to_node1'] : false,
						]
					];
				}
			}
		}
		
		return $neighborlinks;
	}
}