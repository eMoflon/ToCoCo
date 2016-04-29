<?php

namespace App\Analyzer\Helper;

/**
 * Routing Trait
 *
 * Parses routing path for ever mote
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait RoutingTrait {
	/**
	 * collects and preprocesses mote prr information
	 * 
	 * The information will be returned in the format:
	 * array(
	 *     :moteid-1 => :information,
	 *     :moteid-2 => :information,
	 *     ...,
	 *     :moteid-n => :information,
	 * )
	 * and :information has the following structure:
	 * array(
	 *     :minute-1 => :stats,
	 *     :minute-2 => :stats,
	 *     ...,
	 *     :minute-n => :stats,
	 * )
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	protected function _collectMoteRouting(array $messages, array $address2moteid) {
		$starttime = $messages[0]->getTimestamp();
		$moteRoutingAtMinute = array_combine($address2moteid, array_fill(0, count($address2moteid), []));
		
		foreach($messages as $message) {
			if(preg_match("/routing:(?P<route_entry>\s+.+~>.+,?)*/", $message->getMessage(), $matches)) {
				$routeEntries = [];
				
				if(isset($matches["route_entry"])) {
					foreach(explode(",", $matches["route_entry"]) as $routeStr) {
						$nexthop = explode("~>", trim($routeStr))[0];
						$destination = explode("~>", trim($routeStr))[1];
						
						// sometime route discovery seems to insert routes to non-existing destinations
						// because rime has no packet checksum and the destination address in the rreq
						// packet is corrupted
						if(isset($address2moteid[$nexthop]) && isset($address2moteid[$destination])) {
							$routeEntries[] = ["nexthop" => $address2moteid[$nexthop], "destination" => $address2moteid[$destination]];
						}
					}
				}
				
				$minute = (int) round(($message->getTimestamp() - $starttime) / 60);
				$moteRoutingAtMinute[$message->getMoteid()][$minute] = $routeEntries;
			}
		}
		
		return $moteRoutingAtMinute;
	}
}