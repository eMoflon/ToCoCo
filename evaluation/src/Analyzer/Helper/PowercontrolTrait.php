<?php

namespace App\Analyzer\Helper;

use App\Analyzer\Exception;

/**
 * Powercontrol Trait
 *
 * Parses powercontrol information from output
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait PowercontrolTrait {
	/**
	 * collects and preprocesses powercontrol information
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
	 *     :mote-x => :powercontrols,
	 *     :mote-y => :powercontrols,
	 *     :mote-z => :powercontrols
	 * )
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	protected function _collectPowercontrol(array $messages, array $address2moteid) {
		$starttime = $messages[0]->getTimestamp();
		
		$powercontrols = [];
		foreach($messages as $message) {
			if(preg_match("/^\[evaluation\] powercontrol: (?P<powercontrols>.*)$/i", $message->getMessage(), $matches)) {
				$minute = (int) round(($message->getTimestamp() - $starttime) / 60);
				if(!isset($powercontrols[$minute]))
					$powercontrols[$minute] = [];
				if(!isset($powercontrols[$minute][$message->getMoteid()]))
					$powercontrols[$minute][$message->getMoteid()] = [];

				if(!$matches["powercontrols"])
					continue;
				
				foreach(explode(", ", trim($matches["powercontrols"])) as $powercontrol) {
					if(!preg_match("/(?P<address>.+)=(?P<powerlevel>\-?\d+)$/U", trim($powercontrol), $matches2))
						throw new Exception("powercontrol '" . trim($powercontrol) . "' has wrong format of logid " . $message->getLogid());
					
					if(!isset($address2moteid[$matches2['address']])) 
						throw new Exception("unknown mote address '" . $matches2['address'] . "' of logid " . $message->getLogid());
					
					$powercontrols[$minute][$message->getMoteid()][$address2moteid[$matches2['address']]] = (int) $matches2["powerlevel"];
				}
			}
		}
		
		return $powercontrols;
	}
}