<?php

namespace App\Analyzer\Helper;

/**
 * Packet Reception Rate Trait
 *
 * Parses the sent and received packets from the mote output and will relate it to the
 * closest minute since the simulation's starting time to pretend motes running perfectly time
 * synchronized. 
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait PrrTrait {
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
	protected function _collectMotePrr(array $messages, array $address2moteid) {
		$starttime = $messages[0]->getTimestamp();
		$motePrrAtMinute = array_combine($address2moteid, array_fill(0, count($address2moteid), []));
		$packetDestinations = [];
		
		// prr for specific minutes
		foreach($this->_collectSentPackets($messages, $address2moteid) as $packet) {
			$packetDestinations[] = $packet["destination"];
			
			$minute = (int) ceil(($packet["time"] - $starttime) / 60);
			if(!isset($motePrrAtMinute[$packet["source"]][$minute]))
				$motePrrAtMinute[$packet["source"]][$minute] = ["sent" => 0, "received" => 0, "prr" => 0];
			
			$motePrrAtMinute[$packet["source"]][$minute]["sent"]++;
			$motePrrAtMinute[$packet["source"]][$minute]["received"] += ($packet["received"] ? 1 : 0);
			$motePrrAtMinute[$packet["source"]][$minute]["prr"] = ($motePrrAtMinute[$packet["source"]][$minute]["received"] / $motePrrAtMinute[$packet["source"]][$minute]["sent"]);
		}
		
		// remove sink-mote from prr list for data collection applications
		if(count(array_unique($packetDestinations)) == 1) {
			unset($motePrrAtMinute[array_unique($packetDestinations)[0]]);
		}
		
		// mote's are sending at random intervals [min, max] and it's possible that a mote has not
		// sent a single message in an interval, so add an empty prr entry to the $motePrrAtMinute
		$minutes = [];
		foreach($motePrrAtMinute as $prrAtMinute) {
			$minutes = array_unique(array_merge($minutes, array_keys($prrAtMinute)));
		}
		$minutes = array_unique(array_merge($minutes, range(0, max($minutes))));
		foreach($motePrrAtMinute as $moteid => $_prrAtMinute) {
			foreach($minutes as $minute) {
				if(!isset($motePrrAtMinute[$moteid][$minute])) {
					$motePrrAtMinute[$moteid][$minute] = ["sent" => 0, "received" => 0, "prr" => 0];
				}
			}
		}
		
		// overall prr of a mote
		foreach($motePrrAtMinute as $moteid => $minutes) {
			$values = array_column($minutes, "sent");
			
			$motePrrAtMinute[$moteid][-1] = [
				"sent" => array_sum(array_column($minutes, "sent")),
				"received" => array_sum(array_column($minutes, "received")),
				"prr" => array_sum(array_column($minutes, "received")) / array_sum(array_column($minutes, "sent"))
			];
			ksort($motePrrAtMinute[$moteid], SORT_NUMERIC);
		}
		
		return $motePrrAtMinute;
	}
	
	/**
	 * extracts send packet from the messages log and calculates whether the packet has been received
	 *
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	protected function _collectSentPackets(array $messages, array $address2moteid) {
		$sentMessages = [];
		$TYPE_SEND = 1;
		$TYPE_RECEIVE = 2;
		
		$prrEvents = $this->_extractPacketReceptionEvents($messages, $address2moteid);
		for($i = 0; $i < count($prrEvents); $i++) {
			if($prrEvents[$i]["type"] == $TYPE_SEND) {
				$received = false;
				for($j = $i + 1; $j < count($prrEvents); $j++) {
					if($prrEvents[$j]["type"] == $TYPE_SEND && $prrEvents[$j]["source"] == $prrEvents[$i]["source"])
						break; // mote has sent another message without the last message beeing received -> packet loss
		
					if($prrEvents[$j]["type"] == $TYPE_RECEIVE && $prrEvents[$j]["source"] == $prrEvents[$i]["source"]) {
						$received = true;
						break;
					}
				}
				
				$sentMessages[] = [
					"time" => $prrEvents[$i]["time"],
					"source" => $prrEvents[$i]["source"],
					"destination" => $prrEvents[$i]["destination"],
					"received" => $received
				];
			}
		}
		
		return $sentMessages;
	}
	
	/**
	 * extracts packet send and packet receive events from the log messages
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	private function _extractPacketReceptionEvents(array $messages, array $address2moteid) {
		$TYPE_SEND = 1;
		$TYPE_RECEIVE = 2;
		
		$prrEvents = [];
		foreach($messages as $message) {
			if(preg_match("/^\[application-.*\] sending data to (?P<address>.+)$/", $message->getMessage(), $matches)) {
				if(!isset($address2moteid[$matches["address"]])) {
					trigger_error("destinatin mote address {$matches["address"]} of logid {$message->getLogid()} is unknown", E_USER_WARNING);
					continue;
				}
	
				$prrEvents[] = [
					"type" => $TYPE_SEND,
					"time" => $message->getTimestamp(),
					"source" => $message->getMoteid(),
					"destination" => $address2moteid[$matches["address"]]
				];
			}
	
			if(preg_match("/^\[application-.*\] received data from (?P<address>.+)$/", $message->getMessage(), $matches)) {
				if(!isset($address2moteid[$matches["address"]])) {
					trigger_error("source mote address {$matches["address"]} of logid {$message->getLogid()} is unknown", E_USER_WARNING);
					continue;
				}
	
				$prrEvents[] = [
					"type" => $TYPE_RECEIVE,
					"time" => $message->getTimestamp(),
					"source" => $address2moteid[$matches["address"]],
					"destination" => $message->getMoteid()
				];
			}
		}
	
		return $prrEvents;
	}
}