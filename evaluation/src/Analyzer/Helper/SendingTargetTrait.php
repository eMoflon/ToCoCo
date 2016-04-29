<?php

namespace App\Analyzer\Helper;

/**
 * Mote Sending Target Trait
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait SendingTargetTrait {
	/**
	 * collects all sending targets for a mote from the log messages
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	public function _collectMoteSendingTargets(array $messages, array $address2moteid) {
		$sendingTargets = [];
		foreach($messages as $message) {
			if(preg_match("/\[application-.*\] sending targets: (?P<sending_targets>.+)$/",$message->getMessage(), $matches)) {
				if(!isset($sendingTargets[$message->getMoteid()]))
					$sendingTargets[$message->getMoteid()] = [];
				
				foreach(explode(", ", $matches["sending_targets"]) as $moteaddress) {
					if(!isset($address2moteid[$moteaddress])) {
						trigger_error("sending target $moteaddress of {$message->getMoteid()} is unknown", E_USER_WARNING);
						continue;
					}
						
					$sendingTargets[$message->getMoteid()][] = $address2moteid[$moteaddress];
				}
			}
		}
		
		return $sendingTargets;
	}
}